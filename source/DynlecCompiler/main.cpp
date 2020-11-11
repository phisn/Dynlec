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

void compile(std::string libraryPath, std::string sourcePath);

void Prepare(std::string& sourcePath, std::string& libraryPath, std::vector<std::filesystem::path>& pathSources, std::vector<std::unique_ptr<Library>>& libraries);

void LoadLibraryBinaries(std::string& libraryPath, std::vector<std::unique_ptr<Library>>& libraries);

void FindLibraryBinaries(std::string& libraryPath, std::vector<std::filesystem::path>& pathBinaryLibraries);

int main()
{
	try 
	{
		compile("dynleclib", ".");
		std::cout << "Compilation succeeded" << std::endl;
	}
	catch (std::exception e)
	{
		std::cout << "Compilation failed" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

void MapDirectory(
	std::string sourcePath,
	std::vector<std::filesystem::path>& pathLibraries,
	std::vector<std::filesystem::path>& pathSources);

void compile(std::string libraryPath, std::string sourcePath)
{
	std::vector<std::unique_ptr<Library>> libraries;
	std::vector<std::filesystem::path> pathSources;

	Prepare(sourcePath, libraryPath, pathSources, libraries);

	std::regex pattern("^\\s*//:dynlec:(.*):(.*)");
	std::smatch matches;

	for (std::filesystem::path& source : pathSources)
	{
		std::vector<std::string> lines;
		std::ifstream file(source);

		for (std::string line; std::getline(file, line);)
			lines.push_back(std::move(line));

		for (int i = 0; i < lines.size(); ++i)
		{
			if (std::regex_match(lines[i], matches, pattern))
			{
				if (matches[0] == "import")
				{
					auto library = std::find_if(libraries.begin(), libraries.end(),
						[&matches](std::unique_ptr<Library> library)
						{
							return matches[1] == library->getName();
						});



					continue;
				}

				std::ostringstream oss;
				oss << "failed to parse dynlec call \"";
				oss << source << "\"";
				oss << "at line (" << i << ")";

				throw std::exception(oss.str().c_str());
			}
		}


	}
}

//:dynlec:import:file

void Prepare(std::string& sourcePath, std::string& libraryPath, std::vector<std::filesystem::path>& pathSources, std::vector<std::unique_ptr<Library>>& libraries)
{
	std::vector<std::filesystem::path> pathLibraries;

	MapDirectory(sourcePath, pathLibraries, pathSources);

	for (std::filesystem::path& path : pathLibraries)
	{
		libraries.push_back(std::make_unique<Library>(path));
	}

	LoadLibraryBinaries(libraryPath, libraries);
}

void LoadLibraryBinaries(std::string& libraryPath, std::vector<std::unique_ptr<Library>>& libraries)
{
	std::vector<std::filesystem::path> pathBinaryLibraries;

	FindLibraryBinaries(libraryPath, pathBinaryLibraries);

	for (std::unique_ptr<Library>& library : libraries)
	{
		library->loadBinary(pathBinaryLibraries);
	}
}

void FindLibraryBinaries(std::string& libraryPath, std::vector<std::filesystem::path>& pathBinaryLibraries)
{
	std::filesystem::recursive_directory_iterator libraryIterator(libraryPath);

	for (const std::filesystem::directory_entry& entry : libraryIterator)
	{
		if (entry.path().extension() == ".dll")
			pathBinaryLibraries.push_back(entry.path());
	}
}

void MapDirectory(
	std::string sourcePath,
	std::vector<std::filesystem::path>& pathLibraries, 
	std::vector<std::filesystem::path>& pathSources)
{
	std::filesystem::recursive_directory_iterator iterator(sourcePath);

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
