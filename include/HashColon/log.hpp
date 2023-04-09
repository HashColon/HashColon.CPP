#ifndef _HG_HASHCOLON_LOG
#define _HG_HASHCOLON_LOG

#include <HashColon/header>

#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>
#include <HashColon/exception.hpp>

namespace HashColon
{
    template <typename T, typename Policy>
    class LogStreamBuf
    {
    private:
        LogStreamBuf() = delete;
    };

    template <typename T>
    using TagTypePolicy = typename std::enable_if<std::is_enum<T>::value, T>::type;

    /** @brief Stream buffer for HashColon::Loggers.
     *  @details LogStreamBuf guarantees that a "message" is not interleaved by multiple threads.
     *           LogStreambuf locks the stream with the start of the message, release the lock when it syncs(flush, endl).
     *           The message is not printed nor flushed until the stream sync.
     *           Ex) Logger << "something" << "another something" << std::endl;
     * @tparam tagType Enum class type having all the tags for the logger
     */
    template <typename TagType>
    class LogStreamBuf<TagType, TagTypePolicy<TagType> > : public std::streambuf
    {
    public:
        /// @brief Alias for argument value type: a variant of char, int, string, double
        using ArgValue = std::variant<char, int, std::string, double>;

        /// @brief Alias for argument type: {tag: value}
        using ArgType = std::pair<TagType, ArgValue>;

        /// @brief Alias for argument list type
        using ArgListType = std::unordered_map<TagType, ArgValue>;

        /// @brief Alias for formatter function type: formatter function returns a formatted string using the log arguments
        using FormatterType = std::function<std::string(std::string, ArgListType)>;

        /// @brief Alias for filter function type: filter function returns if the given message should be printed, depending on the log arguments
        using FilterType = std::function<bool(ArgListType)>;

    protected:
        /**
         * @brief Override function for std::streambuf::xsputn
         * @details Writes characters from the array pointed to by s into the controlled output sequence,
         *          until either n characters have been written or the end of the output sequence is reached.
         *
         * @param s const char string
         * @param n length of the string
         * @return std::streamsize return the size of the printed string
         */
        virtual std::streamsize xsputn(const char *s, std::streamsize n) final override;

        /**
         * @brief Override function for std::streambuf::overflow
         * @details Virtual function called by other member functions to put a character into the controlled output sequence without changing the current position.
         *
         * @param c Character to be put
         * @return int_type The character put
         */
        virtual int_type overflow(int_type c = traits_type::eof()) final override;

        /**
         * @brief Override function for std::streambuf::sync
         *
         * @return int 0 if success, -1 if failed.
         */
        virtual int sync() final override;

    protected:
        /// @brief Internal stringstream used as a buffer.
        std::stringstream _ss;
        /// @brief Mutex for _ss
        std::mutex _mutex;

    public:
        /// @brief Argument lists
        ArgListType arguments;
        /// @brief Formatter function
        FormatterType formatter;
        /// @brief Filter function
        FilterType filter;
        /// @brief Output streams to pass the messages
        std::vector<std::shared_ptr<std::ostream> > streamList;

        /// @brief Create a new copied Logger with additional arguments
        /// @param args additional arguments
        /// @return new LogStreamBuf with additional arguments
        LogStreamBuf<TagType, TagTypePolicy<TagType> > withArgs(const ArgListType args) const;

        /// @brief Create a new copied Logger with new formatter function
        /// @param formatter new formatter function
        /// @return new LogStreamBuf with new formatter function
        LogStreamBuf<TagType, TagTypePolicy<TagType> > withFormatter(const FormatterType formatter) const;

        /// @brief Create a new copied Logger with new filter function
        /// @param filter new filter function
        /// @return new LogStreamBuf with new filter function
        LogStreamBuf<TagType, TagTypePolicy<TagType> > withFilter(const FilterType filter) const;

        /// @brief Equivalent of (*this)
        /// @return (*this)
        LogStreamBuf<TagType, TagTypePolicy<TagType> > operator()() const;

        /// @brief Equivalent of withArgs
        /// @param args additional arguments
        /// @return new LogStreamBuf with additional arguments
        LogStreamBuf<TagType, TagTypePolicy<TagType> > operator()(const ArgListType args) const;

