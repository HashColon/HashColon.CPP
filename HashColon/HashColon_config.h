#ifndef HASHCOLON_CONFIG_H
#define HASHCOLON_CONFIG_H

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

// Set real as double
#ifndef HASHCOLON_REAL_AS_DOUBLEPRECISION
#define HASHCOLON_REAL_AS_DOUBLEPRECISION
#endif 

// Set real as float
//#ifndef HASHCOLON_REAL_AS_SINGLEPRECISION
//#define HASHCOLON_REAL_AS_SINGLEPRECISION
//#endif 

#ifndef EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#endif 

// define for using grand route in Feline
// #define Feline_DISTANCE_USING_GRANDROUTE

// disable intellisense error: identifier "__float128" is undefined
//#ifdef __INTELLISENSE__ 
////using __float128 = long double; // or some fake 128 bit floating point type
////#define __GNUC__ 10
////#define __GNUC__MINOR__ 3
//#ifdef __GNUC__
//#define TestXSTR(x) TestSTR(x)
//#define TestSTR(x) #x
//#error TestXSTR(__GNUC__)
//#else
//#error hinghing
//#endif 
//
//#endif



#endif
