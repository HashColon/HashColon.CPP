#ifndef HASHCOLON_HELPER_HPP
#define HASHCOLON_HELPER_HPP

// std libraries
#include <string>
#include <vector>
// dependant external libraries
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/type_index.hpp>

// Filesystem helper functions
namespace HashColon::Fs
{
	// Get files in the given directories 'recursively'
	std::vector<std::string> GetFilesInDirectories(
		const std::vector<std::string>& iDirs,
		const std::vector<std::string> additonalFiles = {},
		const std::string filterRegexStr = ".*");


	std::vector<std::string> GetFilesFromPaths(
		const std::vector<std::string>& iPaths,
		const std::string filterRegexStr = ".*");

	// Get files in the given directory 'recursively'
	std::vector<std::string> GetFilesInDirectory(
		const std::string iDir,
		const std::vector<std::string> additonalFiles = {},
		const std::string filterRegexStr = ".*");

	std::string RefinedAbsolutePathStr(const std::string iPathString);

	bool BuildDirectoryStructure(const std::string iDirectoryPathString);

	bool RemoveAllInDirectory(const std::string iDirectoryPathString);
}

// String helper functions
namespace HashColon::String
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

//// Typenames helper functions
//namespace HashColon::TypeName
//{
//	template <typename T>
//	inline std::string ShortTypename()
//	{
//		return HashColon::String::Split(boost::typeindex::type_id_with_cvr<T>.pretty_name()).back();
//	}
//
//	template <typename T>
//	inline std::string LongTypename()
//	{
//		return boost::typeindex::type_id_with_cvr<T>.pretty_name();
//	}
//}

#endif
