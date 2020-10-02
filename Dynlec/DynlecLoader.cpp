#include "DynlecLoader.h"

#include <sysinfoapi.h>
#include <winnt.h>

struct ExportNameEntry {
    const char* name;
    unsigned short idx;
};

typedef int (__stdcall* DllEntryProc)(void* hinstDLL, unsigned long fdwReason, void* lpReserved);
typedef int (__stdcall* ExeEntryProc)(void);


#ifndef IMAGE_SIZEOF_BASE_RELOCATION
// Vista SDKs no longer define IMAGE_SIZEOF_BASE_RELOCATION!?
#define IMAGE_SIZEOF_BASE_RELOCATION (sizeof(IMAGE_BASE_RELOCATION))
#endif

#define GET_HEADER_DICTIONARY(module, idx)  &(module)->headers->OptionalHeader.DataDirectory[idx]

namespace Dynlec
{
    inline uintptr_t AlignValueDown(uintptr_t value, uintptr_t alignment) {
        return value & ~(alignment - 1);
    }

    inline LPVOID AlignAddressDown(LPVOID address, uintptr_t alignment) {
        return (LPVOID)AlignValueDown((uintptr_t)address, alignment);
    }

    inline size_t AlignValueUp(size_t value, size_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    inline void* OffsetPointer(void* data, ptrdiff_t offset) {
        return (void*)((uintptr_t)data + offset);
    }

    Library::Library(const char* binary, size_t size)
    {
        unsigned char* codeBase, * headersBuffer;
        ptrdiff_t locationDelta;
        SYSTEM_INFO sysInfo;
        unsigned long i;
        size_t lastSectionEnd = 0;
        size_t alignedImageSize;

        if (size < sizeof(IMAGE_DOS_HEADER)) 
        {
            DYNLEC_HANDLE_FATAL("Invalid dos header size");
            return;
        }

        PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER) binary;
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) 
        {
            DYNLEC_HANDLE_FATAL("Invalid dos signature");
            return;
        }