        /// @brief Create a new copied Logger with new formatter function
        /// @param formatter new formatter function
        /// @return new LogStreamBuf with new formatter function
        LogStreamBuf<TagType, TagTypePolicy<TagType> > operator()(const FormatterType formatter) const;

        /// @brief Create a new copied Logger with new filter function
        /// @param filter new filter function
        /// @return new LogStreamBuf with new filter function
        LogStreamBuf<TagType, TagTypePolicy<TagType> > operator()(const FilterType filter) const;

        /// @brief Equivalent of withFormatter and withArgs
        /// @param formatter new formatter function
        /// @param args additional arguments
        /// @return new LogStreamBuf with new formatter function and new arguments
        LogStreamBuf<TagType, TagTypePolicy<TagType> > operator()(const FormatterType formatter, const ArgListType args) const;

        /// @brief Equivalent of withFilter and withArgs
        /// @param filter new filter function
        /// @param args additional arguments
        /// @return new LogStreamBuf with new filter function and new arguments
        LogStreamBuf<TagType, TagTypePolicy<TagType> > operator()(const FilterType filter, const ArgListType args) const;

        /// @brief Equivalent of withFormatter and withFilter and withArgs
        /// @param formatter new formatter function
        /// @param filter new filter function
        /// @return new LogStreamBuf with new formatter function and with new filter function
        LogStreamBuf<TagType, TagTypePolicy<TagType> > operator()(const FormatterType formatter, const FilterType filter) const;

        /// @brief Equivalent of withFormatter and withFilter and withArgs
        /// @param formatter new formatter function
        /// @param filter new filter function
        /// @param args additional arguments
        /// @return new LogStreamBuf with new formatter function and with new filter function and new arguments
        LogStreamBuf<TagType, TagTypePolicy<TagType> > operator()(const FormatterType formatter, const FilterType filter, const ArgListType args) const;

    public:
        /// @brief Copy constructor
        /// @param rhs Copied right hand side
        LogStreamBuf(const LogStreamBuf<TagType, TagTypePolicy<TagType> > &rhs)
            : _ss(), _mutex(),
              arguments(rhs.arguments),
              formatter(rhs.formatter), filter(rhs.filter),
              streamList(rhs.streamList){};

        /// @brief Constructor with arguments
        /// @param iStreams
        /// @param iFormatter
        /// @param iFilter
        /// @param iArgList
        LogStreamBuf(
            std::vector<std::shared_ptr<std::ostream> > iStreams = {},
            FormatterType iFormatter = nullptr,
            FilterType iFilter = nullptr,
            ArgListType iArgList = {})
            : _ss(), _mutex(),
              arguments(iArgList),
              formatter(iFormatter), filter(iFilter),
              streamList(iStreams){};
    };

    /**
     * @brief Base class for logging
     * @details To use this class, tag type(enum)/filter function/format function needs to be defined.
     *          Do not use this class for direct logging. use CommonLogger instead
     *          <Logger ingredients descriptions>
     *          TagType: enum for argument tags. all arguments will be passed as {tag, value} pairs
     *          Formatter: formatting function in form: string [func](string, { {tag, value}, ... } )
     *                     example)
     *                  string TimestampedMessages(string msg, { { tag::time, timevalue} } )
     *                  this formatter gets msg as original message, timevalue as given argument name time.
     *                  this formatter may give output as "[timevalue] msg"
     *          Filter: filter function in form: bool [func]( { {tag, value}, ... } )
     *                  this function determines whether the logging message should be shown or not.
     *                  example)
     *                  bool levelFilter( { { tag::lvl, level } } )
     *                  If the given level is over certain value, the function will return false.
     *                  Which will suppress the message.
     * @tparam TagType Enum class type having all the tags for the logger
     */
    template <typename TagType>
    class Logger : public std::ostream
    {
    public:
        /// @brief Alias for argument value type: a variant of char, int, string, double
        using ArgValue = std::variant<char, int, std::string, double>;

        /// @brief Alias for argument type: {tag: value}
        using ArgType = std::pair<TagType, ArgValue>;

        /// @brief Alias for argument list type
        using ArgListType = std::unordered_map<TagType, ArgValue>;

        /// @brief Alias for formatter function type: formatter function returns a formatted string using the log arguments
        using FormatterType = std::function<std::string(std::string, ArgListType)>;

