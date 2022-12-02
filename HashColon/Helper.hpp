#ifndef HASHCOLON_HELPER
#define HASHCOLON_HELPER

// std libraries
#include <chrono>
#include <functional>
#include <string>
#include <vector>
// dependant external libraries
/* boost libraries are excluded */
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/type_index.hpp>

#ifdef __GNUC__
#include <unistd.h>
#include <sys/syscall.h>
#endif

// Filesystem helper functions
namespace HashColon::Fs
{
	// Get files in the given directories 'recursively'
	std::vector<std::string> GetFilesInDirectories(
		const std::vector<std::string> &iDirs,
		const std::vector<std::string> additonalFiles = {},
		const std::string filterRegexStr = ".*");

	std::vector<std::string> GetFilesFromPaths(
		const std::vector<std::string> &iPaths,
		const std::string filterRegexStr = ".*");

	// Get files in the given directory 'recursively'
	std::vector<std::string> GetFilesInDirectory(
		const std::string iDir,
		const std::vector<std::string> additonalFiles = {},
		const std::string filterRegexStr = ".*");

	std::string RefinedAbsolutePathStr(const std::string iPathString);

	bool BuildDirectoryStructure(const std::string iDirectoryPathString);

	bool RemoveAllInDirectory(const std::string iDirectoryPathString);
}

// String helper functions
namespace HashColon::String
{
	// trim from left
	inline std::string &TrimStart(std::string &s, const char *t = " \t\n\r\f\v")
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from right
	inline std::string &TrimEnd(std::string &s, const char *t = " \t\n\r\f\v")
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from left & right
	inline std::string &Trim(std::string &s, const char *t = " \t\n\r\f\v")
	{
		return TrimStart(TrimEnd(s, t), t);
	}

	// copying versions
	inline std::string TrimStartCopy(std::string s, const char *t = " \t\n\r\f\v")
	{
		return TrimStart(s, t);
	}

	inline std::string TrimEndCopy(std::string s, const char *t = " \t\n\r\f\v")
	{
		return TrimEnd(s, t);
	}

	inline std::string TrimCopy(std::string s, const char *t = " \t\n\r\f\v")
	{
		return Trim(s, t);
	}

	/* functions using boost libraries are excluded */

	// split string
	inline std::vector<std::string> Split(std::string s, const char *spliter)
	{
		std::vector<std::string> re;
		boost::split(re, s, boost::is_any_of(spliter));
		return re;
	}

	// case convolution
	inline std::string &ToLower(std::string &s)
	{
		boost::to_lower(s);
		return s;
	}

	inline std::string &ToUpper(std::string &s)
	{
		boost::to_upper(s);
		return s;
	}

	inline std::string ToLowerCopy(std::string s)
	{
		return ToLower(s);
	}

	inline std::string ToUpperCopy(std::string s)
	{
		return ToUpper(s);
	}

}

// Timepoint
namespace HashColon
{
	// TimeInterval definition as duration
	using Duration = std::chrono::duration<
		std::chrono::system_clock::rep,
		std::chrono::system_clock::period>;

	// TimePoint definition
	class TimePoint : public std::chrono::time_point<std::chrono::system_clock>
	{
	private:
		inline static std::string defaultFormat = "%Y-%m-%d %T";
	
	public:
		constexpr TimePoint()
			: std::chrono::time_point<std::chrono::system_clock>(){};

		constexpr explicit TimePoint(const Duration &d)
			: std::chrono::time_point<std::chrono::system_clock>(d){};

		template <class Duration2>
		constexpr TimePoint(const time_point<std::chrono::system_clock, Duration2> &t)
			: std::chrono::time_point<std::chrono::system_clock>(t){};

		inline TimePoint(std::string datetimeStr) { fromString(datetimeStr); };
		inline TimePoint(std::pair<std::string, std::string> timedef) { fromString(timedef.first, timedef.second); };
		
		inline static void SetDefaultFormat(std::string frmstr) { defaultFormat = frmstr; };
		inline static TimePoint Now() { return std::chrono::system_clock::now(); };

	public:
		inline TimePoint &operator=(std::string datetimeStr)
		{
			fromString(datetimeStr);
			return (*this);
		};
		inline TimePoint &operator=(std::pair<std::string, std::string> timedef)
		{
			fromString(timedef.first, timedef.second);
			return (*this);
		};
		void fromString(std::string datetimeStr, const std::string formatStr = defaultFormat);
		std::string toString(const std::string formatStr = defaultFormat) const;
	};

}

// Thread/Process performance
namespace HashColon
{
	#ifdef __GNUC__
	std::pair<float, float> GetCpuMem_fromPID(int pid);
	inline size_t GetPID() { return (size_t)::getpid(); }
    	inline size_t GetTID() { return (size_t)::syscall(SYS_gettid); }
	#endif
}

// time guard for periodic processes
namespace HashColon
{
	class TimeGuard
	{
	private:
		TimePoint _start;
		Duration _period;

	public:
		TimeGuard(const Duration period);
		void Restart();
		void ResetPeriod(const Duration period);
		void ResetPeriod_sec(const size_t period_sec);
		void ResetPeriod_millisec(const size_t period_millisec);
		void ResetPeriod_nanosec(const size_t period_nanosec);
		void CheckPeriod(
			std::function<void(void)> ok = []() {},
			std::function<void(void)> period_violated = []() {});
		void CheckAndContinuePeriod(
			std::function<void(void)> ok = []() {},
			std::function<void(void)> period_violated = []() {});
		TimePoint GetPeriodStartTime() { return _start; };
	};
}

#define AsLambda(func) [&](auto&&... args) -> decltype(func(std::forward<decltype(args)>(args)...)) { return func(std::forward<decltype(args)>(args)...); }

//// Typenames helper functions
// namespace HashColon::TypeName
//{
//	template <typename T>
//	inline std::string ShortTypename()
//	{
//		return HashColon::String::Split(boost::typeindex::type_id_with_cvr<T>.pretty_name()).back();
//	}
//
//	template <typename T>
//	inline std::string LongTypename()
//	{
//		return boost::typeindex::type_id_with_cvr<T>.pretty_name();
//	}
// }

#endif
