#pragma once

#include "DynlecCommon.h"

#ifdef _DEBUG
#define DYNLEC_DEBUG_REPORT(text) ::Dynlec::Report(__FILE__, __LINE__, text)
#else
#define DYNLEC_DEBUG_REPORT() __noop
#endif

#ifdef DYNLEC_CUSTOM_FATAL
#define DYNLEC_HANDLE_FATAL(text) DYNLEC_DEBUG_REPORT(text); ::Dynlec::HandleFatal()
#else
#define DYNLEC_HANDLE_FATAL(text) DYNLEC_DEBUG_REPORT(text)
#endif

namespace Dynlec
{
#ifdef _DEBUG
	bool HasConsoleWindow()
	{
		DWORD processID;
		GetWindowThreadProcessId(GetConsoleWindow(), &processID);
		return GetCurrentProcessId() == processID;
	}

	void Report(
		std::string file, 
		int line, 
		std::string text)
	{
		if (HasConsoleWindow())
		{
			std::cout
				<< "File: \"" << file << "\""
				<< "Line: \"" << line << "\""
				<< "Text: \"" << text << "\""
				<< std::endl;
		}
		else
		{
			_CrtDbgReport(
				_CRT_ERROR,
				file.c_str(),
				line,
				NULL,
				"$ls",
				text.c_str());
		}
	}
#endif
#ifdef DYNLEC_CUSTOM_FATAL
	extern std::function<void()> HandleFatal;
#endif
}
