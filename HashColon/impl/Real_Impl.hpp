#ifndef HASHCOLON_REAL_IMPL
#define HASHCOLON_REAL_IMPL

// HashColon config
#include <HashColon/HashColon_config.h>
// std libraries
#include <cmath>
#include <stdexcept>
// header file for this source file
#include <HashColon/Real.hpp>

namespace HashColon
{
	template <const Real& minVal, const Real& maxVal>
	BoundedReal<minVal, maxVal, false>::BoundedReal(const Real& val)
	{
		(minVal <= val && maxVal >= val)
			? _val = val
			: throw std::range_error("BoundedReal out-of-range error"); 
	}
	
	template <const Real& minVal, const Real& maxVal>
	BoundedReal<minVal, maxVal, false>& BoundedReal<minVal, maxVal, false>::operator=(Real val)
	{
		(minVal <= val && maxVal >= val)
			? _val = val
			: throw std::range_error("BoundedReal out-of-range error");
		return *this;
	}

	template <const Real& minVal, const Real& maxVal>
	BoundedReal<minVal, maxVal, true>::BoundedReal(const Real& val)
	{		
		(minVal <= val && maxVal >= val)
			? _val = val
			: _val = std::remainder(val - .5 * (minVal + maxVal), maxVal - minVal) +
			.5 * (minVal + maxVal);
	};
		
	template <const Real& minVal, const Real& maxVal>
	BoundedReal<minVal, maxVal, true>& BoundedReal<minVal, maxVal, true>::operator=(Real val)
	{
		(minVal <= val && maxVal >= val)
			? _val = val
			: _val = std::remainder(val - .5 * (minVal + maxVal), maxVal - minVal) +
			.5 * (minVal + maxVal);
		return *this;
	};
}

#endif
