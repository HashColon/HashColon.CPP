#include <HashColon/filesystem.hpp>

#include <string>
#include <vector>
#include <HashColon/headers/filesystem>
#include <HashColon/headers/regex>

using std::string;
using std::vector;

// if boost::filesystem enabled
#ifdef HASHCOLON_HEADER_FILESYSTEM_BOOST
using boost::filesystem::absolute;
using boost::filesystem::canonical;
using boost::filesystem::create_directories;
using boost::filesystem::directory_entry;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::filesystem::is_regular_file;
using boost::filesystem::path;
using boost::filesystem::recursive_directory_iterator;
using boost::filesystem::remove_all;
using boost::system::error_code;
#else
using std::error_code;
using std::filesystem::absolute;
using std::filesystem::canonical;
using std::filesystem::create_directories;
using std::filesystem::directory_entry;
using std::filesystem::exists;
using std::filesystem::is_directory;
using std::filesystem::is_regular_file;
using std::filesystem::path;
using std::filesystem::recursive_directory_iterator;
using std::filesystem::remove_all;
#endif

// if boost::regex enabled,
#ifdef BOOST_RE_REGEX_HPP
using boost::regex;
using boost::regex_match;
using boost::smatch;
#else
using std::regex;
using std::regex_match;
using std::smatch;
#endif

namespace HashColon
{

    vector<string> GetFilesFromPaths(
        const vector<string> &iPaths, const string filterRegexStr)
    {
        // a new return vector re
        vector<string> re{};

        // regex & smatch object to filter files
        regex filterRegex(filterRegexStr);
        smatch base_match;

        // iterate through given paths
        for (const string &aPathStr : iPaths)
        {
            // define a path
            path aPath = aPathStr.c_str();

            // if current path is a file
            if (is_regular_file(aPath))
            {
                // add current file's path
                re.push_back(aPath.string());
            }
            // else if current path is a directory
            else if (is_directory(aPath))
            {
                // iterate
                for (const directory_entry aItem : recursive_directory_iterator(aPath.c_str()))
                {
                    // if aItem is a file
                    if (is_regular_file(aItem.path()))
                    {
                        // if filename matches the regex filter,
                        // add the file's full path to output
                        string filenameStr = aItem.path().filename().string();
                        if (regex_match(filenameStr, base_match, filterRegex))
                            re.push_back(aItem.path().string());
                    }
                }
            }
            // else, ignore current path
            else
                continue;
        }
        return re;
    }

    inline string RefinedAbsolutePathStr(const string iPathString)
    {
        /* references:
         * https://en.cppreference.com/w/cpp/filesystem/canonical
         * https://en.cppreference.com/w/cpp/filesystem/absolute
         */

        return canonical(absolute(path(iPathString))).string();
    }

    bool BuildDirectoryStructure(const string iDirectoryPathString)
    {
        path p = absolute(path(iDirectoryPathString));
        if (!exists(p))
            return create_directories(p);
        else
            return true;
    }

    bool RemoveAllInDirectory(const string iDirectoryPathString)
    {
        path p = absolute(path(iDirectoryPathString));
        error_code ec;

        if (exists(p))
        {
            remove_all(p, ec);
            return true;
        }
        else
            return false;
    }
}