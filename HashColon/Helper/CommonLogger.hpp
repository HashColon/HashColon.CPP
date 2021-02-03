#ifndef HASHCOLON_HELPER_COMMONLOG_HPP
#define HASHCOLON_HELPER_COMMONLOG_HPP

#include "Log.hpp"
#include "LogUtils.hpp"
#include <HashColon/Helper/Log.hpp>
#include <HashColon/Helper/LogUtils.hpp>
#include <HashColon/Helper/Exception.hpp>

namespace HashColon::Helper
{
	/*
	* Simple example:
		CommonLogger logger;
		logger.Log( { { tag::lvl, 1 }, ... } ) << "some kind of message" << std::endl;
		logger.Error( { __CODEINFO_TAGS__ } << "some kind of error message " << std::endl;
		logger.Debug( { { tag::lvl, 1 }, __CODEINFO_TAGS__ } << "some kind of debug message " << std::endl;
		logger.Message() << "blah" << std::endl;

	* CommonLogger uses tags/filter/formatter in HashColon::Helper::LogUtils
	  For more information, refer HashColon::Helper::LogUtils

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
			std::shared_ptr<std::ostream> ErrorFile;
			std::shared_ptr<std::ostream> LogFile;

			struct { bool Screen; bool File; } enableLog;
			struct { bool Screen; bool File; } enableError;
			struct { bool Screen; bool File; } enableDebug;
			bool enableMessage;
			int verbose_level = -1;
		};

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(CommonLogger);

	private:
		static _Params _cDefault;
		_Params _c;
	public:
		static _Params& GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };
		static void Initialize();
		CommonLogger(_Params params = _cDefault);

	public:
		Logger<LogUtils::Tag> Log;
		Logger<LogUtils::Tag> Error;
		Logger<LogUtils::Tag> Debug;
		Logger<LogUtils::Tag> Message;
	};
}

namespace HashColon::Helper
{
	class ResultPrinter final : public Logger<LogUtils::Tag>
	{
	public:
		struct _Params { bool Screen; bool File; };

		HASHCOLON_CLASS_EXCEPTION_DEFINITION(ResultPrinter);

	private:
		static _Params _cDefault;
		_Params _c;

	public:
		static _Params GetDefaultParams() { return _cDefault; };
		_Params GetParams() { return _c; };
		static void Initialize();
		ResultPrinter(std::string filepath, _Params params = _cDefault);
		ResultPrinter(const ResultPrinter& rhs);

	};
}

#endif