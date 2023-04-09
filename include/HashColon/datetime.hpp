#ifndef _HG_HASHCOLON_DATETIME
#define _HG_HASHCOLON_DATETIME

#include <HashColon/header>

#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <HashColon/exception.hpp>

namespace HashColon
{
    /// @brief mutex for all ctime functions...
    extern std::mutex ctime_mx;

    /// @brief Alias Duration as duration of system_clock
    using Duration = std::chrono::duration<
        std::chrono::system_clock::rep,
        std::chrono::system_clock::period>;

    /// @brief Alias TimePoint as time_point of system_clock
    /// @details All time should be managed as system_clock. Do not store utc values in this class.
    class TimePoint : public std::chrono::time_point<std::chrono::system_clock>
    {
    public:
        HASHCOLON_NAMED_EXCEPTION_DEFINITION(InvalidTimeZone);
        HASHCOLON_NAMED_EXCEPTION_DEFINITION(TimeStringParseFailed);

    private:
        /// @brief Default time string format used for conversion to/from strings.
        inline static std::string _defaultFormat = "%Y-%m-%d %T %Z";

    public:
        /// @brief Initialize the clocks and string default conversion formats
        /// @param defaultFormat Default time string format used for conversion to/from strings. If blank string is given, this value is set as the default value "%Y-%m-%d %T %Z".
        /// @param systemTimeZone Time zone string of the OS. If blank string is given, the function automatically detects the system time zone from OS.
        /// @param currentTimeZone Time zone string of the current app. If blank string is given, the function uses UTC as default time zone.
        /// @throw InvalidTimeZoneException Throws if systemTimeZone or currentTimeZone has invalid value.
        static void Initialize(const std::string defaultFormat,
                               const std::string systemTimeZone,
                               const std::string currentTimeZone);

        /// @brief Set the time zone of the current app
        /// @param currentTimeZone Time zone string of the current app.
        /// @throw InvalidTimeZoneException Throws if systemTimeZone or currentTimeZone has invalid value.
        static void SetCurrentTimeZone(const std::string currentTimeZone = "UTC");

        /// @brief Get the time zone of the current app
        /// @return _currentTimeZone
        static const std::string &GetCurrentTimeZone() noexcept;

        /// @brief Get the time zone of the OS
        /// @return _systemTimeZone
        static const std::string &GetSystemTimeZone() noexcept;

        /// @brief Set default time format string. _defaultFormat is not thread-safe.
        /// @param formatStr New time format string
        static void SetDefaultFormat(const std::string formatStr) noexcept;

        /// @brief Get default time format string.
        /// @return The value of _defaultFormat
        static const std::string &GetDefaultFormat() noexcept;

        /// @brief Returns current system time
        /// @return system time
        static TimePoint Now() noexcept;

    public:
        /// @brief Default blank constructor
        constexpr TimePoint()
            : std::chrono::time_point<std::chrono::system_clock>(){};

        /// @brief Constructor using duration value.
        /// @param d Duration value.
        constexpr explicit TimePoint(const Duration &d)
            : std::chrono::time_point<std::chrono::system_clock>(d){};

        /// @brief Copy constructor from time_point<system_clock>
        /// @tparam Duration2 type of the duration which the time_point is using
        /// @param t The copying value
        template <class Duration2>
        constexpr TimePoint(const time_point<std::chrono::system_clock, Duration2> &t)
            : std::chrono::time_point<std::chrono::system_clock>(t){};

        /// @brief Constructor from datetime string. _defaultFormat is used as the time format.
        /// @param datetimeStr The datetime string.
        TimePoint(const std::string datetimeStr);

        /// @brief Constructor from datetime string. _defaultFormat is used as the time format.
        /// @param timeDef Pair of datetime string, and the format string. {datetime, formatStr}.
        TimePoint(const std::pair<const std::string, const std::string> timeDef);

        /// @brief Build TimePoint from string
        /// @param datetimeStr datetime string
        /// @param formatStr format string
        void FromString(std::string datetimeStr, const std::string formatStr = _defaultFormat);

        /// @brief Convert TimePoint to string
        /// @param formatStr format string
        /// @return datetime string
        std::string ToString(const std::string formatStr = _defaultFormat) const;

        std::string ToUtcString(const std::string formatStr = _defaultFormat) const;
        std::string ToAppTimeString(const std::string formatStr = _defaultFormat) const;
    };

    /// @brief Maintains periodic actions
    class TimeGuard
    {
    private:
        /// @brief Period start time
        TimePoint _start;

        /// @brief Period of the action
        Duration _period;

    public:
        /// @brief Constructor
        /// @param period Period of the periodic action
        inline TimeGuard(const Duration period);

        /// @brief Reset the period start time as current time
        inline void Restart() noexcept;

        /// @brief Reset the period of this TimeGuard
        /// @param period Period
        inline constexpr void ResetPeriod(const Duration period) noexcept;

        /// @brief Wait until the period to be ended.
        /// @param ok Run this call back if the period has not elapsed.
        /// @param period_violated Run this call back if the period has elapsed.
        void WaitToPeriodEnd(
            std::function<void(void)> ok = []() {},
            std::function<void(void)> period_violated = []() {});

        /// @brief Get the period start time of the current period
        /// @return period start time
        inline constexpr const TimePoint &GetPeriodStartTime();
    };
}

/* inline function implementation */
namespace HashColon
{
    TimeGuard::TimeGuard(const Duration period)
    {
        ResetPeriod(period);
        Restart();
    }

    void TimeGuard::Restart() noexcept
    {
        _start = TimePoint::Now();
    };

    constexpr void TimeGuard::ResetPeriod(const Duration period) noexcept
    {
        _period = period;
    };

    constexpr const TimePoint &TimeGuard::GetPeriodStartTime()
    {
        return _start;
    };
}

#endif