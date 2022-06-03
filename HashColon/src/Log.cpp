// header file for this source file
#include <HashColon/Log.hpp>

// std libraries
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
// #include <map>
#include <variant>
#include <vector>
// check filesystem support
#if defined __has_include
#if __has_include(<filesystem>)
#include <filesystem>
#define HASHCOLON_USING_FILESYSTEM using namespace std::filesystem
// #elif __has_include(<experimental/filesystem>)
// #include <experimental/filesystem>
// #define HASHCOLON_USING_FILESYSTEM using namespace std::experimental::filesystem
#elif __has_include(<boost/filesystem.hpp>)
#define HASHCOLON_USING_FILESYSTEM using namespace boost::filesystem
#include <boost/filesystem.hpp>
#else
#error C++17 support or boost library required.
#endif
#else
#error C++ compiler with __has_include support required.
#endif

// HashColon libraries
#include <HashColon/CLI11.hpp>
#include <HashColon/CLI11_JsonSupport.hpp>
#include <HashColon/Exception.hpp>
#include <HashColon/Helper.hpp>
#include <HashColon/SingletonCLI.hpp>

using namespace std;
using namespace std::chrono;
using namespace HashColon;
using namespace HashColon::Fs;

// LogUtils
namespace HashColon::LogUtils
{
	string LogFormat(string msg, ArgListType args)
	{
		// get current time;
		auto now = system_clock::now();
		// auto now_time_t = system_clock::to_time_t(now);
		auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

		stringstream ss;

		// print timestamp and tag
		ss << TimeStamp()
		   << "[" << get<string>(args.at(Tag::type)) << "]"
		   << ": " << msg;

		// return formatted string
		return ss.str();
	}

