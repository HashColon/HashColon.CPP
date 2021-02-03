#ifndef HASHCOLON_HELPER_REAL_HPP
#define HASHCOLON_HELPER_REAL_HPP
#pragma once

#include <array>
#include <cmath>
#include <stdexcept>
#include <HashColon/Helper/Macros.hpp>

namespace HashColon::Helper
{
	// Real definition
	// define real number as float / double
#if defined HASHCOLON_REAL_AS_DOUBLEPRECISION
	using Real = double;
#elif defined HASHCOLON_REAL_AS_SINGLEPRECISION
	using Real = float;
#else
	using Real = float;
#endif

	// void pointer
	using VoidPtr = void*;

	/// <summary>
	/// value with intervals
	/// </summary>
	template <typename valueTypeT>
	struct ValueInterval
	{
		ValueInterval() : minVal(), repVal(), maxVal() {};

		union
		{
			struct
			{
				valueTypeT minVal;
				valueTypeT repVal;
				valueTypeT maxVal;
			};
			std::array<valueTypeT, 3> val;
		};

		inline valueTypeT& operator[](std::size_t i) { return val[i]; }
	};

	// BoundedReal definition
	template <const Real& minVal, const Real& maxVal, bool isCircular = false>
	class BoundedReal;

	/// <summary>
	// Real values which has min/max boundary
	/// </summary>
	template <const Real& minVal, const Real& maxVal>
	class BoundedReal<minVal, maxVal, false>
	{
	protected:
		Real _val = 0.0;

	public:
		BoundedReal() {}
		BoundedReal(const Real& val)
		{
			(minVal <= val && maxVal >= val)
				? _val = val
				: throw std::range_error(
					"error"); // todo : logwriter : throw exception with log
		}
		operator Real() const { return _val; }

		BoundedReal& operator=(Real val)
		{
			(minVal <= val && maxVal >= val)
				? _val = val
				: throw std::range_error(
					"error"); // todo : logwriter : throw exception with log
			return *this;
		}
	};

	/// <summary>
	// Real values which circulates like degrees.
	/// </summary>
	template <const Real& minVal, const Real& maxVal>
	class BoundedReal<minVal, maxVal, true>
	{
	protected:
		Real _val = 0.0;

	public:
		BoundedReal() {}
		BoundedReal(const Real& val)
		{
			(minVal <= val && maxVal >= val)
				? _val = val
				: _val = std::remainder(val - .5 * (minVal + maxVal), maxVal - minVal) +
				.5 * (minVal + maxVal);
		}
		operator Real() const { return _val; }

		BoundedReal& operator=(Real val)
		{
			(minVal <= val && maxVal >= val)
				? _val = val
				: _val = std::remainder(val - .5 * (minVal + maxVal), maxVal - minVal) +
				.5 * (minVal + maxVal);
			return *this;
		}

		inline const Real toReal() { return _val; }
	};

	namespace _details
	{
		struct hiddenAnchor
		{
			static Real _minDegree;
			static Real _maxDegree;
			static Real _negativePI;
			static Real _positivePI;

			static Real _realZero;
			static Real _realTen;
			static Real _realMax;
		};
	}

	/// <summary>
	/// Angle in degrees. Circular value [0, 360)
	/// </summary>
	/// <seealso cref="BoundedReal{_minDegree, _maxDegree, true}" />	
	class Degree : public BoundedReal<
		_details::hiddenAnchor::_minDegree, 
		_details::hiddenAnchor::_maxDegree, true>
	{
	public:
		Degree() : BoundedReal() {}
		Degree(const Real& val) : BoundedReal(val) {}

		friend inline bool operator==(const Degree& lhs, const Degree& rhs)
		{
			return (lhs._val == rhs._val) ||
				((std::fabs(lhs._val) == 180.0) &&
					(std::fabs(lhs._val) == std::fabs(rhs._val)));
		}
		friend inline bool operator!=(const Degree& lhs, const Degree& rhs)
		{
			return !(lhs == rhs);
		}
	};

	// radian definition
	using Radian = BoundedReal<
		_details::hiddenAnchor::_negativePI, 
		_details::hiddenAnchor::_positivePI, true>;

	// unsinged real definition
	using UnsignedReal = BoundedReal<
		_details::hiddenAnchor::_realZero, 
		_details::hiddenAnchor::_realMax>;
}

#ifdef EIGEN_WORLD_VERSION
namespace Eigen
{
	using VectorXR = ::Eigen::Matrix<HashColon::Helper::Real, -1, 1, 0, -1, 1>;
	using MatrixXR = ::Eigen::Matrix<HashColon::Helper::Real, -1, -1, 0, -1, -1>;
}
#endif

#endif