#pragma once

#include "DynlecCommon.h"
#include "DynlecEncrypt.h"

/*
Definition example:
	DYNLEC_QUICK_LIBR(User32Library, "User32.dll");
	DYNLEC_QUICK_FUNC(
		MessageBoxA,
		User32Library,
		"MessageBoxA",
		int(__stdcall*)(HWND, LPCSTR, LPCSTR, UINT));

	DYNLEC_QUICK_LIBR(Winsock2Library, "Ws2_32.dll");
	DYNLEC_QUICK_FUNC(
		htons,
		Winsock2Library,
		"htons",
		unsigned short(__stdcall*)(unsigned short));

Call example:
	DL::Call<DL::MessageBoxA>((HWND) NULL, "Content", "Title", MB_OK);
	unsigned short port = DL::Call<DL::htons>(8080);
*/

namespace Dynlec
{
	extern std::function<void(DWORD, const char*)> HandleFailedLoadLibrary;
	extern std::function<void(DWORD, const char*)> HandleFailedGetProcAddress;

	template <typename T>
	const char* Obtain(T* value)
	{
		if constexpr (
			std::is_same_v<T, const char*> ||
			std::is_same_v<T, char*>)
		{
			return *value;
		}
		else
		{
			return value->obtain();
		}
	}

	template <typename Function, typename... Arguments>
	typename Function::Return Call(Arguments&&... arguments)
	{
		if (Function::Library::Module == NULL)
		{
			Function::Library::Module = LoadLibraryA(
				Obtain(&Function::Library::Name));

			if (Function::Library::Module == NULL)
			{
				if (HandleFailedLoadLibrary)
					HandleFailedLoadLibrary(
						GetLastError(),
						Obtain(&Function::Library::Name));
			}
		}

		if (Function::Procedure == NULL)
		{
			Function::Procedure = GetProcAddress(
				Function::Library::Module,
				Obtain(&Function::Name));

			if (Function::Procedure == NULL)
			{
				if (HandleFailedGetProcAddress)
					HandleFailedGetProcAddress(
						GetLastError(),
						Obtain(&Function::Name));
			}
		}

		return ((typename Function::Definition) Function::Procedure)(arguments...);
	}
}

#ifndef DYNLEC_ALIAS_HIDE
namespace DL = Dynlec;
#endif
