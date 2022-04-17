// HashColon config
#include <HashColon/Helper.hpp>
// std libraries
// check filesystem support
#if defined __has_include
#if __has_include(<filesystem>)
#include <filesystem>
#define HASHCOLON_USING_FILESYSTEM using namespace std::filesystem
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
#define HASHCOLON_USING_FILESYSTEM using namespace std::experimental::filesystem
#elif __has_include(<boost/filesystem.hpp>)
#define HASHCOLON_USING_FILESYSTEM using namespace boost::filesystem
#include <boost/filesystem.hpp>
#else
#error C++17 support or boost library required.
#endif
#else
#error C++ compiler with __has_include support required.
#endif

#include <regex>
#include <sstream>
#include <string>
#include <vector>
// header file for this source file

using namespace std;

namespace HashColon::Fs
{
	// purpose
	// describe varied file name as

	vector<string> GetFilesInDirectory(
		const string iDir,
		const vector<string> additonalFiles,
		const string filterRegexStr)
	{
		vector<string> tmp;
		tmp.clear();
		tmp.push_back(iDir);
		return GetFilesInDirectories(tmp, additonalFiles, filterRegexStr);
	}

	vector<string> GetFilesFromPaths(
		const vector<string> &iPaths, const string filterRegexStr)
	{
		vector<string> re;
		re.clear();

		regex filterRegex(filterRegexStr);
		smatch base_match;

		// iterate
		for (const string &aPathStr : iPaths)
		{
			// we are not checking the validity of directory cause we have done it with cli11

			// we are using filesystem library here (std/std::experimental/boost)
			HASHCOLON_USING_FILESYSTEM;

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
						string filenamestr = aItem.path().filename().string();
						if (regex_match(filenamestr, base_match, filterRegex))
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

	vector<string> GetFilesInDirectories(
		const vector<string> &iDirs,
		const vector<string> additonalFiles,
		const string filterRegexStr)
	{
		vector<string> re;
		re.clear();

		regex filterRegex(filterRegexStr);
		smatch base_match;

		// iterate
		for (const string &aDirStr : iDirs)
		{
			// we are not checking the validity of directory cause we have done it with cli11

			// we are using filesystem library here (std/std::experimental/boost)
			HASHCOLON_USING_FILESYSTEM;

			// define a path
			path aDirPath = aDirStr.c_str();

			for (const directory_entry aItem : recursive_directory_iterator(aDirPath))
			{
				// if aItem is a file
				if (is_regular_file(aItem.path()))
				{
					// if filename matches the regex filter,
					// add the file's full path to output
					string filenamestr = aItem.path().filename().string();
					if (regex_match(filenamestr, base_match, filterRegex))
						re.push_back(aItem.path().string());
				}
			}
		}

		// if additionalFiles exists, add them
		if (!additonalFiles.empty())
			re.insert(re.end(), additonalFiles.begin(), additonalFiles.end());

		return re;
	}

	inline string RefinedAbsolutePathStr(const string iPathstring)
	{
		// we are using filesystem library here (std/std::experimental/boost)
		HASHCOLON_USING_FILESYSTEM;
		return canonical(absolute(path(iPathstring))).string();
	}

	bool BuildDirectoryStructure(const string iDirectoryPathString)
	{
		// we are using filesystem library here (std/std::experimental/boost)
		HASHCOLON_USING_FILESYSTEM;
		path p = absolute(path(iDirectoryPathString));
		if (!exists(p))
			return create_directories(p);
		else
			return true;
	}

	bool RemoveAllInDirectory(const string iDirectoryPathString)
	{
		// we are using filesystem library here (std/std::experimental/boost)
		HASHCOLON_USING_FILESYSTEM;
		path p = absolute(path(iDirectoryPathString));
		std::error_code ec;
		if (exists(p))
		{
			remove_all(p, ec);
			return true;
		}
		else
			return false;
	}
}