        /// @brief Alias for filter function type: filter function returns if the given message should be printed, depending on the log arguments
        using FilterType = std::function<bool(ArgListType)>;

        /// @brief Internal LogStreamBuf
        LogStreamBuf<TagType, TagTypePolicy<TagType> > stream;

    public:
        /// @brief Copy constructor
        /// @param logger Copied object
        Logger(const Logger<TagType> &logger)
            : std::ostream(&stream), stream(logger.stream){};

        /// @brief Constructor using LogStreamBuf
        /// @param logStream
        Logger(const LogStreamBuf<TagType, TagTypePolicy<TagType> > *logStream)
            : std::ostream(&stream), stream(*logStream){};

        /// @brief Constructor using all parameters
        /// @param iStreams output stream list
        /// @param iFormatter formatter function
        /// @param iFilter filter function
        /// @param iArgList argument list
        Logger(
            std::vector<std::shared_ptr<std::ostream> > iStreams = {},
            FormatterType iFormatter = nullptr,
            FilterType iFilter = nullptr,
            ArgListType iArgList = {})
            : std::ostream(&stream),
              stream(iStreams, iFormatter, iFilter, iArgList){};

    public:
        /// @brief Create a new copied Logger with additional arguments
        /// @param args additional arguments
        /// @return new Logger with additional arguments
        Logger<TagType> withArgs(const ArgListType args) const;

        /// @brief Create a new copied Logger with new formatter function
        /// @param formatter new formatter function
        /// @return new Logger with new formatter function
        Logger<TagType> withFormatter(const FormatterType formatter) const;

        /// @brief Create a new copied Logger with new filter function
        /// @param filter new filter function
        /// @return new Logger with new filter function
        Logger<TagType> withFilter(const FilterType filter) const;

        /// @brief Equivalent of (*this)
        /// @return (*this)
        Logger<TagType> operator()() const;

        /// @brief Equivalent of withArgs
        /// @param args additional arguments
        /// @return new Logger with additional arguments
        Logger<TagType> operator()(const ArgListType args) const;

        /// @brief Create a new copied Logger with new formatter function
        /// @param formatter new formatter function
        /// @return new Logger with new formatter function
        Logger<TagType> operator()(const FormatterType formatter) const;

        /// @brief Create a new copied Logger with new filter function
        /// @param filter new filter function
        /// @return new Logger with new filter function
        Logger<TagType> operator()(const FilterType filter) const;

        /// @brief Equivalent of withFormatter and withArgs
        /// @param formatter new formatter function
        /// @param args additional arguments
        /// @return new Logger with new formatter function and new arguments
        Logger<TagType> operator()(const FormatterType formatter, const ArgListType args) const;

        /// @brief Equivalent of withFilter and withArgs
        /// @param filter new filter function
        /// @param args additional arguments
        /// @return new Logger with new filter function and new arguments
        Logger<TagType> operator()(const FilterType filter, const ArgListType args) const;

        /// @brief Equivalent of withFormatter and withFilter and withArgs
        /// @param formatter new formatter function
        /// @param filter new filter function
        /// @return new Logger with new formatter function and with new filter function
        Logger<TagType> operator()(const FormatterType formatter, const FilterType filter) const;

        /// @brief Equivalent of withFormatter and withFilter and withArgs
        /// @param formatter new formatter function
        /// @param filter new filter function
        /// @param args additional arguments
        /// @return new Logger with new formatter function and with new filter function and new arguments
        Logger<TagType> operator()(const FormatterType formatter, const FilterType filter, const ArgListType args) const;
    };
}

/// @brief LogUtils namespace provides tag/formatter/filter/helper functions for the loggers
namespace HashColon::LogUtils
{
    /// @brief Tags for the logging
    enum class Tag
    {
        /**
         * @brief type: string: Defines message type, or gives information about the origin of the message.
         * @details You can give any kind of string.
                    For example, CommonLogger uses following type values.
                    Log: "Log" / Error: "Error" / Debug: "Debug" / Message: "Message"
         */
        type,

        /**
         * @brief file: string: Used only for error messages. Gives source file name where error is raised.
         * @details For example, { Tag::file, "LogUtils.hpp" }
                    To use compiler macro, use { Tag::file, __FILE__ }
         */
        file,

