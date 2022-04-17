#ifndef HASHCOLON_LOG_IMPL
#define HASHCOLON_LOG_IMPL

// header file for this source file
#include <HashColon/Log.hpp>
// std libraries
#include <ios>
#include <sstream>
#include <streambuf>
#include <string>

namespace HashColon
{
	template<typename T>
	std::streamsize LogStreamBuf<T>::xsputn(const char* s, std::streamsize n)
	{
		// if internal stringstream is empty, apply lock
		if (_ss.tellp() == std::streampos(0))
		{
			_mutex.try_lock();
		}

		// store string in _ss 
		// _ss stream will flushed in _Logger::sync
		_ss << std::string(s, n);
		return n;
	}

	template<typename T>
	std::streambuf::int_type LogStreamBuf<T>::overflow(std::streambuf::int_type c)
	{
		if (c == traits_type::eof())
			return traits_type::eof();
		else
		{
			char_type ch = traits_type::to_char_type(c);
			return xsputn(&ch, 1) == 1 ? c : traits_type::eof();
		}
	}

	template<typename T>
	int LogStreamBuf<T>::sync()
	{
		using namespace std;
		if (_filter(_args))
		{
			std::string formattedString = _formatter(_ss.str(), _args);
			for (auto& aStream : _streams)
				(*aStream) << formattedString << flush;
		}

		// clear string after flush
		stringstream().swap(_ss);
		_mutex.unlock();
		return 0;
	}
}

namespace HashColon
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

namespace HashColon
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
