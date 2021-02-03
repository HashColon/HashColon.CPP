/* IMPORTANT */
// must instantiate as following in ONLY ONE CPP FILE
#include <filesystem>
#include <sstream>
#include <HashColon/Helper/SingletonCLI.hpp>

using namespace std;
namespace HashColon::Helper
{
	SingletonCLI& SingletonCLI::Initialize(
		ConfigurationFileType configtype)
	{
		// Instatiate singleton
		GetInstance();

		// set config formatter by config file type
		switch (configtype)
		{
		case ConfigurationFileType::json:
			GetInstance().GetCLI()->config_formatter(std::make_shared<CLI::ConfigJson>());
			break;
		default:
			break;
		}
		return GetInstance();
	}

	void SingletonCLI::Parse(int argc, char** argv, vector<string> configFiles)
	{
		stringstream ss;
		for (size_t i = 1; i < argc; i++)
			ss << argv[i] << " ";

		for (size_t i = 0; i < configFiles.size(); i++)
			ss << "--config " << configFiles[i] << " ";

		// set config
		GetInstance().GetCLI()
			->set_config("--config", "", "Read configuration json files", true)
			->expected(configFiles.size())->check(CLI::ExistingFile);

		// allow config extras
		GetInstance().GetCLI()->allow_config_extras(true);

		// parse
		GetInstance().GetCLI()->parse(ss.str());		
	}

	once_flag SingletonCLI::_onlyOne;
	shared_ptr<SingletonCLI> SingletonCLI::_instance = nullptr;
}