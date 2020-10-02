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
	DYNLEC_QUICK_FUNC(
		VirtualAlloc,
		KernelLibrary,
		"VirtualAlloc",
		LPVOID(__stdcall*)(LPVOID, SIZE_T, DWORD, DWORD));
	DYNLEC_QUICK_FUNC(
		VirtualFree,
		KernelLibrary,
		"VirtualFree",
		BOOL(__stdcall*)(LPVOID, SIZE_T, DWORD));
	DYNLEC_QUICK_FUNC(
		GetModuleHandleA,
		KernelLibrary,
		"GetModuleHandleA",
		HMODULE(__stdcall*)(LPCSTR));
	DYNLEC_QUICK_FUNC(
		LoadLibraryA,
		KernelLibrary,
		"LoadLibraryA",
		HMODULE(__stdcall*)(LPCSTR));
	DYNLEC_QUICK_FUNC(
		FreeLibrary,
		KernelLibrary,
		"FreeLibrary",
		BOOL(__stdcall*)(HMODULE));
	DYNLEC_QUICK_FUNC(
		GetProcAddress,
		KernelLibrary,
		"GetProcAddress",
		FARPROC(__stdcall*)(HMODULE, LPCSTR));

	typedef enum _SHUTDOWN_ACTION {
		ShutdownNoReboot,
		ShutdownReboot,
		ShutdownPowerOff
	} SHUTDOWN_ACTION, * PSHUTDOWN_ACTION;

	DYNLEC_QUICK_LIBR(NTLibrary, "ntdll.dll");
	DYNLEC_QUICK_FUNC(
		RtlAdjustPrivilege,
		NTLibrary,
		"RtlAdjustPrivilege",
		LONG(__stdcall*)(
			ULONG privilege,
			BOOLEAN enable,
			BOOLEAN currentThread,
			PBOOLEAN enabled));
	DYNLEC_QUICK_FUNC(
		NtOpenProcessToken,
		NTLibrary,
		"NtOpenProcessToken",
		LONG(__stdcall*)(
			HANDLE process, 
			ACCESS_MASK access, 
			PHANDLE token));
	DYNLEC_QUICK_FUNC(
		NtAdjustPrivilegesToken,
		NTLibrary,
		"NtAdjustPrivilegesToken",
		LONG(__stdcall*)(
			HANDLE token,
			BOOLEAN disableall,
			PTOKEN_PRIVILEGES privileges,
			ULONG privilegesLength,
			PTOKEN_PRIVILEGES,
			ULONG));
	DYNLEC_QUICK_FUNC(
		NtShutdownSystem,
		NTLibrary,
		"NtShutdownSystem",
		LONG (__stdcall*)(
			SHUTDOWN_ACTION action));
}
