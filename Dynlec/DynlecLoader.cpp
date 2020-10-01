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

namespace Dynlec
{
    static inline uintptr_t AlignValueDown(uintptr_t value, uintptr_t alignment) {
        return value & ~(alignment - 1);
    }

    static inline LPVOID AlignAddressDown(LPVOID address, uintptr_t alignment) {
        return (LPVOID)AlignValueDown((uintptr_t)address, alignment);
    }

    static inline size_t AlignValueUp(size_t value, size_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    static inline void* OffsetPointer(void* data, ptrdiff_t offset) {
        return (void*)((uintptr_t)data + offset);
    }

    Library::Library(const char* binary, size_t size)
    {
        PIMAGE_DOS_HEADER dos_header;
        PIMAGE_NT_HEADERS old_header;
        unsigned char* codeBase, * headers;
        ptrdiff_t locationDelta;
        SYSTEM_INFO sysInfo;
        PIMAGE_SECTION_HEADER section;
        unsigned long i;
        size_t optionalSectionSize;
        size_t lastSectionEnd = 0;
        size_t alignedImageSize;

        if (size < sizeof(IMAGE_DOS_HEADER)) 
        {
            DYNLEC_HANDLE_FATAL("Invalid dos header size");
            return;
        }

        dos_header = (PIMAGE_DOS_HEADER) binary;
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

        old_header = (PIMAGE_NT_HEADERS) & ((const unsigned char*)(binary))[dos_header->e_lfanew];
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

        section = IMAGE_FIRST_SECTION(old_header);
        optionalSectionSize = old_header->OptionalHeader.SectionAlignment;
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
            goto error;
        }

        // commit memory for headers
        headers = (unsigned char*) Dynlec::Call<Dynlec::VirtualAlloc>(
            codeBase,
            old_header->OptionalHeader.SizeOfHeaders,
            MEM_COMMIT,
            PAGE_READWRITE);

        // copy PE header to code
        memcpy(headers, dos_header, old_header->OptionalHeader.SizeOfHeaders);
        headers = (PIMAGE_NT_HEADERS) & ((const unsigned char*)(headers))[dos_header->e_lfanew];

        // update position
        headers->OptionalHeader.ImageBase = (uintptr_t)codeBase;

        // copy sections from DLL file block to new memory location
        if (
            !CopySections((const unsigned char*)data, size, old_header, result)) {
            goto error;
        }

        // adjust base address of imported data
        locationDelta = (ptrdiff_t)(result->headers->OptionalHeader.ImageBase - old_header->OptionalHeader.ImageBase);
        if (locationDelta != 0) {
            result->isRelocated = PerformBaseRelocation(result, locationDelta);
        }
        else {
            result->isRelocated = TRUE;
        }

        // load required dlls and adjust function table of imports
        if (!BuildImportTable(result)) {
            goto error;
        }

        // mark memory pages depending on section headers and release
        // sections that are marked as "discardable"
        if (!FinalizeSections(result)) {
            goto error;
        }

        // TLS callbacks are executed BEFORE the main loading
        if (!ExecuteTLS(result)) {
            goto error;
        }

        // get entry point of loaded library
        if (result->headers->OptionalHeader.AddressOfEntryPoint != 0) {
            if (result->isDLL) {
                DllEntryProc DllEntry = (DllEntryProc)(LPVOID)(codeBase + result->headers->OptionalHeader.AddressOfEntryPoint);
                // notify library about attaching to process
                BOOL successfull = (*DllEntry)((HINSTANCE)codeBase, DLL_PROCESS_ATTACH, 0);
                if (!successfull) {
                    SetLastError(ERROR_DLL_INIT_FAILED);
                    goto error;
                }
                result->initialized = TRUE;
            }
            else {
                result->exeEntry = (ExeEntryProc)(LPVOID)(codeBase + result->headers->OptionalHeader.AddressOfEntryPoint);
            }
        }
        else {
            result->exeEntry = NULL;
        }

        return (HMEMORYMODULE)result;

    error:
        // cleanup
        MemoryFreeLibrary(result);
        return NULL;
    }

    void Library::FreePointerList(POINTER_LIST* head)
    {
        POINTER_LIST* node = head;
        while (node) {
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
}
