#ifndef HASHCOLON_HELPER_STRINGHELPER_HPP
#define HASHCOLON_HELPER_STRINGHELPER_HPP

#include <string>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace HashColon::Helper
{
	// trim from left 
	inline std::string& TrimStart(std::string& s, const char* t = " \t\n\r\f\v") 
	{ s.erase(0, s.find_first_not_of(t)); return s; } 
	
	// trim from right 
	inline std::string& TrimEnd(std::string& s, const char* t = " \t\n\r\f\v") 
	{ s.erase(s.find_last_not_of(t) + 1); return s; } 
	
	// trim from left & right 
	inline std::string& Trim(std::string& s, const char* t = " \t\n\r\f\v") 
	{ return TrimStart(TrimEnd(s, t), t); } 
	
	// copying versions 
	inline std::string TrimStartCopy(std::string s, const char* t = " \t\n\r\f\v") 
	{ return TrimStart(s, t); } 
	
	inline std::string TrimEndCopy(std::string s, const char* t = " \t\n\r\f\v") 
	{ return TrimEnd(s, t); } 
	
	inline std::string TrimCopy(std::string s, const char* t = " \t\n\r\f\v") 
	{ return Trim(s, t); }

	// split string
	inline std::vector<std::string> Split(std::string s, const char* spliter)
	{
		std::vector<std::string> re;
		boost::split(re, s, boost::is_any_of(spliter));
		return re;
	}

	// case convolution
	inline std::string& ToLower(std::string& s)
	{ boost::to_lower(s); return s;	}

	inline std::string& ToUpper(std::string& s)
	{ boost::to_upper(s); return s;	}

	inline std::string ToLowerCopy(std::string s)
	{ return ToLower(s); }

	inline std::string ToUpperCopy(std::string s)
	{ return ToUpper(s); }

}

#endif