        /**
         * @brief line: string: Used only for error messages. Gives line number where error is raised.
         * @details For example, { Tag::file, "24" }
                    To use compiler macro, use { Tag::line, __LINE__ }
         */
        line,

        /**
         * @brief func: string: Used only for error messages. Gives function name where error is raised.
         * @details For example, { Tag::func, "ErrFormat()" }
                    To use compiler macro, use { Tag::file, __FUNC__ }
         */
        func,

        /**
         * @brief lvl: int: Defines message priority. If the lvl is greater than maxlvl the message is suppressed.
         * @details *** Recommendation for lvl values ***
                    - 1 : starting/finishing messages for important event.
                          such as reading large input data files, core computational steps, printing output files etc.
                    - 3 : primary steps in important method/algorithms.
                          for example, each steps in Bezier curve division can be logged.
                        - step 1: build front-part division matrix
                        - step 2: build end-part division matrix
                        - step 3: multiply coefficients with each matrix
                        - step 4: return results.
                    - 5 : miscellaneous logs
         */
        lvl,

        /**
         * @brief maxlvl: int: Defines message priority criteria. If the lvl is greater than maxlvl the message is suppressed.
         */
        maxlvl
    };

    /// @brief Alias for argument value type: a variant of char, int, string, double
    using ArgValue = std::variant<char, int, std::string, double>;

    /// @brief Alias for argument list type
    using ArgListType = std::unordered_map<Tag, ArgValue>;

    /**
     * @brief  Basic log format.
     * @details [timestamp][type] message
     *
     *      	ex) [2021-07-23 14:23:57.078 UTC][Log] blah....
     *
     * @param msg Message to print
     * @param args Other argument list
     * @return std::string Formatted message
     */
    std::string LogFormat(std::string msg, ArgListType args);

    /**
     * @brief Format for error logging
     * @details [timestamp][type]:@file#line
     *          \t	{func()}: message
     *
     *          ex) [2021-07-23 14:23:57.078 UTC][Error]: @LogUtils.hpp#38
     *                  {HashColon::LogUtils::ErrFormat()}: blah....
     * @param msg Message to print
     * @param args Other argument list
     * @return std::string Formatted message
     */
    std::string ErrFormat(std::string msg, ArgListType args);

    /**
     * @brief Prints nothing
     *
     * @param msg Message to print
     * @param args Other argument list
     * @return std::string An empty string
     */
    std::string NullFormat(std::string msg, ArgListType args);

    /**
     * @brief No formatted message.
     *
     * @param msg Message to print
     * @param args Other argument list
     * @return std::string Identical to msg
     */
    std::string BasicFormat(std::string msg, ArgListType args);

    /**
     * @brief Always true
     *
     * @param args Some logging argument lists
     * @return true Always returns true
     */
    bool PassFilter(ArgListType args);

    /**
     * @brief Always false
     *
     * @param args Some logging argument lists
     * @return false Always returns false
     */
    bool BlockFilter(ArgListType args);

    /**
     * @brief Filters message by checking the Tag::lvl and Tag::maxlvl from the arguments
     *
     * @param args Message arguments
     * @return true lvl > maxlvl
     * @return false lvl <= maxlvl
     */
    bool VerboseFilter(ArgListType args);

    /**
     * @brief Flashl: Flash line: A fragment which can be used to flash messages
     * @details Messages through Flashl is erased after the next message.
     * Recommended to show current working progress, such as things like:
     *  Working...[ xx.x%][***********...............................]
     * By using Flashl, the printed line can be modified instead of stacking bunch of messages in the terminal.
     *	This function is not tested with the file output.
     *	Please use this with CommonLogger::Message
     *	*** IMPORTANT *** : DO NOT USE endl with Flashl. It will not work.
     *	*** IMPORTANT *** : It is recommended to flush after all the progress is finished. For good.
     *	USAGE:  logger.Message << Flashl(msg);
     *	USAGE:  atomic<size_t> progressCnt{0};
     *		    #pragma omp parallel for
     *		    for( ... ){
     *		        // some kind of works...
     *				stringstream tempss;
     *				tempss << "working...: " << Frag::Percentage(++progressCnt, N);
     *				logger.Message << Flashl(tempss.str());
     *			}
     *			logger.Message << flush;
     *			logger.Log() << "Progress finished." >> endl;
     */
    class Flashl
    {
    private:
        /// @brief internal message to flash
        std::string _msg;

