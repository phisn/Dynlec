#include "Library.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define LIBRARY_FILE_ENDING ".dynlec"

bool isSourceFile(std::filesystem::path& path)
{
	return path.extension() == ".cpp"
		|| path.extension() == ".cxx"
		|| path.extension() == ".c++"
		|| path.extension() == ".h"
		|| path.extension() == ".hpp"
		|| path.extension() == ".hxx"
		|| path.extension() == ".h++";
}

void compile();

int main()
{
	try 
	{
		compile();
		std::cout << "Compilation succeeded" << std::endl;
	}
	catch (std::exception e)
	{
		std::cout << "Compilation failed" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

void MapDirectory(
	std::filesystem::recursive_directory_iterator& iterator,
	std::vector<std::filesystem::path>& pathLibraries,
	std::vector<std::filesystem::path>& pathSources);

void compile()
{
	std::vector<std::filesystem::path> pathLibraries;
	std::vector<std::filesystem::path> pathSources;

	std::filesystem::recursive_directory_iterator iterator("");

	MapDirectory(iterator, pathLibraries, pathSources);

	std::vector<std::unique_ptr<Library>> libraries;

	for (std::filesystem::path& path : pathLibraries)
	{
		libraries.push_back(std::make_unique<Library>(path));
	}


}

void MapDirectory(
	std::filesystem::recursive_directory_iterator& iterator, 
	std::vector<std::filesystem::path>& pathLibraries, 
	std::vector<std::filesystem::path>& pathSources)
{
	for (const std::filesystem::directory_entry& directory : iterator)
	{
		std::filesystem::path extension = directory.path().extension();

		if (extension == LIBRARY_FILE_ENDING)
		{
			pathLibraries.push_back(directory);
			continue;
		}

		if (isSourceFile(extension))
		{
			pathSources.push_back(directory.path());
			continue;
		}
	}
}
