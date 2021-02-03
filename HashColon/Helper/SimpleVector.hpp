// ***********************************************************************
// Assembly         : HASHCOLON.Helper
// Author           : Wonchul Yoo
// Created          : 01-22-2018
//
// Last Modified By : Wonchul Yoo
// Last Modified On : 01-17-2018
// ***********************************************************************
// <copyright file="SimpleVector.hpp" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary>2-D Vector calculation library</summary>
// ***********************************************************************
#ifndef HASHCOLON_HELPER_SIMPLEVECTOR_HPP
#define HASHCOLON_HELPER_SIMPLEVECTOR_HPP

#include <array>
#include <cmath>

/// <summary>
/// The HASHCOLON namespace.
/// </summary>
namespace HashColon
{
	/// <summary>
	/// The Helper namespace.
	/// </summary>
	namespace Helper
	{
		/// <summary>
		/// Functions in this namespace deals with 2D vectors defined as arrays(ex: double[2])
		/// </summary>
		namespace Vec2D
		{
			/// <summary>
			/// 2D vector sum.
			/// </summary>
			/// <param name="a">a.</param>
			/// <param name="b">The b.</param>
			/// <returns>std.array&lt;_Ty, _Size&gt;.</returns>
			template<typename T>
			inline std::array<T, 2> plus(std::array<T, 2> a, std::array<T, 2> b)
			{
				std::array<T, 2> re;
				re[0] = a[0] + b[0]; re[1] = a[1] + b[1];
				return re;
			}

			/// <summary>
			/// 2D vector subtraction
			/// </summary>
			/// <param name="a">a.</param>
			/// <param name="b">The b.</param>
			/// <returns>std.array&lt;_Ty, _Size&gt;.</returns>
			template<typename T>
			inline std::array<T, 2> minus(std::array<T, 2> a, std::array<T, 2> b)
			{
				std::array<T, 2> re;
				re[0] = a[0] - b[0]; re[1] = a[1] - b[1];
				return re;
			}

			/// <summary>
			/// Inner product of 2D vector
			/// </summary>
			/// <param name="a">a.</param>
			/// <param name="b">The b.</param>
			/// <returns>T.</returns>
			template<typename T>
			inline T dot(std::array<T, 2> a, std::array<T, 2> b)
			{
				return a[0] * b[0] + a[1] * b[1];
			}

			/// <summary>
			/// Outer product of 2D vector (gives only the size)
			/// </summary>
			/// <param name="a">a.</param>
			/// <param name="b">The b.</param>
			/// <returns>T.</returns>
			template<typename T>
			inline T cross(std::array<T, 2> a, std::array<T, 2> b)
			{
				return a[0] * b[1] - a[1] * b[0];
			}

			/// <summary>
			/// Absolute value of a 2D vector
			/// </summary>
			/// <param name="a">a.</param>
			/// <returns>T.</returns>
			template<typename T>
			inline T abs(std::array<T, 2> a)
			{
				return std::sqrt(a[0] * a[0] + a[1] * a[1]);
			}

			/// <summary>
			/// Multiplication of a scalar and a vector.
			/// </summary>
			/// <param name="a">a.</param>
			/// <param name="b">The b.</param>
			/// <returns>std.array&lt;_Ty, _Size&gt;.</returns>
			template<typename T>
			inline std::array<T, 2> multiply(T a, std::array<T, 2> b)
			{
				std::array<T, 2> re;
				re[0] = a * b[0]; re[1] = a * b[1];
				return re;
			}
		}

		/// <summary>
		/// Functions in this namespace deals with 3D vectors defined as arrays(ex: double[3])
		/// </summary>
		namespace Vec3D
		{

		}
	}
}


#endif
