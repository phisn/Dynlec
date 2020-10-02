#pragma once

#include "DynlecCommon.h"

#ifdef _DEBUG
#define DYNLEC_DEBUG_REPORT(text) ::Dynlec::Debug::Report(__FILE__, __LINE__, text)
#define DYNLEC_DEBUG_INFO(text, ...) ::Dynlec::Debug::Info(__FILE__, __LINE__, text, __VA_ARGS__) 
#else
#define DYNLEC_DEBUG_REPORT() __noop
#define DYNLEC_DEBUG_INFO()
#endif

#ifdef DYNLEC_CUSTOM_FATAL
#define DYNLEC_HANDLE_FATAL(text) DYNLEC_DEBUG_REPORT(text); ::Dynlec::HandleFatal()
#else
#define DYNLEC_HANDLE_FATAL(text) DYNLEC_DEBUG_REPORT(text)
#endif

namespace Dynlec
{
#ifdef _DEBUG
	namespace Debug
	{
		template <int i, typename Arg, typename... Args>
		void ExpandArguments(std::ostream& os, Arg arg, Args... args)
		{
			os << ", Arg" << i << ": \"" << arg << "\"";
			ExpandArguments<i + 1, Args...>(os, args...);
		}

		template <int i>
		void ExpandArguments(std::ostream& os)
		{
		}

		template <typename... Args>
		void Info(
			std::string file,
			int line,
			std::string text,
			Args... args)
		{
			std::cout
				<< "File: \"" << file << "\""
				<< ", Line: \"" << line << "\""
				<< ", Text: \"" << text << "\"";
			ExpandArguments<0, Args...>(std::cout, Args...);
			std::cout << std::endl;
		}

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
	}
#endif
#ifdef DYNLEC_CUSTOM_FATAL
	extern std::function<void()> HandleFatal;
#endif
}
