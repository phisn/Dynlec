#pragma once

#include "DynlecEncrypt.h"

#ifndef __INTELLISENSE__
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <libloaderapi.h>

// Usage example:
//
// DYNLEC_QUICK_LIBR(ExampleLibrary, "example.dll")
// DYNLEC_QUICK_FUNC(ExampleFunction, ExampleLibrary, "doexample", int(*)(int));
//

// shortcut for defining dynlec libraries
#define DYNLEC_QUICK_LIBR(alias, path) \
	struct alias \
	{ \
		static inline DYNLEC_CT_ENCRYPT_INPLACE(Name, path); \
		static inline HMODULE Module; \
	}

// shortcut for defining dynlec functions
// symbol has to be a cstring
// definition has to be defined as function pointer
#define DYNLEC_QUICK_FUNC(alias, library, symbol, definition) \
	static_assert(::Dynlec::ReturnTypeExtractor<definition>::valid, \
		"Invalid quick dynlec function definition '" symbol "' from '" #alias "'"); \
	struct alias \
	{ \
		using Definition = definition; \
		typedef library Library;\
		typedef ::Dynlec::ReturnTypeExtractor<Definition>::type Return; \
		static inline DYNLEC_CT_ENCRYPT_INPLACE(Name, symbol); \
		static inline FARPROC Procedure; \
	}

namespace Dynlec
{
	template <typename Function>
	struct ReturnTypeExtractor
	{
		static constexpr bool valid = false;
	};

	template <typename Return, typename... Arguments>
	struct ReturnTypeExtractor<Return(*)(Arguments...)>
	{
		static constexpr bool valid = true;
		typedef Return type;
	};
}
