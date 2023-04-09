#ifndef _HG_HASHCOLON_EXCEPTION
#define _HG_HASHCOLON_EXCEPTION

#include <HashColon/header>

#include <exception>
#include <map>
#include <stdexcept>
#include <string>

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

namespace HashColon
{
    /// @brief Base class for HashColon-defined exceptions. This class is derived from std::runtime_error
    class Exception : public std::runtime_error
    {
    protected:
        // std::string name;	// name of the exception
        // std::string what;	// description of the exception

        /// @brief Source file where the exception is thrown.
        const std::string _file;

        /// @brief Line number where the exception is thrown.
        const int _line;

        /// @brief The function where the exception is thrown
        const std::string _func;

        /// @brief Constructor
        /// @param msg Exception message
        /// @param file The source file where the exception is thrown.
        /// @param line The line where the exception is thrown
        /// @param func The function where the exception is thrown
        Exception(
            const std::string msg = "Error",
            const std::string file = "???",
            const int line = 0,
            const std::string func = "???")
            : std::runtime_error(msg), _file(file), _line(line), _func(func){};

        /// @brief Constructor using mapped parameters
        /// @param args parameter maps: {"message": message, "file": file, "line": line, "func": func}
        Exception(std::map<std::string, std::string> args)
            : std::runtime_error(args.at("message")),
              _file(args.at("file")), _line(std::stoi(args.at("line"))), _func(args.at("func")){};

    public:
        /// @brief Returns arguments map of the exception
        /// @return Map of the args
        const std::map<const std::string, const std::string> Args() const
        {
            return {
                {"name", name()},
                {"message", what()},
                {"file", _file},
                {"line", std::to_string(_line)},
                {"func", _func}};
        };

        /// @brief Returns formatted string of the exception for screen output
        /// @details "@file#line\n\t{func()}: name: \twhat"
        /// @return formatted exception string
        std::string formattedString() const
        {
            return "@" + _file + "#" + std::to_string(_line) + "\n\t{" + func() + "()}: " + name() + ":\n\t" + what();
        };

        /// @brief Returns the name of the exception. The name is defined in the derived exceptions.
        /// @return Name in const string
        virtual std::string name() const { return "HashColon error"; };

        /// @brief Returns the file where the exception is thrown.
        /// @return File name in const string
        const std::string &file() const { return _file; };

        /// @brief Return the line number where the exception is thrown.
        /// @return Line number in const int
        const int &line() const { return _line; };

        /// @brief Return the function where the exception is thrown.
        /// @return Function name in const string
        const std::string &func() const { return _func; };
    };

    /// @brief A dummy exception to indicate a function/object is not implemented.
    class NotImplementedException : public std::logic_error
    {
    protected:
        /// @brief Name of the exception: "Not implemented."
        inline static const std::string _name = "Not implemented.";
        // std::string what;	// description of the exception

        /// @brief Source file where the exception is thrown.
        const std::string _file;

        /// @brief Line number where the exception is thrown.
        const int _line;

        /// @brief The function where the exception is thrown
        const std::string _func;

    public:
        /// @brief Constructor with full arguments
        /// @param file The source file where the exception is thrown.
        /// @param line The line where the exception is thrown
        /// @param func The function where the exception is thrown
        NotImplementedException(
            const std::string file = "???",
            const int line = 0,
            const std::string func = "???")
            : std::logic_error(_name), _file(file), _line(line), _func(func){};

        /// @brief Returns arguments map of the exception
        /// @return Map of the args
        const std::map<const std::string, const std::string> Args() const
        {
            return {
                {"name", _name},
                {"message", (std::string)what()},
                {"file", _file},
                {"line", std::to_string(_line)},
                {"func", _func}};
        };

        /// @brief Returns formatted string of the exception for screen output
        /// @details "@file#line\n\t{func()}: name: \twhat"
        /// @return formatted exception string
        std::string formattedString() const
        {
            return "@" + _file + "#" + std::to_string(_line) + "\n\t{" + func() + "()}: " + name() + ":\n\t" + what();
        };

        /// @brief Returns the name of this exception: "Not implemented."
        /// @return "Not implemented."
        std::string name() const { return _name; };

        /// @brief Returns the file where the exception is thrown.
        /// @return File name in const string
        const std::string &file() const { return _file; };

        /// @brief Return the line number where the exception is thrown.
        /// @return Line number in const int
        const int &line() const { return _line; };

        /// @brief Return the function where the exception is thrown.
        /// @return Function name in const string
        const std::string &func() const { return _func; };
    };
}

// Alias for NotImplemented Exception with code debug information
#define NOT_IMPLEMENTED_EXCEPTION \
    HashColon::NotImplementedException(__CODEINFO__)

#define HASHCOLON_CLASS_EXCEPTION_DEFINITION(classname)                   \
    class Exception : public HashColon::Exception                         \
    {                                                                     \
    public:                                                               \
        Exception(                                                        \
            const std::string msg,                                        \
            const std::string file = __FILE__,                            \
            const int line = __LINE__,                                    \
            const std::string func = __FUNC__)                            \
            : HashColon::Exception(msg, file, line, func){};              \
        virtual std::string name() const override                         \
        {                                                                 \
            return "Exception from " + std::string(STR(classname)) + "."; \
        };                                                                \
    }

#define HASHCOLON_NAMED_EXCEPTION_DEFINITION(exception_name)                    \
    class exception_name##Exception : public HashColon::Exception               \
    {                                                                           \
    public:                                                                     \
        exception_name##Exception(                                              \
            const std::string msg,                                              \
            const std::string file = __FILE__,                                  \
            const int line = __LINE__,                                          \
            const std::string func = __FUNC__)                                  \
            : HashColon::Exception(msg, file, line, func){};                    \
        virtual std::string name() const override                               \
        {                                                                       \
            return "Exception named " + std::string(STR(exception_name)) + "."; \
        };                                                                      \
    }

#endif