#pragma once

#include "DynlecEncrypt.h"

#include <type_traits>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <libloaderapi.h>

namespace Dynlec
{
	template <typename Function, typename... Arguments>
	typename Function::Return Call(Arguments&&... arguments)
	{
		/*static_assert(std::is_same_v<
			decltype(Call<Function, Arguments...>),
			typename Function::Definition>,
			"Invalid arguments in 'Dynlec' call");*/

		if (Function::Library::Module == NULL)
		{
			Function::Library::Module = LoadLibraryA(Function::Library::Name);
		}

		if (Function::Procedure == NULL)
		{
			Function::Procedure = GetProcAddress(
				Function::Library::Module,
				Function::Name);
		}

		return ((typename Function::Definition) Function::Procedure)(arguments...);
	}
}

#ifndef DYNLEC_ALIAS_HIDE
namespace DL = Dynlec;
#endif