    public:
        /// @brief Constructor
        /// @param msg message to flash
        Flashl(std::string msg) : _msg(msg){};

        friend std::ostream &operator<<(std::ostream &lhs, Flashl rhs);
    };
    /// @brief Shift operator for Flashl
    /// @param lhs ostream object
    /// @param rhs Flashl object
    /// @return ostream
    std::ostream &operator<<(std::ostream &lhs, Flashl rhs);

    /**
     * @brief Simple percentage fragment
     * @details Fragments like: [100.0%], [45.6%]
     * USAGE: logger.Log() << Frag::Percentage(12, 100) << endl; // [ 12.0%]
     * USAGE: logger.Log() << Frag::Percentage(0.6582) << endl;  // [ 65.8%]
     */
    class Percentage
    {
    private:
        /// @brief Internal percentage value
        double _percentage;

    public:
        /// @brief Constructor with two integers: partial, total
        /// @param item_done Partial number of the items
        /// @param item_total Total number of the items
        Percentage(int item_done, int item_total);

        /// @brief Constructor with a ratio value in [0.0, 1.0] range
        /// @param percentage ratio value in [0.0, 1.0] range
        Percentage(double percentage);

        friend std::ostream &operator<<(std::ostream &lhs, Percentage rhs);
    };

    /// @brief Shift operator for Percentage
    /// @param lhs ostream object
    /// @param rhs Percentage object
    /// @return ostream
    std::ostream &operator<<(std::ostream &lhs, Percentage rhs);

    /// @brief Timestamp in UTC time
    class TimestampUtc
    {
    public:
        /// @brief Constructor
        TimestampUtc(){};
        friend std::ostream &operator<<(std::ostream &lhs, TimestampUtc rhs);
    };

    /// @brief Shift operator for TimestampUtc
    /// @param lhs ostream object
    /// @param rhs TimestampUtc object
    /// @return ostream
    std::ostream &operator<<(std::ostream &lhs, TimestampUtc rhs);
}

#ifndef __CODEINFO_TAGS__
/**
 * @brief __CODEINFO_TAGS__ : combines file, line, func as one. no need to give values(compiler automatically provides their values)
 * @details USAGE: CommonLogger.Error({ __CODEINFO_TAGS__ }) << "an example" << std::endl;
 */
#define __CODEINFO_TAGS__                           \
    {HashColon::LogUtils::Tag::file, __FILE__},     \
        {HashColon::LogUtils::Tag::line, __LINE__}, \
    {                                               \
        HashColon::LogUtils::Tag::func, __FUNC__    \
    }
#endif // !__CODEINFO_TAGS__

namespace HashColon
{
    /**
     * @brief An easy logger for general use
     * @details
     * USAGE: CommonLogger logger;
     *   logger.Log( { { tag::lvl, 1 }, ... } ) << "some kind of message" << std::endl;
     *   logger.Error( { __CODEINFO_TAGS__ } << "some kind of error message " << std::endl;
     *   logger.Debug( { { tag::lvl, 1 }, __CODEINFO_TAGS__ } << "some kind of debug message " << std::endl;
     *   logger.Message() << "blah" << std::endl;
     *
     * * CommonLogger uses tags/filter/formatter in HashColon::LogUtils
     * 	 For more information, refer HashColon::LogUtils
     */
    class CommonLogger final
    {
    public:
        /// @brief Parameters for CommonLogger
        struct _Params
        {
            /// @brief List of ostream for channel "Log"
            std::vector<std::shared_ptr<std::ostream> > logStreams;
            /// @brief List of ostream for channel "Error"
            std::vector<std::shared_ptr<std::ostream> > errorStreams;
            /// @brief List of ostream for channel "Debug"
            std::vector<std::shared_ptr<std::ostream> > debugStreams;
            /// @brief List of ostream for channel "Message"
            std::vector<std::shared_ptr<std::ostream> > messageStreams;
            /// @brief verbose level to suppress/show messages
            int verbose_level = std::numeric_limits<int>::max();
        };

    public:
        /// @brief Constructor
        /// @param params parameters
        CommonLogger(_Params params);

