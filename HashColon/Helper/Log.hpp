#ifndef HASHCOLON_HELPER_LOG_HPP
#define HASHCOLON_HELPER_LOG_HPP

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

namespace HashColon::Helper
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
		virtual std::streamsize xsputn(const char * s, std::streamsize n) final override
		{
			// store string in _ss 
			// _ss stream will flushed in _Logger::sync
			_ss << std::string(s, n);
			return n;
		}

		virtual int_type overflow(int_type c = traits_type::eof()) final override
		{
			if (c == traits_type::eof())
				return traits_type::eof();
			else
			{
				char_type ch = traits_type::to_char_type(c);
				return xsputn(&ch, 1) == 1 ? c : traits_type::eof();
			}
		}

		virtual int sync() final override
		{
			if (_filter(_args)) 
			{
				std::string formattedString = _formatter(_ss.str(), _args);
				for (auto& aStream : _streams)
					(*aStream) << formattedString << std::flush;
			}

			// clear string after flush
			std::stringstream().swap(_ss);

			return 0;
		}

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
			: _streams(rhs._streams), _args(rhs._args),
			_ss(), _formatter(rhs._formatter), _filter(rhs._filter)
		{};
		LogStreamBuf(
			std::vector<std::shared_ptr<std::ostream>> iStreams = {},
			FormatterType iFormattter = nullptr,
			FilterType iFilter = nullptr,
			ArgListType iArglist = {})
			: _streams(iStreams), _formatter(iFormattter),
			_args(iArglist), _ss(), _filter(iFilter)
		{};
	};
}

namespace HashColon::Helper
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
			: _internalStream(logger._internalStream),
			std::ostream(&_internalStream)
		{};

		Logger(const LogStreamBuf<tagtype>* logstream)
			: _internalStream(*logstream), std::ostream(&_internalStream)
		{};

		Logger(
			std::vector<std::shared_ptr<std::ostream>> iStreams = {},
			FormatterType iFormatter = nullptr,
			FilterType iFilter = nullptr,
			ArgListType iArglist = {})
			: _internalStream(iStreams, iFormatter, iFilter, iArglist),
			std::ostream(&_internalStream)
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

namespace HashColon::Helper
{
	template <typename T>	
	LogStreamBuf<T> LogStreamBuf<T>::withArgs(const ArgListType args) const
	{
		// copy current object;
		LogStreamBuf tempLogStream = (*this);
		ArgListType tempArgs = args;
		tempArgs.insert(tempLogStream.Arguments().begin(), tempLogStream.Arguments().end());
		tempLogStream.Arguments() = tempArgs;
		// return temporary object using argument list
		return tempLogStream;
	}

	template <typename T>
	LogStreamBuf<T> LogStreamBuf<T>::withFormatter(const FormatterType formatter) const
	{
		LogStreamBuf tempLogStream = (*this);
		tempLogStream.Formatter() = formatter;
		return tempLogStream;
	}

	template <typename T>
	LogStreamBuf<T> LogStreamBuf<T>::withFilter(const FormatterType filter) const
	{
		LogStreamBuf tempLogStream = (*this);
		tempLogStream.Filter() = filter;
		return tempLogStream;
	}

	template<typename T>
	LogStreamBuf<T> LogStreamBuf<T>::operator() () const
	{
		return (*this);
	}

	template <typename T>	
	LogStreamBuf<T> LogStreamBuf<T>::operator() (const ArgListType args) const
	{
		return withArgs(args);
	}

	template<typename T>
	LogStreamBuf<T> LogStreamBuf<T>::operator() (const FormatterType formatter) const
	{
		return withFormatter(formatter);
	}

	template<typename T>
	LogStreamBuf<T> LogStreamBuf<T>::operator() (const FilterType filter) const
	{
		return withFilter(filter);
	}

	template<typename T>	
	LogStreamBuf<T> LogStreamBuf<T>::operator()(const FormatterType formatter, const ArgListType args) const
	{
		// copy current object & set formatter;
		LogStreamBuf tempLogStream = (*this);

		ArgListType tempArgs = args;
		tempArgs.insert(tempLogStream.Arguments().begin(), tempLogStream.Arguments().end());
		tempLogStream.Arguments() = tempArgs;

		tempLogStream.Formatter() = formatter;

		// return temporary object using argument list
		return tempLogStream;
	}

