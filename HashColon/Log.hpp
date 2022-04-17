#ifndef HASHCOLON_LOG
#define HASHCOLON_LOG

// std libraries
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
// HashColon libraries
#include <HashColon/Exception.hpp>

// LogStreamBuf
namespace HashColon
{
	template <typename tagtype>
	class LogStreamBuf : public std::streambuf
	{
	public:
		using ArgValue = std::variant<char, int, std::string, double>;
		using ArgType = std::pair<tagtype, ArgValue>;
		using ArgListType = std::unordered_map<tagtype, ArgValue>;
		using FormatterType = std::function<std::string(std::string, ArgListType)>;
		using FilterType = std::function<bool(ArgListType)>;

	protected:
		virtual std::streamsize xsputn(const char *s, std::streamsize n) final override;

		virtual int_type overflow(int_type c = traits_type::eof()) final override;

		virtual int sync() final override;

	protected:
		ArgListType _args;
		FormatterType _formatter;
		FilterType _filter;
		std::stringstream _ss;
		std::vector<std::shared_ptr<std::ostream>> _streams;
		std::mutex _mutex;

	public:
		ArgListType &Arguments() { return _args; };
		FormatterType &Formatter() { return _formatter; };
		FilterType &Filter() { return _filter; };
		std::vector<std::shared_ptr<std::ostream>> &StreamList() { return _streams; };

		LogStreamBuf<tagtype> withArgs(const ArgListType args) const;
		LogStreamBuf<tagtype> withFormatter(const FormatterType formatter) const;
		LogStreamBuf<tagtype> withFilter(const FormatterType filter) const;

		LogStreamBuf<tagtype> operator()() const;
		LogStreamBuf<tagtype> operator()(const ArgListType args) const;
		LogStreamBuf<tagtype> operator()(const FormatterType formatter) const;
		LogStreamBuf<tagtype> operator()(const FilterType filter) const;

		LogStreamBuf<tagtype> operator()(const FormatterType formatter, const ArgListType args) const;
		LogStreamBuf<tagtype> operator()(const FilterType filter, const ArgListType args) const;
		LogStreamBuf<tagtype> operator()(const FormatterType formatter, const FilterType filter) const;
		LogStreamBuf<tagtype> operator()(const FormatterType formatter, const FilterType filter, const ArgListType args) const;

	public:
		LogStreamBuf(const LogStreamBuf<tagtype> &rhs)
			: _args(rhs._args), _formatter(rhs._formatter), _filter(rhs._filter),
			  _ss(), _streams(rhs._streams){};
		LogStreamBuf(
			std::vector<std::shared_ptr<std::ostream>> iStreams = {},
			FormatterType iFormattter = nullptr,
			FilterType iFilter = nullptr,
			ArgListType iArglist = {})
			: _args(iArglist), _formatter(iFormattter), _filter(iFilter),
			  _ss(), _streams(iStreams){};
	};
}

// Logger
namespace HashColon
{
	/*
		This is the base class for logging.
		To use this class, tag type(enum)/filter function/format function needs to be defined.
		Do not use this class for direct logging. use CommonLogger instead

		<Logger ingredients descriptions>
		tagtype: enum for argument tags. all arguments will be passed as {tag, value} pairs
		Formatter: formatting function in form: string [func](string, { {tag, value}, ... } )
				   example)
					   string TimestampedMessages(string msg, { { tag::time, timevalue} } )
					   this formatter gets msg as original message, timevalue as given argument name time.
					   this formatter may give output as "[timevalue] msg"
		Filter: filter function in form: bool [func]( { {tag, value}, ... } )
				this function determines wheter the logging message should be shown or not.
				example)
					   bool levelFilter( { { tag::lvl, level } } )
					   If the given level is over certain value, the function will return false.
					   Which will suppress the message.
	*/
	template <typename tagtype>
	class Logger : public std::ostream
	{
	public:
		using ArgValue = std::variant<char, int, std::string, double>;
		using ArgType = std::pair<tagtype, ArgValue>;
		using ArgListType = std::unordered_map<tagtype, ArgValue>;
		using FormatterType = std::function<std::string(std::string, ArgListType)>;
		using FilterType = std::function<bool(ArgListType)>;

