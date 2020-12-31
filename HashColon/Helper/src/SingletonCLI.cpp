/* IMPORTANT */
// must instantiate as following in ONLY ONE CPP FILE
#include <HashColon/Helper/SingletonCLI.hpp>

namespace HASHCOLON
{
	namespace Helper
	{
		std::once_flag SingletonCLI::_onlyOne;
		std::shared_ptr<SingletonCLI> SingletonCLI::_instance = nullptr;
	}
}