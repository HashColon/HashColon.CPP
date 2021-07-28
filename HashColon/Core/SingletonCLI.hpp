#ifndef HASHCOLON_CORE_SINGLETONCLI_HPP
#define HASHCOLON_CORE_SINGLETONCLI_HPP

#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include <exception>

#include <CLI11_modified/CLI11_extended.hpp>

namespace HashColon
{
	class SingletonCLI
	{
	private:
		static inline std::shared_ptr<SingletonCLI> _instance = nullptr;
		static inline std::once_flag _onlyOne;

		CLI::App cli;
		std::vector<std::string> _configfiles;

		SingletonCLI(size_t id) : cli("HASHCOLON") { };
		SingletonCLI(const SingletonCLI& rs){ _instance = rs._instance;	};
		SingletonCLI& operator=(const SingletonCLI& rs);
		CLI::App* GetCLI_core(CLI::App* app, const std::string iClassname);		

	public:
		enum ConfigurationFileType { json };
		~SingletonCLI() {};

		static SingletonCLI& GetInstance(size_t id = 0);
		static SingletonCLI& Initialize(
			ConfigurationFileType configtype = ConfigurationFileType::json);

		SingletonCLI& AddConfigFile(std::string configFileName);
		SingletonCLI& AddConfigFile(std::vector<std::string> configFileNames);

		std::vector<std::string>& GetConfigFileList();

		void Parse(int argc, char** argv,
			std::vector<std::string> configFiles);

		CLI::App* GetCLI(const std::string iClassname = "");
	};
}

#endif 
