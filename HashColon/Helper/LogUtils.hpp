#pragma once
#ifndef HASHCOLON_HELPER_LOGUTILS_HPP
#define HASHCOLON_HELPER_LOGUTILS_HPP

#include <string>
#include <variant>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <iostream>

namespace HASHCOLON::Helper::LogUtils
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

namespace HASHCOLON::Helper::LogUtils
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

namespace HASHCOLON::Helper::LogUtils::Frag
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

namespace HASHCOLON::Helper::LogUtils::Frag
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
{ HASHCOLON::Helper::LogUtils::Tag::file, __FILE__ }, \
{ HASHCOLON::Helper::LogUtils::Tag::line, __LINE__ }, \
{ HASHCOLON::Helper::LogUtils::Tag::func, __FUNC__ } 

#endif