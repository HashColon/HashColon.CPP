#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include <HashColon/Helper/Exception.hpp>
#include <HashColon/Helper/CLI11_extended.hpp>
#include <HashColon/Helper/CommonLogger.hpp>
#include <HashColon/Helper/SingletonCLI.hpp>

namespace hidden
{
	using namespace std;

	string getFilename(const string dir, const string prefix)
	{
		// get current time 
		auto now = chrono::system_clock::now();
		auto now_time_t = chrono::system_clock::to_time_t(now);

		stringstream ss;
		string filepath;
		ss << dir << '/' << prefix << put_time(localtime(&now_time_t), "%Y%m%d%H%M%S") << ".log";

		return ss.str();
	}
}

HASHCOLON::Helper::CommonLogger::_Params HASHCOLON::Helper::CommonLogger::_cDefault;
//HASHCOLON::Helper::Logger<HASHCOLON::Helper::LogUtils::Tag>::operator()<

namespace HASHCOLON::Helper
{
	using namespace std;
	namespace Util = LogUtils;
	using SingletonCLI = HASHCOLON::Helper::SingletonCLI;
			
	void CommonLogger::Initialize()
	{		
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Log");

		{
			using namespace CLI::BooleanOption;
			add_option(cli, "--enableLogScreen", _cDefault.enableLog.Screen, "Enable logging to screen");
			add_option(cli, "--enableLogFile", _cDefault.enableLog.File, "Enable logging to file");
			add_option(cli, "--enableErrorScreen", _cDefault.enableError.Screen, "Enable error logging to screen");
			add_option(cli, "--enableErrorFile", _cDefault.enableError.File, "Enable error logging to file");
			add_option(cli, "--enableDebugScreen", _cDefault.enableDebug.Screen, "Enable debug logging to screen");
			add_option(cli, "--enableDebugFile", _cDefault.enableDebug.File, "Enable debug logging to file");
			add_option(cli, "--enableMessageScreen", _cDefault.enableMessage, "Enable Message logging to screen");		
		}
		

		cli->add_option(
			"--errorlogDir", 
			[](vector<string> res)
			{				
				string ErrDir;
				bool re = CLI::detail::lexical_cast(res[0], ErrDir);				
				if(re)
				{	
					shared_ptr<ofstream> ofFile
						= make_shared<ofstream>(hidden::getFilename(ErrDir, "errorlog_"));
					
					if (ofFile->is_open())
					{					
						_cDefault.ErrorFile = ofFile;
						return true;
					}	
					else
					{
						_cDefault.ErrorFile = nullptr;
						return false;
					}
				}				
				else return false;
			},
			"Directory for errorlog files. Error/Debug messages are written as [dir]/errorlog_yymmddHHMMSS.log"
		)->check(CLI::ExistingDirectory);

		string logDir;
		cli->add_option(
			"--logDir",
			[](vector<string> res)
			{
				string logDir;
				bool re = CLI::detail::lexical_cast(res[0], logDir);				
				if(re)
				{			
					shared_ptr<ofstream> ofFile
						= make_shared<ofstream>(hidden::getFilename(logDir, "log_"));					
					if (ofFile->is_open())
					{
						_cDefault.LogFile = ofFile;
						return true;
					}						
					else
					{
						_cDefault.LogFile = nullptr;
						return false;
					}
				}
				else return false;
			},
			"Directory for log files. Log/Error/Debug messages are written as [dir]/log_yymmddHHMMSS.log"
		)->check(CLI::ExistingDirectory);

		cli->add_option(
			"--verboseLvl",
			_cDefault.verbose_level,
			"Enable verbose level. Logs with level exceeding verbose level will not be logged."
		);

		
	}

