#pragma once
#ifndef HASHCOLON_HELPER_COMMONLOG_HPP
#define HASHCOLON_HELPER_COMMONLOG_HPP

#include "Log.hpp"
#include "LogUtils.hpp"
#include <HashColon/Helper/Log.hpp>
#include <HashColon/Helper/LogUtils.hpp>
#include <HashColon/Helper/Exception.hpp>

namespace HASHCOLON::Helper
{
	class CommonLogger final
	{
	public:
		struct _Params
		{
			std::shared_ptr<std::ostream> ErrorFile;
			std::shared_ptr<std::ostream> LogFile;
			// deprecated
			/*string errorDir;
			string logDir;*/

			struct
			{
				bool Screen;
				bool File;				
			} enableLog;

			struct
			{
				bool Screen;
				bool File;
			} enableError;

			struct
			{
				bool Screen;
				bool File;
			} enableDebug;

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

namespace HASHCOLON::Helper
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

	private :
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