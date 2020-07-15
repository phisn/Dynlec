#include "Dynlec.h"

#ifdef _DEBUG
std::function<void(DWORD, const char*)> Dynlec::HandleFailedLoadLibrary =
[](DWORD error, const char* library)
{
	std::cout << "Failed to load library '" << library << "' (" << error << ")" << std::endl;
};

std::function<void(DWORD, const char*)> Dynlec::HandleFailedGetProcAddress =
[](DWORD error, const char* symbol)
{
	std::cout << "Failed to get proc address '" << symbol << "' (" << error << ")" << std::endl;
};
#else
std::function<void(DWORD, const char*)> Dynlec::HandleFailedLoadLibrary;
std::function<void(DWORD, const char*)> Dynlec::HandleFailedGetProcAddress;
#endif
