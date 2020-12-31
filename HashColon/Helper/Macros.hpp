#ifndef HASHCOLON_HELPER_MACROS_HPP
#define HASHCOLON_HELPER_MACROS_HPP

/*
 * Compiler configuration
 */
#ifdef __GNUC__	
	#define DLLExport
#elif _MSC_VER >= 1900	
	#define DLLExport __declspec(dllexport)
#else
	#error Use GCC or MSVC2015 or higher
#endif

// c++ version configuration
#if __cplusplus < 201300  // c++14
	#error c++14 required!
#endif

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

// define stringifier
#define STRINGIFIER(X) #X
#define STR(X) STRINGIFIER(X)

// define code info macro
#define __CODEINFO__ __FILE__, __LINE__, __FUNC__
#define __CODEINFO_STR__ __FILE__, STR(__LINE__), __FUNC__

#endif