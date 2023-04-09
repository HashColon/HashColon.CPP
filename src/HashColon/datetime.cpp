#include <HashColon/datetime.hpp>

#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>

// If C++20 is supported. We'll stick to it.
// Else, we need some help from Howward.Hinnant

// If C++20 is not supported, we need some help from Howward.Hinnant
#if __cplusplus < 202002L
#include <date/tz.h>

// If C++20 or higher supported, stick to <chrono> form time computation.
// However, we need regex support to convert time format string to std::formatter-style
#else
#include <HashColon/headers/regex>

#endif

using std::function;
using std::lock_guard;
using std::mutex;
using std::pair;
using std::shared_ptr;
using std::string;
using std::chrono::system_clock;

#if __cplusplus < 202002L // if c++20 not supported
using date::current_zone;
using date::locate_zone;
using date::parse;
using date::time_zone;
#else // if c++20 supported
using std::chrono::current_zone;
using std::chrono::locate_zone;
using std::chrono::parse;
using std::chrono::time_zone;
#ifdef HASHCOLON_HEADER_REGEX_BOOST
using boost::regex;
using boost::regex_replace;
#else
using std::regex;
using std::regex_replace;
#endif // HASHCOLON_HEADER_REGEX_BOOST
#endif // __cplusplus < 202002L

namespace HashColon
{
    // mutex for ctime functions
    mutex ctime_mx;

    // alias for time zone pointer
    using TimeZonePtr = const time_zone *;

    // Time zone of the OS
    inline static TimeZonePtr _systemTimeZone = nullptr;
    // Time zone of the current app
    inline static TimeZonePtr _currentTimeZone = nullptr;
    // A mutex for time zone parameters
    inline static mutex _tzmx;

    namespace _hidden
    {
#if __cplusplus >= 202002L // if c++20 Supported

        // for C++20, std::formatter uses slightly different format string from C, C++1a

        // detect all replacement tokens using regex: ex) %H, %0y , %EY
        regex replacements("%[%ntCyYbhBmdeaAuwgGVjUWDFxHIMSpRTrXzZcQq]|%O[ymdeuwVWHIMSz]|%E[yYxXzc]");

        /// @brief Convert C/C++1a time string format to C++20 formatter format
        /// @param formatStr
        /// @return
        string ConvertCpp20_TPFormatStr(const string &formatStr)
        {
            // replace all tokens with std::formatter style: %H => {0:%H}
            string stdFmtStr = "{0:$&}";
            return regex_replace(formatStr, replacements, stdFmtStr);
        }
#endif
    }

    void TimePoint::Initialize(const string defaultFormatStr,
                               const string systemTimeZoneStr,
                               const string currentTimeZoneStr)
    {
        // if default time string format is given, use it.
        // if not, use default value "%Y-%m-%d %T %Z".
        if (!defaultFormatStr.empty())
        {
            SetDefaultFormat(defaultFormatStr);
        }

        // if systemTimeZone is given, check and use it.
        if (!systemTimeZoneStr.empty())
        {
            // lock while access to timezones
            lock_guard<mutex> _lg(_tzmx);
            // check if the given time zone is valid
            try
            {
                _systemTimeZone = locate_zone(systemTimeZoneStr);
            }
            // if no time zone found, use os time zone
            catch (const std::runtime_error &e)
            {
                _systemTimeZone = current_zone();
            }
        }
        // if systemTimeZone is not given, get from the system
        else
        {
            // lock while access to timezones
            lock_guard<mutex> _lg(_tzmx);
            _systemTimeZone = current_zone();
        }

        // if currentTimeZone is not given, set to "UTC"
        const string cTzStr = currentTimeZoneStr.empty() ? "UTC" : currentTimeZoneStr;
        SetCurrentTimeZone(cTzStr);
    }

    void TimePoint::SetCurrentTimeZone(const string currentTimeZone)
    {
        // lock while access to timezones
        lock_guard<mutex> _lg(_tzmx);

        // check if the given time zone is valid
        try
        {
            _currentTimeZone = locate_zone(currentTimeZone);
        }
        // if no time zone found, throw
        catch (const std::runtime_error &e)
        {
            throw InvalidTimeZoneException("Invalid time zone given.");
        }
    }

    const string &TimePoint::GetCurrentTimeZone() noexcept
    {
        // lock while access to timezones
        lock_guard<mutex> _lg(_tzmx);
        return _currentTimeZone->name();
    }

    const string &TimePoint::GetSystemTimeZone() noexcept
    {
        // lock while access to timezones
        lock_guard<mutex> _lg(_tzmx);
        return _systemTimeZone->name();
    }

    void TimePoint::SetDefaultFormat(const string formatStr) noexcept
    {
        _defaultFormat = formatStr;
    }

    const string &TimePoint::GetDefaultFormat() noexcept
    {
        return _defaultFormat;
    }

    TimePoint TimePoint::Now() noexcept
    {
        return static_cast<TimePoint>(system_clock::now());
    }

    TimePoint::TimePoint(const string datetimeStr)
    {
        FromString(datetimeStr);
    }

    TimePoint::TimePoint(const pair<const string, const string> timeDef)
    {
        FromString(timeDef.first, timeDef.second);
    }

    void TimePoint::FromString(string datetimeStr, const string formatStr)
    {
        std::istringstream ss{datetimeStr};
        ss >> parse(formatStr, (*this));
        if (ss.fail() || !ss)
            throw TimeStringParseFailedException(
                "Failed to parse the given time string: " + datetimeStr + " : " + formatStr);
    }

    string TimePoint::ToString(const string formatStr) const
    {
#if __cplusplus < 202002L // if c++20 not supported
        return date::format(formatStr, *this);
#else // if c++20 supported
        return std::format(
            ConvertCpp20_TPFormatStr(formatStr), *this);
#endif
    }

    string TimePoint::ToUtcString(const string formatStr) const
    {
        // Convert to zoned_time point
        date::zoned_time<Duration> zonedTP("UTC", (*this));

#if __cplusplus < 202002L // if c++20 not supported
        return date::format(formatStr, zonedTP);
#else // if c++20 supported
        return std::format(
            ConvertCpp20_TPFormatStr(formatStr), zonedTP);
#endif
    }

    string TimePoint::ToAppTimeString(const string formatStr) const
    {
        // lock timezones
        lock_guard<mutex> _lg(_tzmx);

        // Convert to zoned_time point
        date::zoned_time<Duration> zonedTP(_currentTimeZone, (*this));

#if __cplusplus < 202002L // if c++20 not supported
        return date::format(formatStr, zonedTP);
#else // if c++20 supported
        return std::format(
            ConvertCpp20_TPFormatStr(formatStr), zonedTP);
#endif
    }

    void TimeGuard::WaitToPeriodEnd(function<void(void)> ok,
                                    function<void(void)> period_violated)
    {
        auto interval = TimePoint::Now() - _start;
        if (interval <= _period)
        {
            // run function ok()
            ok();
            // reset start time
            _start += _period;
            // wait until the period to be finished.
            std::this_thread::sleep_until(_start);
        }
        else
        {
            // run function period_violated()
            period_violated();
            // reset start time
            Restart();
        }
    }
}