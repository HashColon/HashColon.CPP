#ifndef HASHCOLON_HELPER_FILEUTILITY_HPP
#define HASHCOLON_HELPER_FILEUTILITY_HPP

#include <string>
#include <vector>

namespace HashColon::Helper
{
	// Get files in the given directories 'recursively'
	std::vector<std::string> GetFilesInDirectories(
		std::vector<std::string>& iDirs,
		std::vector<std::string> additonalFiles = {}, std::string filterRegexStr = ".*");


	std::vector<std::string> GetFilesFromPaths(
		std::vector<std::string>& iPaths, std::string filterRegexStr = ".*");

	// Get files in the given directory 'recursively'
	std::vector<std::string> GetFilesInDirectory(
		std::string iDir,
		std::vector<std::string> additonalFiles = {}, std::string filterRegexStr = ".*");

	std::string RefinedAbsolutePathStr(std::string iPathString);

	bool BuildDirectoryStructure(std::string iDirectoryPathString);

	bool RemoveAllInDirectory(std::string iDirectoryPathString);
}

#endif

