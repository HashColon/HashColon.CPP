#include <avikus/log.hpp>

#include <cassert>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <avikus/headers/filesystem>

#include <avikus/datetime.hpp>
#include <avikus/singleton-cli.hpp>
#include <avikus/stringhelper.hpp>

using std::endl;
using std::fixed;
using std::flush;
using std::get;
using std::make_shared;
using std::max;
using std::min;
using std::ostream;
using std::put_time;
using std::setfill;
using std::setprecision;
using std::setw;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

#ifdef AVIKUS_HEADER_FILESYSTEM_STD
using std::filesystem::is_directory;
using std::filesystem::path;
#else
using boost::filesystem::is_directory;
using boost::filesystem::path;
#endif

namespace Avikus
{
    // initialize GlobalLogger with empty streams
    // Use Initialize() to define parameters from CLI/config/ENV
    CommonLogger GlobalLogger({{}, {}, {}, {}, std::numeric_limits<int>::max()});
}
namespace Avikus::GlobalLogger_
{
    unordered_map<string, const shared_ptr<ostream>> Streams;
}

// LogUtils
namespace Avikus::LogUtils
{
    string LogFormat(string msg, ArgListType args)
    {
        stringstream ss;

        // print timestamp and tag
        ss << TimestampUtc()
           << "[" << get<string>(args.at(Tag::type)) << "]"
           << ": " << msg;

        // return formatted string
        return ss.str();
    }

    string ErrFormat(string msg, ArgListType args)
    {
        stringstream ss;
        // print timestamp and tag
        ss << TimestampUtc()
           << "[" << get<string>(args.at(Tag::type)) << "]: ";

        // print filename and line
        if (args.find(Tag::file) != args.end() && args.find(Tag::line) != args.end() && args.find(Tag::func) != args.end())
        {
            ss << "@" << get<string>(args.at(Tag::file)) << "#" << get<int>(args.at(Tag::line)) << "\n"
               << "\t{" << get<string>(args.at(Tag::func)) << "()}: \n";
        }

        ss << msg;
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

    ostream &operator<<(ostream &lhs, TimestampUtc rhs)
    {
        TimePoint now = TimePoint::Now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        lhs << "[" << now.ToUtcString("%F %T.") << setfill('0') << setw(3) << ms.count() << " UTC]";

        return lhs;
    }
} // namespace Avikus::LogUtils

namespace Avikus
{
    CommonLogger::CommonLogger(_Params params)
        : Log(params.logStreams,
              LogUtils::LogFormat, LogUtils::VerboseFilter,
              {{LogUtils::Tag::type, " Log "}, {LogUtils::Tag::maxlvl, params.verbose_level}}),
          Error(params.errorStreams,
                LogUtils::ErrFormat, LogUtils::VerboseFilter,
                {{LogUtils::Tag::type, "Error"}, {LogUtils::Tag::maxlvl, params.verbose_level}}),
          Debug(params.debugStreams,
                LogUtils::ErrFormat, LogUtils::VerboseFilter,
                {{LogUtils::Tag::type, "Debug"}, {LogUtils::Tag::maxlvl, params.verbose_level}}),
          Message(params.messageStreams,
                  LogUtils::BasicFormat, LogUtils::PassFilter,
                  {{LogUtils::Tag::type, "Message"}, {LogUtils::Tag::maxlvl, params.verbose_level}}){};

} // namespace Avikus

namespace Avikus::GlobalLogger_
{
    void Initialize(
        const string configFilePath,
        const string configNamespace)
    {
        InitializeStreams();

        // Get Singleton cli instance
        CLI::App *cli = SingletonCLI::GetInstance().GetCLI(configNamespace);

        if (!configFilePath.empty())
        {
            SingletonCLI::GetInstance().AddConfigFile(configFilePath);
        }

        // add logStreams options using SetStreams
        cli->add_option_function<vector<string>>(
               "--logStreams",
               [](const vector<string> &vals)
               { ConvertToStreams(vals, GlobalLogger.Log.stream.streamList, "log"); },
               "Streams for default logging. List of any of the File/Directory/stdout/stderr.")
            ->default_val(vector<string>{"-"})
            ->envname(GetEnvName(configNamespace, "logStreams"));

        // add errorStreams options using SetStreams
        cli->add_option_function<vector<string>>(
               "--errorStreams",
               [](const vector<string> &vals)
               { ConvertToStreams(vals, GlobalLogger.Error.stream.streamList, "error"); },
               "Streams for error logging. List of any of the File/Directory/stdout/stderr.")
            ->default_val(vector<string>{"-"})
            ->envname(GetEnvName(configNamespace, "errorStreams"));

        // add debugStreams options using SetStreams
        cli->add_option_function<vector<string>>(
               "--debugStreams",
               [](const vector<string> &vals)
               { ConvertToStreams(vals, GlobalLogger.Debug.stream.streamList, "debug"); },
               "Streams for debug message logging. List of any of the File/Directory/stdout/stderr.")
            ->default_val(vector<string>{"-"})
            ->envname(GetEnvName(configNamespace, "debugStreams"));

        // add messageStreams options using SetStreams
        cli->add_option_function<vector<string>>(
               "--messageStreams",
               [](const vector<string> &vals)
               { ConvertToStreams(vals, GlobalLogger.Log.stream.streamList, "message"); },
               "Streams for messages. List of any of the File/Directory/stdout/stderr.")
            ->default_val(vector<string>{"-"})
            ->run_callback_for_default()
            ->envname(GetEnvName(configNamespace, "messageStreams"));

        // set verbose level
        cli->add_option_function<int>(
               "--verboseLvl",
               [](const int &val)
               {
                   GlobalLogger.Log.stream.arguments[LogUtils::Tag::maxlvl] = val;
                   GlobalLogger.Error.stream.arguments[LogUtils::Tag::maxlvl] = val;
                   GlobalLogger.Debug.stream.arguments[LogUtils::Tag::maxlvl] = val;
                   GlobalLogger.Message.stream.arguments[LogUtils::Tag::maxlvl] = val;
               },
               "Enable verbose level. Logs with level exceeding verbose level will not be logged.")
            ->default_val(5)
            ->envname(GetEnvName(configNamespace, "verboseLvl"));
    }

