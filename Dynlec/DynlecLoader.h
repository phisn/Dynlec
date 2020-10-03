#pragma once

#include "DynlecCaller.h"
#include "DynlecCommon.h"
#include "DynlecDebug.h"
#include "DynlecLibraries.h"

namespace Dynlec
{
	class Library
	{
        typedef int(__stdcall* DllEntryProc)(void* hinstDLL, unsigned long fdwReason, void* lpReserved);
        typedef int(__stdcall* ExeEntryProc)(void);

        struct ExportNameEntry {
            const char* name;
            unsigned short idx;
        };

        typedef struct POINTER_LIST {
            struct POINTER_LIST* next;
            void* address;
        } POINTER_LIST;

        typedef struct {
            LPVOID address;
            LPVOID alignedAddress;
            SIZE_T size;
            DWORD characteristics;
            bool last;
        } SECTIONFINALIZEDATA, * PSECTIONFINALIZEDATA;

	public:
		Library(const char* binary, size_t size);
        ~Library();

        FARPROC getProcAddress(LPCSTR);
        int callEntryPoint();

	private:
        bool loaded = false;

        PIMAGE_NT_HEADERS headers = NULL;
        unsigned char* codeBase = NULL;
        void** modules = NULL;
        int numModules = 0;
        bool initialized = false;
        bool isDLL = false;
        bool isRelocated = false;
        struct ExportNameEntry* nameExportsTable = NULL;
        ExeEntryProc exeEntry = NULL;
        unsigned long pageSize = 0;
        POINTER_LIST* blockedMemory = NULL;

        void FreePointerList(POINTER_LIST* head);
        bool CopySections(
            const unsigned char* data, 
            size_t size, 
            PIMAGE_NT_HEADERS old_headers);
        bool PerformBaseRelocation(ptrdiff_t delta);
        bool BuildImportTable();

        bool FinalizeSection(PSECTIONFINALIZEDATA sectionData);
        bool FinalizeSections();

        size_t GetRealSectionSize(PIMAGE_SECTION_HEADER section);

        bool ExecuteTLS();
	};
}
