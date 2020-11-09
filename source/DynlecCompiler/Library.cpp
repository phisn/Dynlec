#include "Library.h"

void Library::parse()
{
	std::regex pattern("(.*):(.*)[\\n\\r]*");
	std::smatch result{};

	std::ifstream file(path);
	std::string line;

	for (int i = 0; std::getline(file, line); ++i)
	{
		if (!std::regex_match(line, result, pattern))
		{
			std::ostringstream oss;
			oss << "Failed to parse \"";
			oss << path << "\" at line (";
			oss << i << ")";

			throw std::exception(oss.str().c_str());
		}

		environment[result[0]] = result[1];
	}
}
