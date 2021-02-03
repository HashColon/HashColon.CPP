#ifndef HASHCOLON_HELPER_SINGLETONCLI_HPP
#define HASHCOLON_HELPER_SINGLETONCLI_HPP

#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include <exception>

#include <HashColon/Helper/ext/CLI11/CLI11_extended.hpp>


namespace HashColon
{
	namespace Helper
	{
		class SingletonCLI
		{
		private:
			static std::shared_ptr<SingletonCLI> _instance;
			static std::once_flag _onlyOne;

			CLI::App cli;
			std::vector<std::string> _configfiles;

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

			CLI::App* GetCLI_core(CLI::App* app, const std::string iClassname)
			{

				// iClassname is empty string, return parent
				if (iClassname.empty())
					return app;

				// get first part of iClassname
				std::stringstream sscn(iClassname);
				std::string name;
				CLI::App* parent; parent = app;
				CLI::App* child; 

				do
				{
					std::getline(sscn, name, '.');					
					try
					{
						child = parent->get_subcommand(name);
					}
					catch (std::exception e)
					{
						parent->add_subcommand(name, "");
						child = parent->get_subcommand(name);
					}
					parent = child;											
				} while (sscn.rdbuf()->in_avail());

				return parent;
			}



		public:
			enum ConfigurationFileType { json };

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

			static SingletonCLI& Initialize(				
				ConfigurationFileType configtype = ConfigurationFileType::json);

			SingletonCLI& AddConfigFile(std::string configFileName);
			SingletonCLI& AddConfigFile(std::vector<std::string> configFileNames);

			std::vector<std::string>& GetConfigFileList();

			void Parse(int argc, char** argv, 
				std::vector<std::string> configFiles);

			CLI::App* GetCLI(const std::string iClassname = "")
			{
				return GetCLI_core(&cli, iClassname);
			}
		};


	}
}

#endif 
