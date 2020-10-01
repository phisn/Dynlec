#pragma once

#include "DynlecCaller.h"
#include "DynlecCommon.h"
#include "DynlecDebug.h"
#include "DynlecLibraries.h"

namespace Dynlec
{
	class Library
	{
        typedef struct POINTER_LIST {
            struct POINTER_LIST* next;
            void* address;
        } POINTER_LIST;

	public:
		Library(const char* binary, size_t size);

	private:
        PIMAGE_NT_HEADERS headers;
        unsigned char* codeBase;
        void* modules;
        int numModules;
        bool initialized;
        bool isDLL;
        bool isRelocated;
        struct ExportNameEntry* nameExportsTable;
        ExeEntryProc exeEntry;
        unsigned long pageSize;
        POINTER_LIST* blockedMemory;

        void FreePointerList(POINTER_LIST* head);
	};
}
