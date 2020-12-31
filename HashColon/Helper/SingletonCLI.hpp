#ifndef HASHCOLON_HELPER_SINGLETONCLI_HPP
#define HASHCOLON_HELPER_SINGLETONCLI_HPP

#include <memory>
#include <mutex>
#include <vector>
#include <sstream>
#include <iostream>
#include <exception>

#include <HashColon/Helper/CLI11.hpp>


namespace HASHCOLON
{
	namespace Helper
	{
		class SingletonCLI
		{
		private:
			static std::shared_ptr<SingletonCLI> _instance;
			static std::once_flag _onlyOne;

			CLI::App cli; 

			SingletonCLI(int id) : cli("HASHCOLON@SNU") { };
			SingletonCLI(const SingletonCLI& rs)
			{
				_instance = rs._instance;
			};

			SingletonCLI& operator=(const SingletonCLI& rs)
			{
				if (this != &rs)
				{
					_instance = rs._instance;
				}
				return *this;
			}

			CLI::App* GetCLI_core(CLI::App* parent, const std::string iClassname)
			{
				// iClassname is empty string, return parent
				if (iClassname.empty())
					return parent;

				// get first part of iClassname
				std::stringstream sscn(iClassname);
				std::string first, leftovers;
				std::getline(sscn, first, '.');

				CLI::App* child;

				// if no matching class found,
				try
				{
					child = parent->get_subcommand(first);
				}
				catch(std::exception e)
				{
					parent->add_subcommand(first, "");
				}
				
				// get leftover string from sscn
				std::getline(sscn, leftovers);

				return GetCLI_core(parent->get_subcommand(first), leftovers);
			}



		public:
			~SingletonCLI() {};

			static SingletonCLI& GetInstance(int id = 0)
			{
				std::call_once(SingletonCLI::_onlyOne,
					[](int idx)
				{
					SingletonCLI::_instance.reset(new SingletonCLI(idx));
				}, id);
				return *SingletonCLI::_instance;
			}

			CLI::App* GetCLI(const std::string iClassname = "")
			{	
				return GetCLI_core(&cli, iClassname);
			}
			
		};


	}
}

/* IMPORTANT */
// must instantiate as following in ONLY ONE CPP FILE
//namespace HASHCOLON
//{
//	namespace Helper
//	{
//		std::once_flag IniReader_CLI::_onlyOne;
//		std::shared_ptr<IniReader_CLI> IniReader_CLI::_instance = nullptr;
//	}
//}

#endif 