	template<typename T>	
	LogStreamBuf<T> LogStreamBuf<T>::operator()(const FilterType filter, const ArgListType args) const
	{
		// copy current object & set formatter;
		LogStreamBuf tempLogStream = (*this);

		ArgListType tempArgs = args;
		tempArgs.insert(tempLogStream.Arguments().begin(), tempLogStream.Arguments().end());
		tempLogStream.Arguments() = tempArgs;

		tempLogStream.Filter() = filter;

		// return temporary object using argument list
		return tempLogStream;
	}

	template<typename T>
	LogStreamBuf<T> LogStreamBuf<T>::operator()(const FormatterType formatter, const FilterType filter) const
	{
		// copy current object & set formatter;
		LogStreamBuf tempLogStream = (*this);

		tempLogStream.Formatter() = formatter;
		tempLogStream.Filter() = filter;

		// return temporary object using argument list
		return tempLogStream;
	}

	template<typename T>	
	LogStreamBuf<T> LogStreamBuf<T>::operator()(const FormatterType formatter, const FilterType filter, const ArgListType args) const
	{
		// copy current object & set formatter;
		LogStreamBuf tempLogStream = (*this);

		ArgListType tempArgs = args;
		tempArgs.insert(tempLogStream.Arguments().begin(), tempLogStream.Arguments().end());
		tempLogStream.Arguments() = tempArgs;

		tempLogStream.Formatter() = formatter;
		tempLogStream.Filter() = filter;

		// return temporary object using argument list
		return tempLogStream;
	}
}

namespace HashColon::Helper
{
	template<typename T>
	Logger<T> Logger<T>::withArgs(const ArgListType args) const
	{
		LogStreamBuf lsb = _internalStream;

		ArgListType tempArgs = args;
		tempArgs.insert(lsb.Arguments().begin(), lsb.Arguments().end());
		lsb.Arguments() = tempArgs;

		return Logger<T>(&lsb);
	}

	template<typename T>
	Logger<T> Logger<T>::withFormatter(const FormatterType formatter) const
	{
		LogStreamBuf lsb = _internalStream;
		lsb.Formatter() = formatter;
		return Logger<T>(&lsb);
	}

	template<typename T>
	Logger<T> Logger<T>::withFilter(const FilterType filter) const
	{
		LogStreamBuf lsb = _internalStream;
		lsb.Filter() = filter;
		return Logger<T>(&lsb);
	}

	template<typename T>
	Logger<T> Logger<T>::operator() () const
	{
		return (*this);
	}

	template<typename T>
	Logger<T> Logger<T>::operator() (const ArgListType args) const
	{
		return withArgs(args);
	}

	template<typename T>
	Logger<T> Logger<T>::operator() (const FormatterType formatter) const
	{
		return withFormatter(formatter);
	}

	template<typename T>
	Logger<T> Logger<T>::operator() (const FilterType filter) const
	{
		return withFilter(filter);
	}

	template<typename T>
	Logger<T> Logger<T>::operator() (const FormatterType formatter, const ArgListType args) const
	{
		LogStreamBuf lsb = _internalStream;

		ArgListType tempArgs = args;
		tempArgs.insert(lsb.Arguments().begin(), lsb.Arguments().end());
		lsb.Arguments() = tempArgs;

		lsb.Formatter() = formatter;
		
		return Logger<T>(&lsb);
	}

	template<typename T>
	Logger<T> Logger<T>::operator() (const FilterType filter, const ArgListType args) const
	{
		LogStreamBuf lsb = _internalStream;

		ArgListType tempArgs = args;
		tempArgs.insert(lsb.Arguments().begin(), lsb.Arguments().end());
		lsb.Arguments() = tempArgs;

		lsb.Filter() = filter;
		
		return Logger<T>(&lsb);
	}

	template<typename T>
	Logger<T> Logger<T>::operator() (const FormatterType formatter, const FilterType filter) const
	{
		LogStreamBuf lsb = _internalStream;
		lsb.Formatter() = formatter;
		lsb.Filter() = filter;
		return Logger<T>(&lsb);
	}

	template<typename T>
	Logger<T> Logger<T>::operator() (const FormatterType formatter, const FilterType filter, const ArgListType args) const
	{
		LogStreamBuf lsb = _internalStream;

		ArgListType tempArgs = args;
		tempArgs.insert(lsb.Arguments().begin(), lsb.Arguments().end());
		lsb.Arguments() = tempArgs;

		lsb.Formatter() = formatter;
		lsb.Filter() = filter;
		
		return Logger<T>(&lsb);
	}
}

#endif