    public:
        /// @brief Logger for the channel "Log"
        Logger<LogUtils::Tag> Log;
        /// @brief Logger for the channel "Error"
        Logger<LogUtils::Tag> Error;
        /// @brief Logger for the channel "Debug"
        Logger<LogUtils::Tag> Debug;
        /// @brief Logger for the channel "Message"
        Logger<LogUtils::Tag> Message;
    };

    /// @brief CommonLogger which can be used globally
    extern CommonLogger GlobalLogger;

    namespace GlobalLogger_
    {
        /// @brief If invalid stream name is given for GlobalLogger_::SetStreams()
        HASHCOLON_NAMED_EXCEPTION_DEFINITION(InvalidStreamName);

        /// @brief Globally defined streams. Such as stdout(alias of cout), stderr(alias of cerr)
        extern std::unordered_map<std::string, const std::shared_ptr<std::ostream> > Streams;

        /// @brief Define initialization parameters of GlobalLogger
        /// @details This function also initializes the GlobalLogger_::Streams
        /// @param configFilePath Path to configuration file
        /// @param configNamespace Namespace of the configuration
        void Initialize(const std::string configFilePath = "",
                        const std::string configNamespace = "Log");

        /// @brief Insert stdout, stderr to GlobalStreams
        void InitializeStreams();

        /// @brief Convert list of stream names to streams
        /// @param vals List of stream names
        /// @param targetStreamList Output: vector of converted streams
        /// @param loggerName Name of the logger for targetStreamList. Mainly for error message.
        void ConvertToStreams(const std::vector<std::string> &vals,
                              std::vector<std::shared_ptr<std::ostream> > &targetStreamList,
                              const std::string loggerName);

    }

}

/* Template Implementations */
namespace HashColon
{
    template <typename T>
    std::streamsize LogStreamBuf<T, TagTypePolicy<T> >::xsputn(const char *s, std::streamsize n)
    {
        // if internal stringstream is empty, apply lock
        if (_ss.tellp() == std::streampos(0))
        {
            _mutex.try_lock();
        }

        // store string in _ss
        // _ss stream will flushed in _Logger::sync
        _ss << std::string(s, n);
        return n;
    }

    template <typename T>
    std::streambuf::int_type LogStreamBuf<T, TagTypePolicy<T> >::overflow(std::streambuf::int_type c)
    {
        if (c == traits_type::eof())
            return traits_type::eof();
        else
        {
            char_type ch = traits_type::to_char_type(c);
            return xsputn(&ch, 1) == 1 ? c : traits_type::eof();
        }
    }

