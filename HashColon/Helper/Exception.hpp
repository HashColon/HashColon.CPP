#ifndef HASHCOLON_HELPER_EXCEPTION_HPP
#define HASHCOLON_HELPER_EXCEPTION_HPP

#include <exception>
#include <stdexcept>
#include <string>
#include <map>
#include <HashColon/Helper/Macros.hpp>

namespace HashColon
{
	namespace Exceptions
	{
		class Exception : public std::runtime_error
		{
		protected:
			//std::string name;	// name of the exception
			//std::string what;	// description of the exception
			std::string _file;	// source file of the exception code(full path)
			int _line;			// source line number of the exception code
			std::string _func;	// function name including parent classes of the error code

			Exception(
				const std::string msg = "Error",
				const std::string file = "???",
				const int line = 0,
				const std::string func = "???")
				: std::runtime_error(msg), _file(file), _line(line), _func(func) {};

			Exception(std::map<std::string, std::string> args)
				: std::runtime_error(args.at("message")),
				_line(std::stoi(args.at("line"))),
				_file(args.at("file")), _func(args.at("func")) {};

		public:
			std::map<std::string, std::string> Args()
			{
				return {
					{"name", name()},
					{"message", what()},
					{"file", _file},
					{"line", std::to_string(_line)},
					{"func", _func}
				};
			};
			std::string formattedString()
			{
				return "@" + _file + "#" + std::to_string(_line) + "\n"
					+ "\t{" + __FUNC__ + "()}: " + name() + ":\n"
					+ "\t" + what();
			};
			virtual std::string name() const { return "HASHCOLON error"; };
			std::string file() const { return _file; };
			int line() const { return _line; };
			std::string func() const { return _func; };
		};

		class NotImplementedException : public std::logic_error
		{
		protected:
			//std::string name;	// name of the exception
			//std::string what;	// description of the exception
			std::string _file;	// source file of the error code(full path)
			int _line;			// source line number of the error code
			std::string _func;	// function name including parent classes of the error code

		public:
			NotImplementedException(
				const std::string file = "???",
				const int line = 0,
				const std::string func = "???")
				: std::logic_error("Function is not Implemented."), _file(file), _line(line), _func(func) {};

			std::map<std::string, std::string> Args()
			{
				return {
					{"name", name()},
					{"message", (std::string)what()},
					{"file", _file},
					{"line", std::to_string(_line)},
					{"func", _func}
				};
			};
			std::string formattedString()
			{
				return "@" + _file + "#" + std::to_string(_line) + "\n"
					+ "\t{" + _func + "()}: " + name() + ":\n"
					+ "\t" + what();
			};
			std::string name() const { return "Not implemented error"; };
			std::string file() const { return _file; };
			int line() const { return _line; };
			std::string func() const{ return _func; };

		};
	}
}

#define NotImplementedException \
HashColon::Exceptions::NotImplementedException(__CODEINFO__)

#define HASHCOLON_CLASS_EXCEPTION_DEFINITION(classname) \
class Exception : public HashColon::Exceptions::Exception \
{	\
public:	\
	Exception(	\
		const std::string msg,	\
		const std::string file = __FILE__,	\
		const int line = __LINE__,	\
		const std::string func = __FUNC__)	\
		: HashColon::Exceptions::Exception(msg, file, line, func) {};	\
	virtual std::string name() const override { return std::string(STR(classname)) + " error"; };	\
}

#define HASHCOLON_NAMED_EXCEPTION_DEFINITION(exception_name) \
class exception_name##Exception : public HashColon::Exceptions::Exception \
{	\
public:	\
	exception_name##Exception(	\
		const std::string msg,	\
		const std::string file = __FILE__,	\
		const int line = __LINE__,	\
		const std::string func = __FUNC__)	\
		: HashColon::Exceptions::Exception(msg, file, line, func) {};	\
	virtual std::string name() const override { return std::string(STR(exception_name)) + " error"; };	\
}

#endif  // HASHCOLON_HELPER_EXCEPTION_H
