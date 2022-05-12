// HashColon config
#include <HashColon/Helper.hpp>
// std libraries
#include <chrono>
#include <ctime>
#include <iomanip>
// check filesystem support
#if defined __has_include
#if __has_include(<filesystem>)
#include <filesystem>
#define HASHCOLON_USING_FILESYSTEM using namespace std::filesystem
#define HASHCOLON_FILESYSTEM_STD
// #elif __has_include(<experimental/filesystem>)
// #include <experimental/filesystem>
// #define HASHCOLON_USING_FILESYSTEM using namespace std::experimental::filesystem
#elif __has_include(<boost/filesystem.hpp>)
#define HASHCOLON_USING_FILESYSTEM using namespace boost::filesystem
#define HASHCOLON_FILESYSTEM_BOOST
#include <boost/filesystem.hpp>
#else
#error C++17 support or boost library required.
#endif
#else
#error C++ compiler with __has_include support required.
#endif

#include <mutex>
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
#if defined HASHCOLON_FILESYSTEM_STD
		std::error_code ec;
#elif defined HASHCOLON_FILESYSTEM_BOOST
		boost::system::error_code ec;
#else
#error Neither std::filesystem nor boost::filesystem is available!
#endif
		if (exists(p))
		{
			remove_all(p, ec);
			return true;
		}
		else
			return false;
	}
}

// time point functions
namespace HashColon
{
	// mutex for ctime functions
	mutex ctime_mx;

	//{ TimePoint functions
	void TimePoint::fromString(string datetimeStr, const string formatStr)
	{
		tm temp_tm = {0};
		stringstream ss(datetimeStr.c_str());
		ss >> get_time(&temp_tm, formatStr.c_str());

		// check if the datetimestr satisfies the format
		if (ss.fail() || !ss)
			throw out_of_range("datetime string out of range ( HashColon::Feline::Types::Timepoint::fromString() ).");
		else
		{
			time_t temptime_t;

			// unfortunately, mktime and localtime from ctime is not threadsafe.
			// therefore a lock should be provided.
			// refer to the following link for more information about tregedic behavior of mktime & localtime
			// https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
			{
				lock_guard<mutex> lock_mx(ctime_mx);
				temptime_t = mktime(&temp_tm);
			}
			auto temp_tp = chrono::system_clock::from_time_t(temptime_t);
			(*this) = temp_tp;
		}
	}

	string TimePoint::toString(const string formatStr) const
	{
		time_t this_C = chrono::system_clock::to_time_t(*this);
		stringstream ss;

		// unfortunately, mktime and localtime from ctime is not threadsafe.
		// therefore a lock should be provided.
		// refer to the following link for more information about tregedic behavior of mktime & localtime
		// https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
		{
			lock_guard<mutex> lock_mx(ctime_mx);
			ss << put_time(localtime(&this_C), formatStr.c_str());
		}
		return ss.str();
	}

}