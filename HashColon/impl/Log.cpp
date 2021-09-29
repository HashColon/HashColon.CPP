// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
// modified external libraries
#include <CLI11_modified/CLI11.hpp>
#include <CLI11_modified/CLI11_extended.hpp>
// HashColon libraries
#include <HashColon/Exception.hpp>
#include <HashColon/Helper.hpp>
#include <HashColon/SingletonCLI.hpp>
// header file for this source file
#include <HashColon/Log.hpp>

using namespace std;
using namespace std::chrono;
using namespace HashColon;
using namespace HashColon::Fs;

// LogUtils
namespace HashColon::LogUtils
{
	string LogFormat(string msg, unordered_map<Tag, ArgValue> args)
	{		
		// get current time;
		auto now = system_clock::now();
		//auto now_time_t = system_clock::to_time_t(now);
		auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

		stringstream ss;
		
		// print timestamp and tag
		ss << TimeStamp()
			<< "[" << get<string>(args.at(Tag::type)) << "]"
			<< ": " << msg;

		// return formatted string
		return ss.str();
	}

	string ErrFormat(string msg, unordered_map<Tag, ArgValue> args)
	{		
		// get current time;
		auto now = system_clock::now();
		//auto now_time_t = system_clock::to_time_t(now);
		auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

		stringstream ss;
		// print timestamp and tag
		ss << TimeStamp()
			<< "[" << get<string>(args.at(Tag::type)) << "]"
			// print filename and line
			<< ": @" << get<string>(args.at(Tag::file)) << "#" << get<int>(args.at(Tag::line)) << std::endl
			<< "\t{" << get<string>(args.at(Tag::func)) << "()}"
			<< ": " << msg;

		// return formatted string
		return ss.str();
	}

	string NullFormat(string msg, unordered_map<Tag, ArgValue> args)
	{
		return "";
	}

	string BasicFormat(string msg, unordered_map<Tag, ArgValue> args)
	{
		return msg;
	}

	bool PassFilter(unordered_map<Tag, ArgValue> args) { return true; }
	bool BlockFilter(unordered_map<Tag, ArgValue> args) { return false; }

	bool VerboseFilter(unordered_map<Tag, ArgValue> args)
	{		
		if (args.count(Tag::lvl) == 1 && args.count(Tag::maxlvl) == 1)
		{
			int lvl_int = get<int>(args.at(Tag::lvl));
			int verbose_lvl_int = get<int>(args.at(Tag::maxlvl));

			if (lvl_int > verbose_lvl_int)
				return false;
		}
		return true;
	}

	ostream& operator<< (ostream& lhs, Flashl rhs)
	{
		lhs << rhs._msg << flush;
		lhs << '\r' << string(rhs._msg.length(), ' ') << '\r';		
		return lhs;
	}

	Percentage::Percentage(int item_done, int item_total)
	{
		_percentage = 
			max(0.0, 
				min(100.0, 
					(100.0 * (double)item_done / (double)item_total)
				)
			);
	}

	Percentage::Percentage(double percentage)
	{
		_percentage =
			max(0.0, 
				min(100.0, percentage)
			);
	}

	ostream& operator<< (ostream& lhs, Percentage rhs)
	{
		lhs << '[' << setw(5) << fixed << setprecision(1) << rhs._percentage << "\%]";
		return lhs;
	}	

	ostream& operator<< (ostream& lhs, TimeStamp rhs)
	{		
		// get current time;
		auto now = system_clock::now();
		auto now_time_t = system_clock::to_time_t(now);
		auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
		// print timestamp and tag
		lhs << "[" << put_time(localtime(&now_time_t), "%F %T.") << setfill('0') << setw(3) << ms.count() << "]";

		return lhs;
	}
}

// CommonLogger
namespace HashColon
{	
	namespace Util = LogUtils;

	namespace _local
	{
		string getFilename(const string dir, const string prefix)
		{
			// get current time 
			auto now = system_clock::now();
			auto now_time_t = system_clock::to_time_t(now);

			stringstream ss;
			string filepath;
			ss << dir << '/' << prefix << put_time(localtime(&now_time_t), "%Y%m%d%H%M%S") << ".log";

			return ss.str();
		}
	}

	void CommonLogger::Initialize(const string configFilePath)
	{		
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Log");

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}

		cli->add_option("--enableLogScreen", _cDefault.enableLog.Screen, "Enable logging to screen");
		cli->add_option("--enableLogFile", _cDefault.enableLog.File, "Enable logging to file");
		cli->add_option("--enableErrorScreen", _cDefault.enableError.Screen, "Enable error logging to screen");
		cli->add_option("--enableErrorFile", _cDefault.enableError.File, "Enable error logging to file");
		cli->add_option("--enableDebugScreen", _cDefault.enableDebug.Screen, "Enable debug logging to screen");
		cli->add_option("--enableDebugFile", _cDefault.enableDebug.File, "Enable debug logging to file");
		cli->add_option("--enableMessageScreen", _cDefault.enableMessage, "Enable Message logging to screen");