    template <typename T>
    int LogStreamBuf<T, TagTypePolicy<T> >::sync()
    {
        if (filter(arguments))
        {
            std::string formattedString = formatter(_ss.str(), arguments);
            for (auto &aStream : streamList)
                (*aStream) << formattedString << std::flush;
        }

        // clear string after flush
        std::stringstream().swap(_ss);
        _mutex.unlock();
        return 0;
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::withArgs(const ArgListType args) const
    {
        // copy current object;
        LogStreamBuf tempLogStream = (*this);
        ArgListType tempArgs = args;
        tempArgs.insert(tempLogStream.arguments.begin(), tempLogStream.arguments.end());
        tempLogStream.arguments = tempArgs;
        // return temporary object using argument list
        return tempLogStream;
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::withFormatter(const FormatterType formatter) const
    {
        LogStreamBuf tempLogStream = (*this);
        tempLogStream.formatter = formatter;
        return tempLogStream;
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::withFilter(const FilterType filter) const
    {
        LogStreamBuf tempLogStream = (*this);
        tempLogStream.filter = filter;
        return tempLogStream;
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::operator()() const
    {
        return (*this);
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::operator()(const ArgListType args) const
    {
        return withArgs(args);
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::operator()(const FormatterType formatter) const
    {
        return withFormatter(formatter);
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::operator()(const FilterType filter) const
    {
        return withFilter(filter);
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::operator()(const FormatterType formatter, const ArgListType args) const
    {
        // copy current object & set formatter;
        LogStreamBuf tempLogStream = (*this);

        ArgListType tempArgs = args;
        tempArgs.insert(tempLogStream.arguments.begin(), tempLogStream.arguments.end());
        tempLogStream.arguments = tempArgs;

        tempLogStream.formatter = formatter;

        // return temporary object using argument list
        return tempLogStream;
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::operator()(const FilterType filter, const ArgListType args) const
    {
        // copy current object & set formatter;
        LogStreamBuf tempLogStream = (*this);

        ArgListType tempArgs = args;
        tempArgs.insert(tempLogStream.arguments.begin(), tempLogStream.arguments.end());
        tempLogStream.arguments = tempArgs;

        tempLogStream.filter = filter;

        // return temporary object using argument list
        return tempLogStream;
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::operator()(const FormatterType formatter, const FilterType filter) const
    {
        // copy current object & set formatter;
        LogStreamBuf tempLogStream = (*this);

        tempLogStream.formatter = formatter;
        tempLogStream.filter = filter;

        // return temporary object using argument list
        return tempLogStream;
    }

    template <typename T>
    LogStreamBuf<T, TagTypePolicy<T> > LogStreamBuf<T, TagTypePolicy<T> >::operator()(const FormatterType formatter, const FilterType filter, const ArgListType args) const
    {
        // copy current object & set formatter;
        LogStreamBuf tempLogStream = (*this);

        ArgListType tempArgs = args;
        tempArgs.insert(tempLogStream.arguments.begin(), tempLogStream.arguments.end());
        tempLogStream.arguments = tempArgs;

        tempLogStream.formatter = formatter;
        tempLogStream.filter = filter;

        // return temporary object using argument list
        return tempLogStream;
    }

    template <typename T>
    Logger<T> Logger<T>::withArgs(const ArgListType args) const
    {
        LogStreamBuf lsb = stream;

        ArgListType tempArgs = args;
        tempArgs.insert(lsb.arguments.begin(), lsb.arguments.end());
        lsb.arguments = tempArgs;

        return Logger<T>(&lsb);
    }

    template <typename T>
    Logger<T> Logger<T>::withFormatter(const FormatterType formatter) const
    {
        LogStreamBuf lsb = stream;
        lsb.formatter = formatter;
        return Logger<T>(&lsb);
    }

    template <typename T>
    Logger<T> Logger<T>::withFilter(const FilterType filter) const
    {
        LogStreamBuf lsb = stream;
        lsb.filter = filter;
        return Logger<T>(&lsb);
    }

    template <typename T>
    Logger<T> Logger<T>::operator()() const
    {
        return (*this);
    }

    template <typename T>
    Logger<T> Logger<T>::operator()(const ArgListType args) const
    {
        return withArgs(args);
    }

    template <typename T>
    Logger<T> Logger<T>::operator()(const FormatterType formatter) const
    {
        return withFormatter(formatter);
    }

    template <typename T>
    Logger<T> Logger<T>::operator()(const FilterType filter) const
    {
        return withFilter(filter);
    }

    template <typename T>
    Logger<T> Logger<T>::operator()(const FormatterType formatter, const ArgListType args) const
    {
        LogStreamBuf lsb = stream;

        ArgListType tempArgs = args;
        tempArgs.insert(lsb.arguments.begin(), lsb.arguments.end());
        lsb.arguments = tempArgs;

        lsb.formatter = formatter;

        return Logger<T>(&lsb);
    }

    template <typename T>
    Logger<T> Logger<T>::operator()(const FilterType filter, const ArgListType args) const
    {
        LogStreamBuf lsb = stream;

        ArgListType tempArgs = args;
        tempArgs.insert(lsb.arguments.begin(), lsb.arguments.end());
        lsb.arguments = tempArgs;

        lsb.filter = filter;

        return Logger<T>(&lsb);
    }

    template <typename T>
    Logger<T> Logger<T>::operator()(const FormatterType formatter, const FilterType filter) const
    {
        LogStreamBuf lsb = stream;
        lsb.formatter = formatter;
        lsb.filter = filter;
        return Logger<T>(&lsb);
    }

    template <typename T>
    Logger<T> Logger<T>::operator()(const FormatterType formatter, const FilterType filter, const ArgListType args) const
    {
        LogStreamBuf lsb = stream;

        ArgListType tempArgs = args;
        tempArgs.insert(lsb.arguments.begin(), lsb.arguments.end());
        lsb.arguments = tempArgs;

        lsb.formatter = formatter;
        lsb.filter = filter;

        return Logger<T>(&lsb);
    }
}

#endif