	protected:
		LogStreamBuf<tagtype> _internalStream;

	public:
		Logger(const Logger<tagtype> &logger)
			: std::ostream(&_internalStream), _internalStream(logger._internalStream){};

		Logger(const LogStreamBuf<tagtype> *logstream)
			: std::ostream(&_internalStream), _internalStream(*logstream){};

		Logger(
			std::vector<std::shared_ptr<std::ostream>> iStreams = {},
			FormatterType iFormatter = nullptr,
			FilterType iFilter = nullptr,
			ArgListType iArglist = {})
			: std::ostream(&_internalStream),
			  _internalStream(iStreams, iFormatter, iFilter, iArglist){};

	public:
		LogStreamBuf<tagtype> &Stream() { return _internalStream; };

		Logger<tagtype> withArgs(const ArgListType arg) const;
		Logger<tagtype> withFormatter(const FormatterType formatter) const;
		Logger<tagtype> withFilter(const FilterType filter) const;

		Logger<tagtype> operator()() const;
		Logger<tagtype> operator()(const ArgListType arg) const;
		Logger<tagtype> operator()(const FormatterType formatter) const;
		Logger<tagtype> operator()(const FilterType filter) const;

		Logger<tagtype> operator()(const FormatterType formatter, const ArgListType arg) const;
		Logger<tagtype> operator()(const FilterType filter, const ArgListType arg) const;
		Logger<tagtype> operator()(const FormatterType formatter, const FilterType filter) const;
		Logger<tagtype> operator()(const FormatterType formatter, const FilterType filter, const ArgListType arg) const;
	};
}

// LogUtils
namespace HashColon::LogUtils
{
	/*
	* LogUtils provide [ tag/formatter/filter/helper functions ] for HashColon::Helper::Logger
	* Use CommonLogger for easy logging.

	* Tags(enum LogUtils::Tag):
		type	: string:	Defines message type, or gives information about the origin of the message.
							You can give any kind of string.
							For example, CommonLogger uses following type values.
							Log: "Log" / Error: "Error" / Debug: "Debug" / Message: "Message"
		file	: string:	Used only for error messages. Gives source file name where error occured.
							For example, { Tag::file, "LogUtils.hpp" }
							To use compiler macro, use { Tag::file, __FILE__ }
		line	: string:	Used only for error messages. Gives line number where error occured.
							For example, { Tag::file, "24" }
							To use compiler macro, use { Tag::file, __LINE__ }
		func	: string:	Used only for error messages. Gives function name where error occured.
							For example, { Tag::func, "ErrFormat()" }
							To use compiler macro, use { Tag::file, __FUNC__ }
		lvl		: int	:	Defines message priority. If the lvl is greater than maxlvl the message is suppressed.
							*** Recommendation for lvl values ***
							- 1 : starting/finishing messages for important event.
									such as reading large input data files, core computational steps, printing output files etc.
							- 3 : primary steps in important method/algorithms.
									for example, each steps in Bezier curve division can be logged.
									- step 1: build front-part division matrix
									- step 2: build end-part division matrix
									- step 3: multiply coefficients with each matrix
									- step 4: return results.
							- 5 : miscellaneous logs
		maxlvl	: int	:	Defines message priority criteria. If the lvl is greater than maxlvl the message is suppressed.

	* Special tag:
		Special tag is defined as macros for easy use.
		__CODEINFO_TAGS__ : combines file, line, func as one. no need to give values(compiler automatically provides their values)
							ex) { __CODEINFO_TAGS__ }

	* Formatters:
		LogFormat  : Basic log format.
					[timestamp][type] message
					ex)
						[2021-07-23 14:23:57.078][Log] blah....

		ErrFormat  : Format for error logging
					[timestamp][type]:@file#line
					\t	{func()}: message
					ex)
						[2021-07-23 14:23:57.078][Error]: @LogUtils.hpp#38
							{HashColon::LogUtils::ErrFormat()}: blah....

		NullFormat : Prints nothing.
		BasicFormat: No formatted message.

	* Filters:
		PassFilter	 : always true
		BlockFilter	 : always false
		VerboseFilter: if Tag::lvl and Tag::maxlvl is defined and lvl > maxlvl false. else true.
					   if Tag::lvl or Tag::maxlvl is not defined, true.

	* Helper functions:
		Frag::TimeStamp	 :	Provides current timestamp in forma [YYYY-MM-DD HH:mm:ss.xxx]
							usage:
								logger.Log() << Frag::TimeStamp() << messages << endl;
		Frag::Percentage :	Simple percentage format. [100.0%], [45.6%]
							usage:
								logger.Log() << Frag::Percentage(12, 100) << endl;  => [ 12.0%]
								logger.Log() << Frag::Percentage(0.6582) << endl;	=> [ 65.8%]
		Frag::Flashl	 :	Messages through Flashl is erased after the next message.
							Recommended to show current working progress.
							such as things like
								Working...[ xx.x%][***********...............................]
							By using Flashl, the printed line can be modified instead of stacking bunch of messages in the terminal.
							This function is not tested with the file output.
							So please use this with Commonlogger::Message
							*** IMPORTANT *** : DO NOT USE endl with Flashl. It will not work.
							*** IMPORTANT *** : It is recommended to flush after all the progress is finished. For good.
							usage1:
								logger.Message << Flashl(msg);
							usage2:
								atomic<size_t> progressCnt{0};
								#pragma omp parallel for
								for( ... ){
									// some kind of works...
									stringstream tempss;
									tempss << "working...: " << Frag::Percentage(++progressCnt, N);
									logger.Message << Flashl(tempss.str());
								}
								logger.Message << flush;
								logger.Log() << "Progress finished." >> endl;
	*/

