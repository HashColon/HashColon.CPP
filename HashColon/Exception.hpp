#ifndef HASHCOLON_EXCEPTION_HPP
#define HASHCOLON_EXCEPTION_HPP

// std libraries
#include <exception>
#include <map>
#include <stdexcept>
#include <string>

// macros
namespace HashColon
{
// define __FUNC__ (preprocessor macro for function of the current code)
#ifndef __FUNC__
#if defined __GNUC__
#define __FUNC__ __PRETTY_FUNCTION__
#elif defined _MSC_VER
#define __FUNC__ __FUNCTION__
#else
#define __FUNC__ "???"
#endif	
#endif

// define stringifiers
#ifndef STRINGIFIER
#define STRINGIFIER(X) #X
#endif // !STRINGIFIER
#ifndef STR
#define STR(X) STRINGIFIER(X)
#endif // !STR

// define code info macro
#ifndef __CODEINFO__
#define __CODEINFO__ __FILE__, __LINE__, __FUNC__
#endif // !__CODEINFO__
#ifndef __CODEINFO_STR__
#define __CODEINFO_STR__ __FILE__, STR(__LINE__), __FUNC__
#endif // !__CODEINFO_STR__
}

// exception
namespace HashColon
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
			_file(args.at("file")), _line(std::stoi(args.at("line"))), _func(args.at("func")) {};

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
		std::string func() const { return _func; };

	};
}

#define NotImplementedException \
HashColon::NotImplementedException(__CODEINFO__)

#define HASHCOLON_CLASS_EXCEPTION_DEFINITION(classname) \
class Exception : public HashColon::Exception \
{	\
public:	\
	Exception(	\
		const std::string msg,	\
		const std::string file = __FILE__,	\
		const int line = __LINE__,	\
		const std::string func = __FUNC__)	\
		: HashColon::Exception(msg, file, line, func) {};	\
	virtual std::string name() const override { return std::string(STR(classname)) + " error"; };	\
}

#define HASHCOLON_NAMED_EXCEPTION_DEFINITION(exception_name) \
class exception_name##Exception : public HashColon::Exception \
{	\
public:	\
	exception_name##Exception(	\
		const std::string msg,	\
		const std::string file = __FILE__,	\
		const int line = __LINE__,	\
		const std::string func = __FUNC__)	\
		: HashColon::Exception(msg, file, line, func) {};	\
	virtual std::string name() const override { return std::string(STR(exception_name)) + " error"; };	\
}

#endif  // HASHCOLON_HELPER_EXCEPTION_H