        if (size < dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS))
        {
            DYNLEC_HANDLE_FATAL("Invalid image size");
            return;
        }

        PIMAGE_NT_HEADERS old_header = (PIMAGE_NT_HEADERS) & ((const unsigned char*)(binary))[dos_header->e_lfanew];
        if (old_header->Signature != IMAGE_NT_SIGNATURE) 
        {
            DYNLEC_HANDLE_FATAL("Invalid nt signature");
            return;
        }

        if (old_header->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) 
        {
            DYNLEC_HANDLE_FATAL("Invalid machine type");
            return;
        }

        if (old_header->OptionalHeader.SectionAlignment & 1) 
        {
            // Only support section alignments that are a multiple of 2
            DYNLEC_HANDLE_FATAL("Invalid section alignment");
            return;
        }

        PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(old_header);
        size_t optionalSectionSize = old_header->OptionalHeader.SectionAlignment;
        for (i = 0; i < old_header->FileHeader.NumberOfSections; i++, section++) 
        {
            size_t endOfSection = section->SizeOfRawData == 0
                // Section without data in the DLL
                ? section->VirtualAddress + optionalSectionSize
                : section->VirtualAddress + section->SizeOfRawData;

            if (endOfSection > lastSectionEnd) 
                lastSectionEnd = endOfSection;
        }

        GetNativeSystemInfo(&sysInfo);
        alignedImageSize = AlignValueUp(old_header->OptionalHeader.SizeOfImage, sysInfo.dwPageSize);
        if (alignedImageSize != AlignValueUp(lastSectionEnd, sysInfo.dwPageSize)) 
        {
            DYNLEC_HANDLE_FATAL("Invalid image format. Failed to align up");
            return;
        }

        // reserve memory for image of library
        // XXX: is it correct to commit the complete memory region at once?
        //      calling DllEntry raises an exception if we don't...
        codeBase = (unsigned char*) Dynlec::Call<Dynlec::VirtualAlloc>(
            (LPVOID)(old_header->OptionalHeader.ImageBase),
            alignedImageSize,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_READWRITE);

        if (codeBase == NULL)
        {
            // try to allocate memory at arbitrary position
            codeBase = (unsigned char*) Dynlec::Call<Dynlec::VirtualAlloc>(
                NULL,
                alignedImageSize,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE);

            if (codeBase == NULL)
            {
                DYNLEC_HANDLE_FATAL("Missing memory");
                return;
            }
        }

        // Memory block may not span 4 GB boundaries.
        while ((((uintptr_t) codeBase) >> 32) < (((uintptr_t)(codeBase + alignedImageSize)) >> 32)) 
        {
            POINTER_LIST* node = (POINTER_LIST*)malloc(sizeof(POINTER_LIST));
            if (!node) 
            {
                Dynlec::Call<Dynlec::VirtualFree>(
                    codeBase, 
                    0, 
                    MEM_RELEASE);
                FreePointerList(blockedMemory);

                DYNLEC_HANDLE_FATAL("Missing memory");
                return;
            }

            node->next = blockedMemory;
            node->address = codeBase;
            blockedMemory = node;

            codeBase = (unsigned char*) Dynlec::Call<Dynlec::VirtualAlloc>(
                NULL,
                alignedImageSize,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE);
            if (codeBase == NULL) {
                FreePointerList(blockedMemory);

                DYNLEC_HANDLE_FATAL("Missing memory");
                return;
            }
        }

        isDLL = (old_header->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0;
        pageSize = sysInfo.dwPageSize;

        if (size < old_header->OptionalHeader.SizeOfHeaders) 
        {
            DYNLEC_HANDLE_FATAL("Invalid image size");
            return;
        }

        // commit memory for headers
        headersBuffer = (unsigned char*) Dynlec::Call<Dynlec::VirtualAlloc>(
            codeBase,
            old_header->OptionalHeader.SizeOfHeaders,
            MEM_COMMIT,
            PAGE_READWRITE);

        if (headersBuffer == NULL)
        {
            DYNLEC_HANDLE_FATAL("Virtualalloc failed", GetLastError());
            return;
        }

        // copy PE header to code
        memcpy(headersBuffer, dos_header, old_header->OptionalHeader.SizeOfHeaders);
        headers = (PIMAGE_NT_HEADERS) & ((const unsigned char*)(headersBuffer))[dos_header->e_lfanew];

        // update position
        headers->OptionalHeader.ImageBase = (uintptr_t)codeBase;

        // copy sections from DLL file block to new memory location
        if (!CopySections((const unsigned char*) binary, size, old_header)) 
        {
            DYNLEC_HANDLE_FATAL("Failed to copy sections");
            return;
        }

        // adjust base address of imported data
        locationDelta = (ptrdiff_t)(this->headers->OptionalHeader.ImageBase - old_header->OptionalHeader.ImageBase);
        if (locationDelta != 0) 
        {
            isRelocated = PerformBaseRelocation(locationDelta);
        }
        else 
        {
            isRelocated = true;
        }

        // load required dlls and adjust function table of imports
        if (!BuildImportTable()) 
        {
            DYNLEC_HANDLE_FATAL("Failed to handle fatal");
            return;
        }

        // mark memory pages depending on section headers and release
        // sections that are marked as "discardable"
        if (!FinalizeSections(result)) 
        {
            DYNLEC_HANDLE_FATAL("Failed to finalize sections");
            return;
        }

        // TLS callbacks are executed BEFORE the main loading
        if (!ExecuteTLS(result)) 
        {
            DYNLEC_HANDLE_FATAL("Failed to execute tls callbacks");
            return;
        }

        // get entry point of loaded library
        if (result->headers->OptionalHeader.AddressOfEntryPoint != 0) 
        {
            if (result->isDLL) 
            {
                DllEntryProc DllEntry = (DllEntryProc)(LPVOID)(codeBase + result->headers->OptionalHeader.AddressOfEntryPoint);

                // notify library about attaching to process
                BOOL successfull = (*DllEntry)((HINSTANCE) codeBase, DLL_PROCESS_ATTACH, 0);
                if (!successfull) 
                {
                    DYNLEC_HANDLE_FATAL("Failed to initialize dll", GetLastError());
                    return;
                }

                result->initialized = true;
            }
            else 
            {
                result->exeEntry = (ExeEntryProc)(LPVOID)(codeBase + result->headers->OptionalHeader.AddressOfEntryPoint);
            }
        }
        else 
        {
            result->exeEntry = NULL;
        }
    }

    void Library::FreePointerList(POINTER_LIST* head)
    {
        POINTER_LIST* node = head;
        while (node) 
        {
            POINTER_LIST* next;
            Dynlec::Call<Dynlec::VirtualFree>(
                node->address,
                0,
                MEM_RELEASE);
            next = node->next;
            free(node);
            node = next;
        }
    }

    bool Library::CopySections(
        const unsigned char* data, 
        size_t size, 
        PIMAGE_NT_HEADERS old_headers)
    {
        unsigned char* dest;
        PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(headers);
        for (int i = 0; i < headers->FileHeader.NumberOfSections; i++, section++) 
        {
            if (section->SizeOfRawData == 0) 
            {
                // section doesn't contain data in the dll itself, but may define
                // uninitialized data
                int section_size = old_headers->OptionalHeader.SectionAlignment;
                if (section_size > 0) 
                {
                    dest = (unsigned char*) Dynlec::Call<Dynlec::VirtualAlloc>(
                        codeBase + section->VirtualAddress,
                        section_size,
                        MEM_COMMIT,
                        PAGE_READWRITE);

                    if (dest == NULL)
                    {
                        DYNLEC_DEBUG_INFO("Virtualalloc failed", GetLastError());
                        return false;
                    }
                    
                    // Always use position from file to support alignments smaller
                    // than page size (allocation above will align to page size).
                    dest = codeBase + section->VirtualAddress;
                    // NOTE: On 64bit systems we truncate to 32bit here but expand
                    // again later when "PhysicalAddress" is used.
                    section->Misc.PhysicalAddress = (DWORD)((uintptr_t)dest & 0xffffffff);
                    memset(dest, 0, section_size);
                }

                // section is empty
                continue;
            }

            if (size < section->PointerToRawData + section->SizeOfRawData)
                return false;

            // commit memory block and copy data from dll
            dest = (unsigned char*) Dynlec::Call<Dynlec::VirtualAlloc>(
                codeBase + section->VirtualAddress,
                section->SizeOfRawData,
                MEM_COMMIT,
                PAGE_READWRITE);

            if (dest == NULL)
                return false;

            // Always use position from file to support alignments smaller
            // than page size (allocation above will align to page size).
            dest = codeBase + section->VirtualAddress;
            memcpy(dest, data + section->PointerToRawData, section->SizeOfRawData);
            // NOTE: On 64bit systems we truncate to 32bit here but expand
            // again later when "PhysicalAddress" is used.
            section->Misc.PhysicalAddress = (DWORD)((uintptr_t) dest & 0xffffffff);
        }

        return true;
    }

    bool Library::PerformBaseRelocation(ptrdiff_t delta)
    {
        PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(this, IMAGE_DIRECTORY_ENTRY_BASERELOC);

        if (directory->Size == 0)
            return delta == 0;

        PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->VirtualAddress);
        while (relocation->VirtualAddress > 0) 
        {
            unsigned char* dest = codeBase + relocation->VirtualAddress;
            unsigned short* relInfo = (unsigned short*) OffsetPointer(relocation, IMAGE_SIZEOF_BASE_RELOCATION);
            for (DWORD i = 0; i < ((relocation->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2); i++, relInfo++) {
                // the upper 4 bits define the type of relocation
                int type = *relInfo >> 12;
                // the lower 12 bits define the offset
                int offset = *relInfo & 0xfff;

                switch (type)
                {
                case IMAGE_REL_BASED_ABSOLUTE:
                    // skip relocation
                    break;

                case IMAGE_REL_BASED_HIGHLOW:
                    // change complete 32 bit address
                {
                    DWORD* patchAddrHL = (DWORD*)(dest + offset);
                    *patchAddrHL += (DWORD)delta;
                }
                    break;
                case IMAGE_REL_BASED_DIR64:
                {
                    ULONGLONG* patchAddr64 = (ULONGLONG*)(dest + offset);
                    *patchAddr64 += (ULONGLONG)delta;
                }
                    break;
                default:
                    DYNLEC_DEBUG_INFO("Unkown relocation: ", type);
                    break;
                }
            }

            // advance to next relocation block
            relocation = (PIMAGE_BASE_RELOCATION) OffsetPointer(relocation, relocation->SizeOfBlock);
        }

        return true;
    }

    bool Library::BuildImportTable()
    {
        PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(this, IMAGE_DIRECTORY_ENTRY_IMPORT);
        if (directory->Size == 0)
            return true;

        PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(codeBase + directory->VirtualAddress);
        for (; !IsBadReadPtr(importDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && importDesc->Name; importDesc++) {
            uintptr_t* thunkRef;
            FARPROC* funcRef;

            HMODULE handle = Dynlec::Call<Dynlec::LoadLibraryA>(
                (LPCSTR)(codeBase + importDesc->Name));

            if (handle == NULL)
            {
                DYNLEC_DEBUG_INFO("Loadlibrary failed", GetLastError());
                return false;
            }

            modules = (void**) realloc(modules, (numModules + 1) * (sizeof(HMODULE)));
            if (modules == NULL) 
            {
                DYNLEC_DEBUG_INFO("Realloc failed");
                Dynlec::Call<Dynlec::FreeLibrary>(handle);
                return false;
            }

            modules[numModules++] = handle;

            thunkRef = (uintptr_t*)(codeBase + importDesc->OriginalFirstThunk
                ? importDesc->OriginalFirstThunk
                : importDesc->FirstThunk);
            funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);

            for (; *thunkRef; thunkRef++, funcRef++) {
                if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
                {
                    *funcRef = Dynlec::Call<Dynlec::GetProcAddress>(
                        handle,
                        (LPCSTR)IMAGE_ORDINAL(*thunkRef));
                }
                else 
                {
                    PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + (*thunkRef));
                    *funcRef = Dynlec::Call<Dynlec::GetProcAddress>(
                        handle,
                        (LPCSTR) &thunkData->Name);
                }

                if (*funcRef == 0) 
                {
                    DYNLEC_DEBUG_INFO("GetProcAddress failed", GetLastError());
                    Dynlec::Call<Dynlec::FreeLibrary>(handle);
                    return false;
                }
            }
        }

        return true;
    }
}
