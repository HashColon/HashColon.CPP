#ifndef HASHCOLON_HELPER_TYPEHELPER_HPP
#define HASHCOLON_HELPER_TYPEHELPER_HPP

#include <string>
#include <boost/type_index.hpp>
#include <HashColon/Helper/StringHelper.hpp>

namespace HashColon::Helper
{
	template <typename T>
	inline std::string ShortTypename()
	{
		return Split(boost::typeindex::type_id_with_cvr<T>.pretty_name()).back();
	}

	template <typename T>
	inline std::string LongTypename()
	{
		return boost::typeindex::type_id_with_cvr<T>.pretty_name();
	}

}

#endif	
