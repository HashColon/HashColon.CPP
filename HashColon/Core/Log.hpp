#ifndef HASHCOLON_CORE_LOG_HPP
#define HASHCOLON_CORE_LOG_HPP

#include <streambuf>
#include <unordered_map>
#include <functional>
#include <variant>
#include <sstream>
#include <mutex>
#include <vector>
#include <memory>
#include <ostream>
#include <string>

namespace HashColon
{
	template <typename tagtype>
	class LogStreamBuf : public std::streambuf
	{
	public:
		using ArgValue = std::variant<char, int, std::string, double>;
		using ArgType = std::pair<tagtype, ArgValue>;
		using ArgListType = std::unordered_map<tagtype, ArgValue>;
		using FormatterType = std::function<std::string(std::string, ArgListType)>;
		using FilterType = std::function<bool(ArgListType)>;

	protected:
		virtual std::streamsize xsputn(const char* s, std::streamsize n) final override;

		virtual int_type overflow(int_type c = traits_type::eof()) final override;
		
		virtual int sync() final override;
				
	protected:
		ArgListType _args;
		FormatterType _formatter;
		FilterType _filter;
		std::stringstream _ss;
		std::vector<std::shared_ptr<std::ostream>> _streams;

	public:
		ArgListType& Arguments() { return _args; };
		FormatterType& Formatter() { return _formatter; };
		FilterType& Filter() { return _filter; };
		std::vector<std::shared_ptr<std::ostream>>& StreamList() { return _streams; };

		LogStreamBuf<tagtype> withArgs(const ArgListType args) const;
		LogStreamBuf<tagtype> withFormatter(const FormatterType formatter) const;
		LogStreamBuf<tagtype> withFilter(const FormatterType filter) const;

		LogStreamBuf<tagtype> operator() () const;
		LogStreamBuf<tagtype> operator() (const ArgListType args) const;
		LogStreamBuf<tagtype> operator() (const FormatterType formatter) const;
		LogStreamBuf<tagtype> operator() (const FilterType filter) const;

		LogStreamBuf<tagtype> operator() (const FormatterType formatter, const ArgListType args) const;
		LogStreamBuf<tagtype> operator() (const FilterType filter, const ArgListType args) const;
		LogStreamBuf<tagtype> operator() (const FormatterType formatter, const FilterType filter) const;
		LogStreamBuf<tagtype> operator() (const FormatterType formatter, const FilterType filter, const ArgListType args) const;

	public:
		LogStreamBuf(const LogStreamBuf<tagtype>& rhs)
			: _args(rhs._args), _formatter(rhs._formatter), _filter(rhs._filter),
			_ss(), _streams(rhs._streams)
		{};
		LogStreamBuf(
			std::vector<std::shared_ptr<std::ostream>> iStreams = {},
			FormatterType iFormattter = nullptr,
			FilterType iFilter = nullptr,
			ArgListType iArglist = {})
			: _args(iArglist), _formatter(iFormattter), _filter(iFilter),
			 _ss(), _streams(iStreams)
		{};
	};
}

namespace HashColon
{
	/*
		This is the base class for logging. 
		To use this class, tag type(enum)/filter function/format function needs to be defined.
		Do not use this class for direct logging. use CommonLogger instead 

		<Logger ingredients descriptions>
		tagtype: enum for argument tags. all arguments will be passed as {tag, value} pairs
		Formatter: formatting function in form: string [func](string, { {tag, value}, ... } )
				   example) 
				       string TimestampedMessages(string msg, { { tag::time, timevalue} } )
					   this formatter gets msg as original message, timevalue as given argument name time.
					   this formatter may give output as "[timevalue] msg"				       
		Filter: filter function in form: bool [func]( { {tag, value}, ... } )
				this function determines wheter the logging message should be shown or not.
				example)
				       bool levelFilter( { { tag::lvl, level } } )
					   If the given level is over certain value, the function will return false.
					   Which will suppress the message.
	*/
	template <typename tagtype>
	class Logger : public std::ostream
	{
	public:
		using ArgValue = std::variant<char, int, std::string, double>;
		using ArgType = std::pair<tagtype, ArgValue>;
		using ArgListType = std::unordered_map<tagtype, ArgValue>;
		using FormatterType = std::function<std::string(std::string, ArgListType)>;
		using FilterType = std::function<bool(ArgListType)>;

	protected:
		LogStreamBuf<tagtype> _internalStream;

	public:
		Logger(const Logger<tagtype>& logger)
			: std::ostream(&_internalStream), _internalStream(logger._internalStream)
		{};

		Logger(const LogStreamBuf<tagtype>* logstream)
			: std::ostream(&_internalStream), _internalStream(*logstream)
		{};

		Logger(
			std::vector<std::shared_ptr<std::ostream>> iStreams = {},
			FormatterType iFormatter = nullptr,
			FilterType iFilter = nullptr,
			ArgListType iArglist = {})
			: std::ostream(&_internalStream),
			_internalStream(iStreams, iFormatter, iFilter, iArglist)			
		{};

	public:
		LogStreamBuf<tagtype>& Stream() { return _internalStream; };

		Logger<tagtype> withArgs(const ArgListType arg) const;		
		Logger<tagtype> withFormatter(const FormatterType formatter) const;
		Logger<tagtype> withFilter(const FilterType filter) const;

		Logger<tagtype> operator() () const;
		Logger<tagtype> operator() (const ArgListType arg) const;
		Logger<tagtype> operator() (const FormatterType formatter) const;
		Logger<tagtype> operator() (const FilterType filter) const;

		Logger<tagtype> operator() (const FormatterType formatter, const ArgListType arg) const;		
		Logger<tagtype> operator() (const FilterType filter, const ArgListType arg) const;
		Logger<tagtype> operator() (const FormatterType formatter, const FilterType filter) const;		
		Logger<tagtype> operator() (const FormatterType formatter, const FilterType filter, const ArgListType arg) const;
				
	};
}

#endif

#include <HashColon/Core/impl/Log_Impl.hpp>
