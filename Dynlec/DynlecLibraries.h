#pragma once

#include "DynlecQuick.h"

namespace Dynlec
{
	DYNLEC_QUICK_LIBR(KernelLibrary, "Kernel32.dll");
	DYNLEC_QUICK_FUNC(
		FindFirstVolumeA,
		KernelLibrary,
		"FindFirstVolumeA",
		HANDLE(__stdcall*)(LPSTR, DWORD));
	DYNLEC_QUICK_FUNC(
		FindNextVolumeA,
		KernelLibrary,
		"FindNextVolumeA",
		BOOL(__stdcall*)(HANDLE, LPSTR, DWORD));
	DYNLEC_QUICK_FUNC(
		FindVolumeClose,
		KernelLibrary,
		"FindVolumeClose",
		BOOL(__stdcall*)(HANDLE));
	DYNLEC_QUICK_FUNC(
		GetVolumeInformationA,
		KernelLibrary,
		"GetVolumeInformationA",
		BOOL(__stdcall*)(LPCSTR, LPSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPSTR, DWORD));
}