	using ArgValue = std::variant<char, int, std::string, double>;

	/*static const shared_ptr<ostream> Stdout;
	static const shared_ptr<ostream> Stderr;*/
	const std::shared_ptr<std::ostream> Stdout = std::shared_ptr<std::ostream>(&std::cout, [](std::ostream *) {});
	const std::shared_ptr<std::ostream> Stderr = std::shared_ptr<std::ostream>(&std::cerr, [](std::ostream *) {});

	enum class Tag
	{
		type,
		file,
		line,
		func,
		lvl,
		maxlvl
	};

	std::string LogFormat(std::string msg, std::unordered_map<Tag, ArgValue> args);
	std::string ErrFormat(std::string msg, std::unordered_map<Tag, ArgValue> args);
	std::string NullFormat(std::string msg, std::unordered_map<Tag, ArgValue> args);
	std::string BasicFormat(std::string msg, std::unordered_map<Tag, ArgValue> args);

	bool PassFilter(std::unordered_map<Tag, ArgValue> args);
	bool BlockFilter(std::unordered_map<Tag, ArgValue> args);
	bool VerboseFilter(std::unordered_map<Tag, ArgValue> args);

	class Flashl
	{
	private:
		std::string _msg;
		struct Ignitor
		{
		};
		struct Terminator
		{
		};

	public:
		Flashl(std::string msg) : _msg(msg){};
		friend std::ostream &operator<<(std::ostream &lhs, Flashl rhs);
	};
	std::ostream &operator<<(std::ostream &lhs, Flashl rhs);

	class Percentage
	{
	private:
		double _percentage;

	public:
		Percentage(int item_done, int item_total);
		Percentage(double percentage);
		friend std::ostream &operator<<(std::ostream &lhs, Percentage rhs);
	};
	std::ostream &operator<<(std::ostream &lhs, Percentage rhs);

	class TimeStamp
	{
	public:
		TimeStamp(){};
		friend std::ostream &operator<<(std::ostream &lhs, TimeStamp rhs);
	};
	std::ostream &operator<<(std::ostream &lhs, TimeStamp rhs);
}