		cli->add_option(
			"--errorLogDir",
			[](vector<string> res)
			{
				string ErrDir;
				bool re = CLI::detail::lexical_cast(res[0], ErrDir);
				if (re)
				{
					// if directory is given as '-': no error file is created
					if (ErrDir == "-")
					{
						_cDefault.ErrorFile = nullptr;
						return true;
					}

					if (!BuildDirectoryStructure(ErrDir))
						throw CommonLogger::Exception("Cannot find/create errorLogDir", __CODEINFO__);


					shared_ptr<ofstream> ofFile
						= make_shared<ofstream>(_local::getFilename(ErrDir, "errorlog_"));

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
			"Directory for errorlog files. Use \"-\" to prevent errorlog file creation. Error/Debug messages are written as [dir]/errorlog_yymmddHHMMSS.log"
		);//->check(CLI::ExistingDirectory);

		string logDir;
		cli->add_option(
			"--logDir",
			[](vector<string> res)
			{
				string logDir;
				bool re = CLI::detail::lexical_cast(res[0], logDir);
				if (re)
				{
					// if directory is given as '-': no error file is created
					if (logDir == "-")
					{
						_cDefault.LogFile = nullptr;
						return true;
					}

					if (!BuildDirectoryStructure(logDir))
						throw CommonLogger::Exception("Cannot find/create errorLogDir", __CODEINFO__);


					shared_ptr<ofstream> ofFile
						= make_shared<ofstream>(_local::getFilename(logDir, "log_"));
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
			"Directory for log files. Use \"-\" to prevent log file creation. Log/Error/Debug messages are written as [dir]/log_yymmddHHMMSS.log"
		);//->check(CLI::ExistingDirectory);

		cli->add_option(
			"--verboseLvl",
			_cDefault.verbose_level,
			"Enable verbose level. Logs with level exceeding verbose level will not be logged."
		);


	}

	CommonLogger::CommonLogger(_Params params)
		: _c(params),
		Log({}, Util::LogFormat, Util::VerboseFilter, { {Util::Tag::type, " Log "} }),
		Error({}, Util::ErrFormat, Util::VerboseFilter, { {Util::Tag::type, "Error"} }),
		Debug({}, Util::ErrFormat, Util::VerboseFilter, { {Util::Tag::type, "Debug"} }),
		Message({}, Util::BasicFormat, Util::PassFilter, { {Util::Tag::type, "Message"} })
	{
		// assert _c.LogFile and _c.ErrorFile exists
		assert(LogUtils::Stdout && LogUtils::Stderr);
		//assert(_c.LogFile && _c.ErrorFile && LogUtils::Stdout && LogUtils::Stderr);

		// set streams and options for each logger
		// log
		if (_c.enableLog.Screen)
			Log.Stream().StreamList().push_back(LogUtils::Stdout);
		if (_c.enableLog.File)
		{
			if (!_c.LogFile)
				throw CommonLogger::Exception("Log file missing while constructing CommonLogger. Check option logDir.",
					__CODEINFO__);
			Log.Stream().StreamList().push_back(_c.LogFile);
		}
		// error
		if (_c.enableError.Screen)
			Error.Stream().StreamList().push_back(LogUtils::Stderr);
		if (_c.enableError.File)
		{
			if (!_c.LogFile)
				throw CommonLogger::Exception("Log file missing while constructing CommonLogger. Check option logDir.",
					__CODEINFO__);
			if (!_c.ErrorFile)
				throw CommonLogger::Exception("Error log file missing while constructing CommonLogger. Check option errorLogDir.",
					__CODEINFO__);
			Error.Stream().StreamList().push_back(_c.ErrorFile);
		}
		// debug
		if (_c.enableDebug.Screen)
		{
			Debug.Stream().StreamList().push_back(LogUtils::Stdout);
			//Debug.Stream().StreamList().push_back(LogUtils::Stderr);
		}
		if (_c.enableDebug.File)
		{
			if (!_c.LogFile)
				throw CommonLogger::Exception("Log file missing while constructing CommonLogger. Check option logDir.",
					__CODEINFO__);
			if (!_c.ErrorFile)
				throw CommonLogger::Exception("Error log file missing while constructing CommonLogger. Check option errorLogDir.",
					__CODEINFO__);
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

// ResultPrinter
namespace HashColon
{	
	void ResultPrinter::Initialize()
	{
		CLI::App* cli = SingletonCLI::GetInstance().GetCLI("Log");

		cli->add_option("--enableResultScreen", _cDefault.Screen, "Enable logging to screen");
		cli->add_option("--enableResultFile", _cDefault.File, "Enable logging to file");

	}

	ResultPrinter::ResultPrinter(string filepath, _Params params)
		: Logger({}, LogUtils::BasicFormat, LogUtils::PassFilter, { {LogUtils::Tag::type, "Result"} }),
		_c(params)
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

	ResultPrinter::ResultPrinter(const ResultPrinter& rhs)
		: Logger(rhs), _c(rhs._c) {};
}

