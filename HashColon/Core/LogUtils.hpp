#ifndef HASHCOLON_CORE_LOGUTILS_HPP
#define HASHCOLON_CORE_LOGUTILS_HPP

#include <HashColon/Core/Macros.hpp>

#include <string>
#include <variant>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <iostream>

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
namespace HashColon::LogUtils
{	
	using ArgValue = std::variant<char, int, std::string, double>;

	/*static const shared_ptr<ostream> Stdout;
	static const shared_ptr<ostream> Stderr;*/
	const std::shared_ptr<std::ostream> Stdout = std::shared_ptr<std::ostream>(&std::cout, [](std::ostream*) {});
	const std::shared_ptr<std::ostream> Stderr = std::shared_ptr<std::ostream>(&std::cerr, [](std::ostream*) {});

	enum class Tag { type, file, line, func, lvl, maxlvl };

	std::string LogFormat(std::string msg, std::unordered_map<Tag, ArgValue> args);
	std::string ErrFormat(std::string msg, std::unordered_map<Tag, ArgValue> args);
	std::string NullFormat(std::string msg, std::unordered_map<Tag, ArgValue> args);
	std::string BasicFormat(std::string msg, std::unordered_map<Tag, ArgValue> args);

	bool PassFilter(std::unordered_map<Tag, ArgValue> args);
	bool BlockFilter(std::unordered_map<Tag, ArgValue> args);
	bool VerboseFilter(std::unordered_map<Tag, ArgValue> args);
}

namespace HashColon::LogUtils
{
	class Flashl
	{
	private:
		std::string _msg;
		struct Ignitor {};
		struct Terminator {};
	public:
		Flashl(std::string msg) : _msg(msg) {};
		friend std::ostream& operator<< (std::ostream& lhs, Flashl rhs);
	};

	std::ostream& operator<< (std::ostream& lhs, Flashl rhs);
}

namespace HashColon::LogUtils::Frag
{
	class Percentage
	{
	private:
		double _percentage;
	public:
		Percentage(int item_done, int item_total);
		Percentage(double percentage);
		friend std::ostream& operator<< (std::ostream& lhs, Percentage rhs);
	};
	std::ostream& operator<< (std::ostream& lhs, Percentage rhs);
}

namespace HashColon::LogUtils::Frag
{
	class TimeStamp
	{	
	public:
		TimeStamp() {};
		friend std::ostream& operator<< (std::ostream& lhs, TimeStamp rhs);
	};
	std::ostream& operator<< (std::ostream& lhs, TimeStamp rhs);
}

#define __CODEINFO_TAGS__ \
{ HashColon::LogUtils::Tag::file, __FILE__ }, \
{ HashColon::LogUtils::Tag::line, __LINE__ }, \
{ HashColon::LogUtils::Tag::func, __FUNC__ } 

#endif