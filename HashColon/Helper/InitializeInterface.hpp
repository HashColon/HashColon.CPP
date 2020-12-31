#ifndef HASHCOLON_HELPER_INITIALIZEINTERFACE_HPP
#define HASHCOLON_HELPER_INITIALIZEINTERFACE_HPP

#include <mutex>

namespace HASHCOLON
{
	namespace Helper
	{
		template <typename _targetClass, typename _paramsType>
		class InitializeInterface
		{
		public:
			void Initialize()
			{
				std::call_once(
					_initialized,
					_targetClass::Initialize_once()
				);
			}


		protected:
			static _paramsType _cDefault;
			static std::once_flag _initialized;
		};

		template <typename _targetClass, typename _paramsType>
		_paramsType InitializeInterface<_targetClass, _paramsType>::_cDefault;

		template <typename _targetClass, typename _paramsType>
		std::once_flag InitializeInterface<_targetClass, _paramsType>::_initialized;
	}
}

#endif
