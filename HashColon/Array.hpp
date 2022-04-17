#ifndef HASHCOLON_ARRAY
#define HASHCOLON_ARRAY

// std libraries
#include <array>
#include <cassert>
#include <cmath>

// multidimensional arrays
namespace HashColon
{
	template <class T, std::size_t I, std::size_t... J>
	struct ArrayBase
	{
		using Nested = typename ArrayBase<T, J...>::type;
		using type = std::array<Nested, I>;
	};

	template <class T, std::size_t I>
	struct ArrayBase<T, I>
	{
		using type = std::array<T, I>;
	};

	template <typename _Ty, std::size_t... Sizes>
	using array = typename ArrayBase<_Ty, Sizes...>::type;
}

// 2D vector operations
namespace HashColon::Vec2D
{
	/// <summary>
	/// 2D vector sum.
	/// </summary>
	/// <param name="a">a.</param>
	/// <param name="b">The b.</param>
	/// <returns>std.array&lt;_Ty, _Size&gt;.</returns>
	template <typename T>
	inline std::array<T, 2> plus(std::array<T, 2> a, std::array<T, 2> b)
	{
		std::array<T, 2> re;
		re[0] = a[0] + b[0];
		re[1] = a[1] + b[1];
		return re;
	}

	/// <summary>
	/// 2D vector subtraction
	/// </summary>
	/// <param name="a">a.</param>
	/// <param name="b">The b.</param>
	/// <returns>std.array&lt;_Ty, _Size&gt;.</returns>
	template <typename T>
	inline std::array<T, 2> minus(std::array<T, 2> a, std::array<T, 2> b)
	{
		std::array<T, 2> re;
		re[0] = a[0] - b[0];
		re[1] = a[1] - b[1];
		return re;
	}

	/// <summary>
	/// Inner product of 2D vector
	/// </summary>
	/// <param name="a">a.</param>
	/// <param name="b">The b.</param>
	/// <returns>T.</returns>
	template <typename T>
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
	template <typename T>
	inline T cross(std::array<T, 2> a, std::array<T, 2> b)
	{
		return a[0] * b[1] - a[1] * b[0];
	}
	template <typename T>
	inline bool equal(std::array<T, 2> a, std::array<T, 2> b)
	{
		return (a[0] == b[0]) && (a[1] == b[1]);
	}

	/// <summary>
	/// Absolute value of a 2D vector
	/// </summary>
	/// <param name="a">a.</param>
	/// <returns>T.</returns>
	template <typename T>
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
	template <typename T>
	inline std::array<T, 2> multiply(T a, std::array<T, 2> b)
	{
		std::array<T, 2> re;
		re[0] = a * b[0];
		re[1] = a * b[1];
		return re;
	}

	/// <summary>
	/// Check if the query point (queryP) is left or right side of given line(A->B)
	/// </summary>
	/// <param name="lineA">Start point of given line</param>
	/// <param name="lineB">End point of given line</param>
	/// <param name="queryP">Query point</param>
	/// <returns>Left: positive, Right: negative, Intersect: 0</returns>
	template <typename T>
	inline T RelativeSideOfLine(std::array<T, 2> A, std::array<T, 2> B, std::array<T, 2> P)
	{
		// A != B
		assert(!equal(A, B));
		return (B[0] - A[0]) * (P[1] - A[1]) - (B[1] - A[1]) * (P[0] - A[0]);
	}

	/// <summary>
	/// Check if the query point (queryP) is left or right side of given line(A->B)
	/// </summary>
	/// <param name="lineA">Start point of given line</param>
	/// <param name="lineB">End point of given line</param>
	/// <param name="queryP">Query point</param>
	/// <returns>Left: positive, Right: negative, Intersect: 0</returns>
	template <typename T>
	inline T PointToLineDistance(std::array<T, 2> A, std::array<T, 2> B, std::array<T, 2> P)
	{
		// A != B
		assert(!equal(A, B));
		return ((B[0] - A[0]) * (P[1] - A[1]) - (B[1] - A[1]) * (P[0] - A[0])) / abs(minus(A, B));
	}

	template <typename T>
	inline T PointToLineSegmentDistance(std::array<T, 2> A, std::array<T, 2> B, std::array<T, 2> P)
	{
		// if A == B: Dist(A, P)
		if (equal(A, B))
			return abs(minus(P, A));

		// Length of AB-projected line PA
		T projLength = ((B[0] - A[0]) * (P[0] - A[0]) + (B[1] - A[1]) * (P[1] - A[1])) / abs(minus(B, A));
		// if projected(dist(PA)) < 0 : outside point A: dist(P, A)
		if (projLength < 0)
			return abs(minus(P, A));
		// if projected(dist(PA)) > 1 : outside point B: dist(P, B)
		else if (projLength > 1)
			return abs(minus(P, B));
		// else P btwn A,B
		else
			return std::abs(PointToLineDistance(A, B, P));
	}

}

#endif