#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <HashColon/Core/LogUtils.hpp>

//const std::shared_ptr<std::ostream > Stdout = std::shared_ptr<std::ostream>(&std::cout, [](std::ostream*) {});
//const std::shared_ptr<std::ostream> Stderr = std::shared_ptr<std::ostream>(&std::cerr, [](std::ostream*) {});

namespace HashColon::LogUtils
{
	using namespace std;

	string LogFormat(string msg, unordered_map<Tag, ArgValue> args)
	{
		using namespace std::chrono;
		// get current time;
		auto now = system_clock::now();
		//auto now_time_t = system_clock::to_time_t(now);
		auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

		stringstream ss;
		
		// print timestamp and tag
		ss << Frag::TimeStamp()
			<< "[" << get<string>(args.at(Tag::type)) << "]"
			<< ": " << msg;

		// return formatted string
		return ss.str();
	}

	string ErrFormat(string msg, unordered_map<Tag, ArgValue> args)
	{
		using namespace std::chrono;
		// get current time;
		auto now = system_clock::now();
		//auto now_time_t = system_clock::to_time_t(now);
		auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

		std::stringstream ss;
		// print timestamp and tag
		ss << Frag::TimeStamp()
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
}

namespace HashColon::LogUtils
{
	using namespace std;

	ostream& operator<< (ostream& lhs, Flashl rhs)
	{
		lhs << rhs._msg << flush;
		lhs << '\r' << string(rhs._msg.length(), ' ') << '\r';		
		return lhs;
	}
}

namespace HashColon::LogUtils::Frag
{
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
}

namespace HashColon::LogUtils::Frag
{
	ostream& operator<< (ostream& lhs, TimeStamp rhs)
	{
		using namespace std::chrono;
		// get current time;
		auto now = system_clock::now();
		auto now_time_t = system_clock::to_time_t(now);
		auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
		// print timestamp and tag
		lhs << "[" << put_time(localtime(&now_time_t), "%F %T.") << setfill('0') << setw(3) << ms.count() << "]";

		return lhs;
	}
}