// ***********************************************************************
// Assembly         : HASHCOLON.Helper
// Author           : Wonchul Yoo
// Created          : 01-22-2018
//
// Last Modified By : Wonchul Yoo
// Last Modified On : 01-22-2018
// ***********************************************************************
// <copyright file="ClassPropertyForCpp.hpp" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary>Interface class to use C#/JAVA like properties. This code is from https://www.codeproject.com/Tips/640013/Cplusplus-Properties .</summary>
// ***********************************************************************
#ifndef HASHCOLON_HELPER_CLASSPROPERTYFORCPP_HPP
#define HASHCOLON_HELPER_CLASSPROPERTYFORCPP_HPP

#include <functional>

/// <summary>
/// The HASHCOLON namespace.
/// </summary>
namespace HASHCOLON
{
	/// <summary>
	/// The Helper namespace.
	/// </summary>
	namespace Helper
	{		
		/// <summary>
		/// Struct Property
		/// </summary>
		template <
			typename OwnerType_,
			typename ValueType_,
			ValueType_(OwnerType_::*getter_)(),
			void (OwnerType_::*setter_)(ValueType_)>
		struct Property
		{			
			/// <summary>
			/// Initializes a new instance of the <see cref="Property{OwnerType_, ValueType_, getter_, setter_}"/> struct.
			/// </summary>
			/// <param name="owner">The owner.</param>
			Property(OwnerType_* owner)
				: _owner(owner) { }
						
			/// <summary>
			/// Operator=s the specified value.
			/// </summary>
			/// <param name="value">The value.</param>
			/// <returns>const ValueType_ &.</returns>
			const ValueType_& operator=(const ValueType_& value)
			{
				(_owner->*setter_)(value);
				return value;
			}
						
			/// <summary>
			/// Operator=s the specified value.
			/// </summary>
			/// <param name="value">The value.</param>
			/// <returns>ValueType_ &&.</returns>
			ValueType_&& operator=(ValueType_&& value)
			{
				(_owner->*setter_)(std::forward< ValueType_ >(value));
				return std::move((_owner->*getter_)());
			}

			/// <summary>
			/// Implements the operator ValueType_ operator.
			/// </summary>
			/// <returns>The result of the operator.</returns>
			operator ValueType_()
			{
				return (_owner->*getter_)();
			}
						
			/// <summary>
			/// Initializes a new instance of the <see cref="Property{OwnerType_, ValueType_, getter_, setter_}"/> struct.
			/// </summary>
			/// <param name="">The .</param>
			Property(Property&&) = default;
			/// <summary>
			/// Initializes a new instance of the <see cref="Property{OwnerType_, ValueType_, getter_, setter_}"/> struct.
			/// </summary>
			/// <param name="">The .</param>
			Property(const Property&) = delete;
			/// <summary>
			/// Operator=s the specified .
			/// </summary>
			/// <param name="">The .</param>
			/// <returns>HASHCOLON.Helper.Property&lt;OwnerType_, ValueType_, getter_, setter_&gt; &.</returns>
			Property& operator=(const Property&) = delete;
			/// <summary>
			/// Operator=s the specified .
			/// </summary>
			/// <param name="">The .</param>
			/// <returns>HASHCOLON.Helper.Property&lt;OwnerType_, ValueType_, getter_, setter_&gt; &.</returns>
			Property& operator=(Property&&) = delete;

		private:			
			/// <summary>
			/// The owner
			/// </summary>
			OwnerType_* const _owner;
		};
	}
}

/*
* USAGE EXAMPLE:
*	struct A
*	{
*	private:
*		int _test;
*	public:
*		CREATE_PROPERTY( A, public, int, test,
*			{ return _test; },
*			{ _test = 3 * value; }
*		);
*	};
*
* !! IMPORTANT
* getBlock, setBlock are not lambda function!!!!
*/
#define CREATE_PROPERTY(className, accessSpec, valueType, propName, getBlock, setBlock) \
	private: valueType _auto_g_get##propName() getBlock \
	private: void _auto_g_set##propName(valueType value) setBlock \
	public: using _auto_g_propType##propName = \
	HASHCOLON::Helper::Property<className, valueType, \
	&className::_auto_g_get##propName, \
	&className::_auto_g_set##propName>; \
	accessSpec: _auto_g_propType##propName propName = _auto_g_propType##propName(this)

#endif // !HASHCOLON_HELPER_CLASSPROPERTYFORCPP_H



