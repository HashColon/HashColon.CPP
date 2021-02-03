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

	SingletonCLI& SingletonCLI::AddConfigFile(string configFileName)
	{
		GetInstance()._configfiles.push_back(configFileName);
		return GetInstance();
	}

	SingletonCLI& SingletonCLI::AddConfigFile(vector<string> configFileName)
	{
		for (auto& a : configFileName)
			GetInstance()._configfiles.push_back(a);
		return GetInstance();
	}

	vector<string>& SingletonCLI::GetConfigFileList()
	{
		return GetInstance()._configfiles;
	}


	void SingletonCLI::Parse(int argc, char** argv, vector<string> configFiles)
	{
		// set number of config files
		// set config
		GetInstance().GetCLI()
			->set_config("--config", "", "Read configuration json files", true)
			->expected(
				GetInstance().GetConfigFileList().size()
			+ configFiles.size())->check(CLI::ExistingFile);

		// allow config extras
		GetInstance().GetCLI()->allow_config_extras(true);

		stringstream ss;
		for (size_t i = 0; i < configFiles.size(); i++)
			ss << "--config " << configFiles[i] << " ";

		for (size_t i = 0; i < GetInstance()._configfiles.size(); i++)
			ss << "--config " << GetInstance()._configfiles[i] << " ";

		for (size_t i = 1; i < argc; i++)
			ss << argv[i] << " ";

		GetInstance().GetCLI()->parse(ss.str());
	}

	once_flag SingletonCLI::_onlyOne;
	shared_ptr<SingletonCLI> SingletonCLI::_instance = nullptr;
}