	string ErrFormat(string msg, ArgListType args)
	{
		// get current time;
		auto now = system_clock::now();

		// auto now_time_t = system_clock::to_time_t(now);
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

	string NullFormat(string msg, ArgListType args)
	{
		return "";
	}

	string BasicFormat(string msg, ArgListType args)
	{
		return msg;
	}

	bool PassFilter(ArgListType args) { return true; }
	bool BlockFilter(ArgListType args) { return false; }

	bool VerboseFilter(ArgListType args)
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

	ostream &operator<<(ostream &lhs, Flashl rhs)
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
					(100.0 * (double)item_done / (double)item_total)));
	}

	Percentage::Percentage(double percentage)
	{
		_percentage =
			max(0.0,
				min(100.0, percentage));
	}

	ostream &operator<<(ostream &lhs, Percentage rhs)
	{
		lhs << '[' << setw(5) << fixed << setprecision(1) << rhs._percentage << "\%]";
		return lhs;
	}

	ostream &operator<<(ostream &lhs, TimeStamp rhs)
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
		/* getFilesname: auto-gen filename for logging
		 */
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

		/* SetStreams: helper function to set streams
		// vals: input from config
		// targetStreamList: result stream list as output
		// prefix: prefix for auto-generated log files
		*/
		void SetStreams(const vector<string> &vals,
						vector<shared_ptr<ostream>> &targetStreamList,
						const string prefix)
		{
			using namespace HashColon::String;
			HASHCOLON_USING_FILESYSTEM;

			// for each item in input string lists
			for (const string &val : vals)
			{
				// trim & make lower
				string trimVal = TrimCopy(val);
				string lowerVal = ToLowerCopy(trimVal);

				// if val is "-" means end of list
				if (lowerVal == "-")
					break;
				// if val is stdout
				else if (lowerVal == "stdout")
					targetStreamList.push_back(LogUtils::Stdout);
				// if val is stderr
				else if (lowerVal == "stderr")
				{
					cout << "!!!!" << endl;
					targetStreamList.push_back(LogUtils::Stderr);
				}
				else
				{
					path pathval = trimVal.c_str();
					// if val is a file
					if (is_regular_file(pathval))
					{
						shared_ptr<std::ofstream> ofFile = make_shared<std::ofstream>(trimVal);
						targetStreamList.push_back(ofFile);
					}
					// if val is a directory
					else if (is_directory(pathval))
					{
						shared_ptr<std::ofstream> ofFile = make_shared<std::ofstream>(_local::getFilename(trimVal, prefix));
						targetStreamList.push_back(ofFile);
					}
					// if val is not a file nor directory
					else
					{
						throw CommonLogger::Exception("Invalid stream input for " + prefix + ": should be one of file/directory/stdin/stdout.");
					}
				}
			}
		}
	}

	CommonLogger GlobalLogger;

	void CommonLogger::Initialize(
		const string configFilePath,
		const string configNamespace)
	{
		CLI::App *cli = SingletonCLI::GetInstance().GetCLI(configNamespace);

		if (!configFilePath.empty())
		{
			SingletonCLI::GetInstance().AddConfigFile(configFilePath);
		}
		// assert stdout, stderr are defined in LogUtils
		assert(LogUtils::Stdout && LogUtils::Stderr);

		// add logStreams options using SetStreams
		cli->add_option_function<vector<string>>(
			"--logStreams",
			[](const vector<string> &vals)
			{ _local::SetStreams(vals, _cDefault.logStreams, "log"); },
			"Streams for default logging. List of any of the File/Directory/Stdout/Stderr.");

		// add errorStreams options using SetStreams
		cli->add_option_function<vector<string>>(
			"--errorStreams",
			[](const vector<string> &vals)
			{ _local::SetStreams(vals, _cDefault.errorStreams, "error"); },
			"Streams for error logging. List of any of the File/Directory/Stdout/Stderr.");

		// add debugStreams options using SetStreams
		cli->add_option_function<vector<string>>(
			"--debugStreams",
			[](const vector<string> &vals)
			{ _local::SetStreams(vals, _cDefault.debugStreams, "debug"); },
			"Streams for debug message logging. List of any of the File/Directory/Stdout/Stderr.");

		// add messageStreams options using SetStreams
		cli->add_option_function<vector<string>>(
			"--messageStreams",
			[](const vector<string> &vals)
			{ _local::SetStreams(vals, _cDefault.messageStreams, "message"); },
			"Streams for messages. List of any of the File/Directory/Stdout/Stderr.");

		// set verbose level
		cli->add_option(
			"--verboseLvl",
			_cDefault.verbose_level,
			"Enable verbose level. Logs with level exceeding verbose level will not be logged.");

		cli->callback(
			[&]()
			{
				GlobalLogger.Reset();
			});

		cli->configurable();
	}

	CommonLogger::CommonLogger(_Params params)
		: Log({}, Util::LogFormat, Util::VerboseFilter, {{Util::Tag::type, " Log "}}),
		  Error({}, Util::ErrFormat, Util::VerboseFilter, {{Util::Tag::type, "Error"}}),
		  Debug({}, Util::ErrFormat, Util::VerboseFilter, {{Util::Tag::type, "Debug"}}),
		  Message({}, Util::BasicFormat, Util::PassFilter, {{Util::Tag::type, "Message"}})
	{
		// set streams for each logger
		Log.Stream().StreamList() = params.logStreams;
		Error.Stream().StreamList() = params.errorStreams;
		Debug.Stream().StreamList() = params.debugStreams;
		Message.Stream().StreamList() = params.messageStreams;

		// set verbose_lvl
		if (params.verbose_level >= 0)
		{
			Log.Stream().Arguments().insert({LogUtils::Tag::maxlvl, params.verbose_level});
			Error.Stream().Arguments().insert({LogUtils::Tag::maxlvl, params.verbose_level});
			Debug.Stream().Arguments().insert({LogUtils::Tag::maxlvl, params.verbose_level});
			Message.Stream().Arguments().insert({LogUtils::Tag::maxlvl, params.verbose_level});
		}
	}

	void CommonLogger::Reset()
	{
		// set streams for each logger
		Log.Stream().StreamList() = _cDefault.logStreams;
		Error.Stream().StreamList() = _cDefault.errorStreams;
		Debug.Stream().StreamList() = _cDefault.debugStreams;
		Message.Stream().StreamList() = _cDefault.messageStreams;

		// set verbose_lvl
		if (_cDefault.verbose_level >= 0)
		{
			Log.Stream().Arguments().insert_or_assign(LogUtils::Tag::maxlvl, _cDefault.verbose_level);
			Error.Stream().Arguments().insert_or_assign(LogUtils::Tag::maxlvl, _cDefault.verbose_level);
			Debug.Stream().Arguments().insert_or_assign(LogUtils::Tag::maxlvl, _cDefault.verbose_level);
			Message.Stream().Arguments().insert_or_assign(LogUtils::Tag::maxlvl, _cDefault.verbose_level);
		}
	}
}
