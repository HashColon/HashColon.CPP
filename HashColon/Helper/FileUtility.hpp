#ifndef HASHCOLON_HELPER_FILEUTILITY_HPP
#define HASHCOLON_HELPER_FILEUTILITY_HPP

#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>
#include <HashColon/Helper/Exception.hpp>

namespace HASHCOLON
{
	namespace Helper
	{
		std::vector<std::string> GetFilesInDirectories(
			std::vector<std::string>& iDirs,
			std::vector<std::string> additonalFiles = {}, std::string filterRegexStr = ".*");

		std::vector<std::string> GetFilesFromPaths(
			std::vector<std::string>& iPaths, std::string filterRegexStr = ".*");

		std::vector<std::string> GetFilesInDirectory(
			std::string iDir,
			std::vector<std::string> additonalFiles = {}, std::string filterRegexStr = ".*");

	}
}


#endif

