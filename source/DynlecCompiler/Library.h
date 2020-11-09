#pragma once

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_map>

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

private:
	std::filesystem::path path;
	std::unordered_map<std::string, std::string> environment;

	void parse();
};