// Macros for logging
namespace HashColon
{
#ifndef __CODEINFO_TAGS__
#define __CODEINFO_TAGS__                           \
	{HashColon::LogUtils::Tag::file, __FILE__},     \
		{HashColon::LogUtils::Tag::line, __LINE__}, \
	{                                               \
		HashColon::LogUtils::Tag::func, __FUNC__    \
	}
#endif // !__CODEINFO_TAGS__
}

// CommonLogger
namespace HashColon
{
	/*
	* Simple example:
		CommonLogger logger;
		logger.Log( { { tag::lvl, 1 }, ... } ) << "some kind of message" << std::endl;
		logger.Error( { __CODEINFO_TAGS__ } << "some kind of error message " << std::endl;
		logger.Debug( { { tag::lvl, 1 }, __CODEINFO_TAGS__ } << "some kind of debug message " << std::endl;
		logger.Message() << "blah" << std::endl;

	* CommonLogger uses tags/filter/formatter in HashColon::LogUtils
	  For more information, refer HashColon::LogUtils

	* Loggers:
		Log: For basic logging. Use Util::LogFormat for logging.
			{ tag::type, "Log" } is pre-defined.
			stdout, log file available
		Error: For logging Errors. Use Util::ErrFormat for logging.
			{ tag::type, "Error" } is pre-defined.
			File/Screen logging available.
			stderr, log file + error log file available.
		Debug: For debugging messages. Use Util::ErrFormat for logging.
			{ tag::type, "Debug" } is pre-defined.
			stderr, log file + error log file available.
		Message: For screen-only & no formatted messages. Use Util::BasicFormat for logging.
			{ tag::type, "Message" } is pre-defined.
			only stdout is available.

	* Parameters:
		ErrorFile			:	shared_ptr<ostream>	:	Pointer to error logging file stream
		LogFile				:	shared_ptr<ostream>	:	Pointer to basic logging file stream
		enableLog.Screen	:	bool	:	If true, messages through Log will be screen printed to stdout.
		enableLog.File		:	bool	:	If true, messages through Log will be file printed to LogFile.
		enableError.Screen	:	bool	:	If true, messages through Error will be screen printed to stderr.
		enableError.File	:	bool	:	If true, messages through Error will be file printed to LogFile and ErrorFile.
		enableDebug.Screen	:	bool	:	If true, messages through Debug will be screen printed to stderr.
		enableDebug.File	:	bool	:	If true, messages through Debug will be file printed to LogFile and ErrorFile.
		enableMessage		:	bool	:	If true, messages through Message will be screen printed to stdout.
		verboseLevel		:	int		:	Messages with argument { tag: lvl, level } and 'level' less than given 'verboseLevel' will be suppressed.

	*/
	class CommonLogger final
	{
	public:
		struct _Params
		{
			std::vector<std::shared_ptr<std::ostream>> logStreams;
			std::vector<std::shared_ptr<std::ostream>> errorStreams;
			std::vector<std::shared_ptr<std::ostream>> debugStreams;
			std::vector<std::shared_ptr<std::ostream>> messageStreams;

			int verbose_level;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(CommonLogger);

	private:
		inline static _Params _cDefault;

	public:
		static _Params &GetDefaultParams() { return _cDefault; };
		static void Initialize(const std::string configFilePath = "");
		CommonLogger(_Params params = _cDefault);

	public:
		Logger<LogUtils::Tag> Log;
		Logger<LogUtils::Tag> Error;
		Logger<LogUtils::Tag> Debug;
		Logger<LogUtils::Tag> Message;

		// mutex-lock support for multi-threading
		inline static std::mutex _mutex;
	};
}

// ResultPrinter
namespace HashColon
{
	class ResultPrinter final : public Logger<LogUtils::Tag>
	{
	public:
		struct _Params
		{
			bool Screen;
			bool File;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(ResultPrinter);

	private:
		inline static _Params _cDefault;
		_Params _c;

	public:
		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };
		static void Initialize();
		ResultPrinter(std::string filepath, _Params params = _cDefault);
		ResultPrinter(const ResultPrinter &rhs);
	};
}

#endif

#include <HashColon/impl/Log_Impl.hpp>
