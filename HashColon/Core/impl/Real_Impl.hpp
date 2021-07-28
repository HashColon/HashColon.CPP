#ifndef HASHCOLON_CORE_REAL_IMPL
#define HASHCOLON_CORE_REAL_IMPL

#include <HashColon/Core/Real.hpp>

namespace HashColon
{
	template <const Real& minVal, const Real& maxVal>
	BoundedReal<minVal, maxVal, false>::BoundedReal(const Real& val)
	{
		(minVal <= val && maxVal >= val)
			? _val = val
			: throw std::range_error(
				"error"); // todo : logwriter : throw exception with log
	}
	
	template <const Real& minVal, const Real& maxVal>
	BoundedReal<minVal, maxVal, false>& BoundedReal<minVal, maxVal, false>::operator=(Real val)
	{
		(minVal <= val && maxVal >= val)
			? _val = val
			: throw std::range_error(
				"error"); // todo : logwriter : throw exception with log
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
