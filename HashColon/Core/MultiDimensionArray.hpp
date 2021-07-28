#ifndef HASHCOLON_CORE_MULTIDIMENSIONARRAY_HPP
#define HASHCOLON_CORE_MULTIDIMENSIONARRAY_HPP

#include <array>

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

#endif