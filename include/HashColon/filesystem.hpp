#ifndef _HG_HASHCOLON_FILESYSTEM
#define _HG_HASHCOLON_FILESYSTEM

#include <HashColon/header>

#include <string>
#include <vector>

namespace HashColon
{
    /**
     * @brief Get files from the given filepaths
     * @details
     * For given paths:
     *  * If the path is a file: Add the file to the result
     *  * If the path is a directory: Search 'recursively' to add all files matching the regex in the directory.
     *  * If the path is invalid: ignore
     * @param iPaths Paths to be searched.
     * @param filterRegexStr Regex to be applied to the files in directories
     * @return std::vector<std::string> restult filepaths
     */
    std::vector<std::string> GetFilesFromPaths(
        const std::vector<std::string> &iPaths,
        const std::string filterRegexStr = ".*");

    /**
     * @brief Get refined absolute path from relative path
     * @details
     * For current path /tmp/a/b/c1/d:
     * "../../c2/./e" => "/tmp/a/b/c2/e"
     * @param iPathString Any path in string
     * @return Refined absolute path in string
     */
    std::string RefinedAbsolutePathStr(const std::string iPathString);

    /**
     * @brief If a given directory does not exist, build one.
     * @details This functions creates directories 'recursively'
     * For example,
     * If a empty directory /home/hashcolon/a exists,
     * and "/home/hashcolon/a/b/c/d/e" is given as iPathString:
     * Then, this function creates directory b, c, d, e recursively.
     *
     * @param iDirectoryPathString Directory path to create
     * @return true If Directory already exists or created
     * @return false Failed to create the directory
     */
    bool BuildDirectoryStructure(const std::string iDirectoryPathString);

    /**
     * @brief Remove everything under the given path
     *
     * @param iDirectoryPathString Path to the directory
     * @return true If succeed to remove everything.
     * @return false Failed to remove everything.
     */
    bool RemoveAllInDirectory(const std::string iDirectoryPathString);
}

#endif