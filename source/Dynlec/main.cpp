#include "DynlecLoader.h"

#include <filesystem>
#include <fstream>

int main()
{
	int buffer_size = std::filesystem::file_size("helloworld.exe");
	char* buffer = new char[buffer_size];

	std::ifstream ifs("helloworld.exe", std::ios::binary);
	ifs.read(buffer, buffer_size);

	Dynlec::Library helloworld(buffer, buffer_size);
	// helloworld.callEntryPoint();
	// helloworld.getProcAddress("test");
}
