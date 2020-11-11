#pragma once

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_map>

/*
environement:
- name [required]
	Specifies the name used in import
- binary_name [optional]
	Set binary dll name. If not specified the name is used instead
*/
class Library
{
public:
	Library(std::filesystem::path& path)
		:
		path(std::move(path))
	{
		parse();

		if (environment.find("name") == environment.end())
		{
			std::ostringstream oss;
			oss << "Library is missing name \"";
			oss << path << "\"";

			throw std::exception(oss.str().c_str());
		}
	}

	void loadBinary(std::vector<std::filesystem::path>& pathBinaryLibraries)
	{
		const std::string& name = getBinaryName();

		bool binaryFound = false;
		for (const std::filesystem::path& path : pathBinaryLibraries)
			if (path.stem() == name)
			{
				setBinary(path);
				return;
			}

		std::ostringstream oss;
		oss << "Library binary not found \"";
		oss << path << "\"";

		throw std::exception(oss.str().c_str());
	}

	const std::string& getBinaryName()
	{
		auto name = environment.find("binary_name");

		return name == environment.cend()
			? environment["name"]
			: name->second;
	}

	const std::string& getName()
	{
		return environment["name"];
	}

private:
	std::vector<unsigned char> binary;

	std::filesystem::path path;
	std::unordered_map<std::string, std::string> environment;

	void parse();

	void setBinary(const std::filesystem::path& location)
	{
		binary.reserve(std::filesystem::file_size(location));

		std::ifstream file(location);
		std::istream_iterator<unsigned char> start(file), end;
		binary.assign(start, end);
	}
};
