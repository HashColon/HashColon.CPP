#include <HashColon/Helper.hpp>
// std libraries
#include <chrono>
#ifdef __GNUC__
#define __USE_POSIX
#define gmtime_threadsafe gmtime_r
#define localtime_threadsafe localtime_r
#elif _MSC_VER
#define gmtime_threadsafe gmtime_s
#define localtime_threadsage localtime_s
#endif
#include <ctime> // to use threadsafe function gmtime_r, localtime_r
#undef __USE_POSIX
#include <iomanip>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
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

	TimePoint::TimePoint(string datetimeStr)
	{
		fromString(datetimeStr);
	}
	TimePoint::TimePoint(pair<string, string> timedef)
	{
		fromString(timedef.first, timedef.second);
	}

	void TimePoint::SetDefaultFormat(string frmstr)
	{
		defaultFormat = frmstr;
	}
	string TimePoint::GetDefaultFormat()
	{
		return defaultFormat;
	}

	TimePoint TimePoint::Now()
	{
		return chrono::system_clock::now();
	};

	TimePoint TimePoint::UtcNow()
	{
		return Now().Local2Utc();
	}

	TimePoint &TimePoint::operator=(string datetimeStr)
	{
		fromString(datetimeStr);
		return (*this);
	};
	TimePoint &TimePoint::operator=(pair<string, string> timedef)
	{
		fromString(timedef.first, timedef.second);
		return (*this);
	};

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
			// unfortunately, mktime and localtime from ctime is not threadsafe.
			// therefore a lock should be provided.
			// refer to the following link for more information about tregedic behavior of mktime & localtime
			// https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
			lock_guard<mutex> lock_mx(ctime_mx);
			time_t temptime_t;

			temptime_t = mktime(&temp_tm);
			auto temp_tp = chrono::system_clock::from_time_t(temptime_t);
			(*this) = temp_tp;
		}
	}

	string TimePoint::toString(const string formatStr) const
	{
		// unfortunately, mktime and localtime from ctime is not threadsafe.
		// therefore a lock should be provided.
		// refer to the following link for more information about tregedic behavior of mktime & localtime
		// https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
		lock_guard<mutex> lock_mx(ctime_mx);
		time_t this_C = chrono::system_clock::to_time_t(*this);
		struct tm buf;
		struct tm tm_lt = (*localtime_threadsafe(&this_C, &buf));
		stringstream ss;

		ss << put_time(&tm_lt, formatStr.c_str());
		return ss.str();
	}

	TimePoint TimePoint::Local2Utc()
	{
		// unfortunately, mktime and localtime from ctime is not threadsafe.
		// therefore a lock should be provided.
		// refer to the following link for more information about tregedic behavior of mktime & localtime
		// https://stackoverflow.com/questions/16575029/localtime-not-thread-safe-but-okay-to-call-in-only-one-thread
		lock_guard<mutex> lock_mx(ctime_mx);
		struct tm buf;
		time_t this_C = chrono::system_clock::to_time_t(*this);
		struct tm tm_gt = (*gmtime_threadsafe(&this_C, &buf));
		time_t utc = mktime(&tm_gt);
		return TimePoint(chrono::system_clock::from_time_t(utc));
	}

	ostream &operator<<(ostream &lhs, const TimePoint &rhs)
	{
		lhs << rhs.toString();
		return lhs;
	}

	ostream &operator<<(ostream &lhs, const pair<TimePoint, string> rhs)
	{
		lhs << rhs.first.toString(rhs.second);
		return lhs;
	}
	ostream &operator>>(istream &lhs, const TimePoint &rhs)
	{
		lhs >> rhs;
	}
}

// Thread/Process performance
namespace HashColon
{
#ifdef __GNUC__
	pair<float, float> GetCpuMem_fromPID(int pid)
	{
		pair<float, float> re;
		stringstream syscmd;

		syscmd << "ps -p " << pid << " -o %cpu=,%mem=";
		FILE *ps_out = ::popen(syscmd.str().c_str(), "r");
		if (ps_out != nullptr)
		{
			if (2 == fscanf(ps_out, "%f %f", &re.first, &re.second))
				return re;
		}
		return {-100.0, -100.0};
	}
#elif _MSC_VER
#warning GetCpuMem_fromPID(): Not implemented for MSVC
#else
#warning GetCpuMem_fromPID: Undefined compiler
#endif

}

// time guard for periodic processes
namespace HashColon
{
	TimeGuard::TimeGuard(const Duration period)
	{
		ResetPeriod(period);
		Restart();
	};

	void TimeGuard::Restart()
	{
		_start = std::chrono::system_clock::now();
	}

	void TimeGuard::ResetPeriod(const Duration period)
	{
		_period = period;
	}

	void TimeGuard::ResetPeriod_sec(const size_t period_sec)
	{
		ResetPeriod(std::chrono::seconds(period_sec));
	}

	void TimeGuard::ResetPeriod_millisec(const size_t period_millisec)
	{
		ResetPeriod(std::chrono::milliseconds(period_millisec));
	}

	void TimeGuard::ResetPeriod_nanosec(const size_t period_nanosec)
	{
		ResetPeriod(std::chrono::nanoseconds(period_nanosec));
	}

	void TimeGuard::CheckPeriod(std::function<void(void)> ok,
								std::function<void(void)> period_violated)
	{
		TimePoint now = std::chrono::system_clock::now();
		auto interval = now - _start;
		if (interval <= _period)
		{
			ok();
		}
		else
		{
			period_violated();
		}
	}

	void TimeGuard::CheckAndContinuePeriod(std::function<void(void)> ok,
										   std::function<void(void)> period_violated)
	{
		TimePoint now = std::chrono::system_clock::now();
		auto interval = now - _start;
		if (interval <= _period)
		{
			// run function ok()
			ok();		
			// wait until the period to be finished.
			std::this_thread::sleep_until(_start);
			// reset start time
			_start += _period;
		}
		else
		{
			// run function period_violated()
			period_violated();
			// reset start time
			_start = std::chrono::system_clock::now();
		}
	}

}