	CommonLogger::CommonLogger(_Params params)
		: _c(params) ,
		Log({}, Util::LogFormat, Util::VerboseFilter, { {Util::Tag::type, " Log "} }),
		Error({}, Util::ErrFormat, Util::VerboseFilter, {{Util::Tag::type, "Error"} }),
		Debug({}, Util::ErrFormat, Util::VerboseFilter, {{Util::Tag::type, "Debug"}}),
		Message({}, Util::BasicFormat, Util::PassFilter, { {Util::Tag::type, "Message"} })
	{
		// check file streams 
		// if there is no logfile and  at least one of the file option is on,
		// throw an exception
		if (!_c.LogFile && (_c.enableLog.File || _c.enableError.File || _c.enableDebug.File))
			throw CommonLogger::Exception("Log file missing while constructing CommonLogger.",
				__CODEINFO__);
		// if there is no errorlogfile and at least one of the file option is on,
		// throw an exception
		if (!_c.ErrorFile && (_c.enableError.File || _c.enableDebug.File))
			throw CommonLogger::Exception("Error log file missing while constructing CommonLogger.",
				__CODEINFO__);
		
		// assert _c.LogFile and _c.ErrorFile exists
		assert(_c.LogFile && _c.ErrorFile && LogUtils::Stdout && LogUtils::Stderr);

		// set streams and options for each logger
		// log
		if (_c.enableLog.Screen)
			Log.Stream().StreamList().push_back(LogUtils::Stdout);
		if (_c.enableLog.File)
			Log.Stream().StreamList().push_back(_c.LogFile);
		// error
		if (_c.enableError.Screen)
			Error.Stream().StreamList().push_back(LogUtils::Stderr);
		if (_c.enableError.File)
		{
			Error.Stream().StreamList().push_back(_c.LogFile);
			Error.Stream().StreamList().push_back(_c.ErrorFile);
		}
		// debug
		if (_c.enableDebug.Screen)
			Debug.Stream().StreamList().push_back(LogUtils::Stderr);
		if (_c.enableDebug.File)
		{
			Debug.Stream().StreamList().push_back(_c.LogFile);
			Debug.Stream().StreamList().push_back(_c.ErrorFile);
		}
		// Message
		if (_c.enableMessage)
			Message.Stream().StreamList().push_back(LogUtils::Stdout);

		// set verbose_lvl
		if (_c.verbose_level >= 0)
		{
			Log.Stream().Arguments().insert({ LogUtils::Tag::maxlvl, _c.verbose_level });
			Error.Stream().Arguments().insert({ LogUtils::Tag::maxlvl, _c.verbose_level });
			Debug.Stream().Arguments().insert({ LogUtils::Tag::maxlvl, _c.verbose_level });
			Message.Stream().Arguments().insert({ LogUtils::Tag::maxlvl, _c.verbose_level });
		}
	}
}

HASHCOLON::Helper::ResultPrinter::_Params HASHCOLON::Helper::ResultPrinter::_cDefault;
namespace HASHCOLON::Helper
{
	using namespace std;
	using RP = ResultPrinter;

	void RP::Initialize()
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Log");

		{
			using namespace CLI::BooleanOption;
			add_option(cli, "--enableResultScreen", _cDefault.Screen, "Enable logging to screen");
			add_option(cli, "--enableResultFile", _cDefault.File, "Enable logging to file");			
		}
	}

	RP::ResultPrinter(string filepath, _Params params)
		: _c(params), 
		Logger({}, LogUtils::BasicFormat, LogUtils::PassFilter, {{LogUtils::Tag::type, "Result"}})
	{
		if (_c.File)
		{
			shared_ptr<ofstream> filestream = make_shared<ofstream>(filepath);
			if (!filestream->is_open())
				throw ResultPrinter::Exception("Result file " + filepath + "is invalid.", __CODEINFO__);
			else
				this->Stream().StreamList().push_back(filestream);
		}
		
		if (_c.Screen)
		{
			assert(LogUtils::Stdout);
			this->Stream().StreamList().push_back(LogUtils::Stdout);
		}
	}

	RP::ResultPrinter(const ResultPrinter& rhs)
		: _c(rhs._c), Logger(rhs) {}; 
}