    void InitializeStreams()
    {
        // Add stdout, stdin to GlobalStreams
        Streams.emplace("stdout", shared_ptr<ostream>(&std::cout, [](std::ostream *) {}));
        Streams.emplace("stderr", shared_ptr<ostream>(&std::cerr, [](std::ostream *) {}));
    }

    /// @brief Generate the name of the logfiles
    /// @param dir
    /// @param prefix
    /// @return
    string AutoGenerateLogFileName(const string dir, const string prefix)
    {
        // get current time
        auto now = system_clock::now();
        auto now_time_t = system_clock::to_time_t(now);

        stringstream ss;
        string filepath;
        ss << dir << '/' << prefix << put_time(localtime(&now_time_t), "%Y%m%d%H%M%S") << ".log";

        return ss.str();
    }

    void ConvertToStreams(const vector<string> &vals,
                          vector<shared_ptr<ostream>> &targetStreamList,
                          const string prefix)
    {
        // for each item in input string lists
        for (string val : vals)
        {
            // trim input string
            Trim(val);

            // if val is "-" means end of list
            if (val == "-")
            {
                break;
            }
            // if val is defined in GlobalStreams: such as stdout, stderr
            else if (Streams.find(val) != Streams.end())
            {
                targetStreamList.push_back(Streams.at(val));
            }
            else
            {
                path pathVal = val.c_str();
                // if val is a directory
                if (is_directory(pathVal))
                {
                    shared_ptr<std::ofstream> ofFile = make_shared<std::ofstream>(AutoGenerateLogFileName(val, prefix));
                    targetStreamList.push_back(ofFile);
                }
                // else check if the given file is a valid output file
                else
                {
                    // if val is a valid file for output
                    shared_ptr<std::ofstream> ofFile = make_shared<std::ofstream>(val);
                    if (ofFile->is_open())
                        targetStreamList.push_back(ofFile);
                    // if val is not a file nor directory
                    else
                        throw InvalidStreamNameException("Invalid stream input for " + prefix + ": should be one of file/directory/stdin/stdout/[predefined stream].");
                }
            }
        }
    }
} // namespace Avikus::GlobalLogger_