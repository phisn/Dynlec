#pragma once

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
		static constexpr char Name[] = path; \
		static HMODULE Module; \
	};

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
		static constexpr char Name[] = symbol; \
	};

namespace Dynlec
{
	template <typename Function>
	struct ReturnTypeExtractor
	{
		static constexpr bool valid = false;
	};

	template <typename Return, typename... Arguments>
	struct ReturnTypeExtractor<Return(__stdcall *)(Arguments...)>
	{
		static constexpr bool valid = true;
		typedef Return type;
	};

	template <typename Return, typename... Arguments>
	struct ReturnTypeExtractor<Return(__cdecl *)(Arguments...)>
	{
		static constexpr bool valid = true;
		typedef Return type;
	};

	template <typename Return, typename... Arguments>
	struct ReturnTypeExtractor<Return(__fastcall *)(Arguments...)>
	{
		static constexpr bool valid = true;
		typedef Return type;
	};
}