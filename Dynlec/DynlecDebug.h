#pragma once

#include "DynlecCommon.h"

#ifdef _DEBUG
#define DYNLEC_DEBUG_REPORT(text, ...) ::Dynlec::Debug::Report(__FILE__, __LINE__, text, __VA_ARGS__)
#define DYNLEC_DEBUG_INFO(text, ...) ::Dynlec::Debug::Info(__FILE__, __LINE__, text, __VA_ARGS__) 
#else
#define DYNLEC_DEBUG_REPORT(...) __noop
#define DYNLEC_DEBUG_INFO(...) __noop
#endif

#ifdef DYNLEC_CUSTOM_FATAL
#define DYNLEC_HANDLE_FATAL(text, ...) DYNLEC_DEBUG_REPORT(text, __VA_ARGS__); ::Dynlec::HandleFatal()
#else
#define DYNLEC_HANDLE_FATAL(text, ...) DYNLEC_DEBUG_REPORT(text, __VA_ARGS__)
#endif

namespace Dynlec
{
#ifdef _DEBUG
	namespace Debug
	{
		template <int i>
		void ExpandArguments(std::ostream& os)
		{
		}

		template <int i, typename Arg, typename... Args>
		void ExpandArguments(std::ostream& os, Arg arg, Args... args)
		{
			os << ", Arg";
			os << i;
			os << ": \"";
			os << arg;
			os << "\"";
			ExpandArguments<i + 1, Args...>(os, args...);
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
			ExpandArguments<0, Args...>(std::cout, args...);
			std::cout << std::endl;
		}

		inline bool HasConsoleWindow()
		{
			return GetConsoleWindow() != NULL;
		}

		template <typename... Args>
		void Report(
			std::string file,
			int line,
			std::string text,
			Args... args)
		{
			if (HasConsoleWindow())
			{
				std::cout
					<< "File: \"" << file << "\""
					<< ", Line: \"" << line << "\""
					<< ", Text: \"" << text << "\"";
				ExpandArguments<0, Args...>(std::cout, args...);
				std::cout << std::endl;
			}
			else
			{
				std::stringstream ss;
				ss << "File: \"" << file << "\"";
				ss << ", Line: \"" << line << "\"";
				ss << ", Text: \"" << text << "\"";
				ExpandArguments<0, Args...>(ss, args...);

				_CrtDbgReport(
					_CRT_ERROR,
					file.c_str(),
					line,
					NULL,
					"$ls",
					ss.str().c_str());
			}
		}
	}
#endif
#ifdef DYNLEC_CUSTOM_FATAL
	extern std::function<void()> HandleFatal;
#endif
}
