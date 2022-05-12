// header file for this source file
#include <HashColon/SingletonCLI.hpp>
// std libraries
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
// HashColon libraries
#include <HashColon/CLI11.hpp>
#include <HashColon/CLI11_JsonSupport.hpp>

using namespace std;
namespace HashColon
{
	SingletonCLI &SingletonCLI::operator=(const SingletonCLI &rs)
	{
		if (this != &rs)
		{
			_instance = rs._instance;
		}
		return *this;
	}

	CLI::App *SingletonCLI::GetCLI_core(CLI::App *app, const string iClassname)
	{

		// iClassname is empty string, return parent
		if (iClassname.empty())
			return app;

		// get first part of iClassname
		stringstream sscn(iClassname);
		string name;
		CLI::App *parent;
		parent = app;
		CLI::App *child;

		do
		{
			std::getline(sscn, name, '.');
			try
			{
				child = parent->get_subcommand(name);
			}
			catch (const CLI::OptionNotFound &e)
			{
				parent->add_subcommand(name, "");
				child = parent->get_subcommand(name);
			}
			parent = child;
		} while (sscn.rdbuf()->in_avail());

		return parent;
	}

	SingletonCLI &SingletonCLI::GetInstance(size_t id)
	{
		call_once(
			SingletonCLI::_onlyOne,
			[](size_t idx)
			{
				SingletonCLI::_instance.reset(new SingletonCLI(idx));
			},
			id);

		return *SingletonCLI::_instance;
	}

	SingletonCLI &SingletonCLI::Initialize(
		ConfigurationFileType configtype, string appDescription)
	{
		// set app description
		_appDescription = appDescription;

		// Instatiate singleton
		GetInstance();

		// set config formatter by config file type
		switch (configtype)
		{
		case ConfigurationFileType::json:
			GetInstance().GetCLI()->config_formatter(std::make_shared<CLI::ConfigJson>());
			break;
		case ConfigurationFileType::toml:
		case ConfigurationFileType::ini:
		default:
			break;
		}

		return GetInstance();
	}

	SingletonCLI &SingletonCLI::AddConfigFile(string configFileName)
	{
		GetInstance()._configfiles.push_back(configFileName);
		return GetInstance();
	}

	SingletonCLI &SingletonCLI::AddConfigFile(vector<string> configFileName)
	{
		for (auto &a : configFileName)
			GetInstance()._configfiles.push_back(a);
		return GetInstance();
	}

	vector<string> &SingletonCLI::GetConfigFileList()
	{
		return GetInstance()._configfiles;
	}

	void SingletonCLI::Parse(int argc, char **argv, vector<string> configFiles = {})
	{
		// set number of config files
		// set config
		size_t configCnt = GetInstance().GetConfigFileList().size() + configFiles.size();
		GetInstance().GetCLI()->set_config("--config", "", "Read configuration files", true)->expected(configCnt, configCnt + 1)->check(CLI::ExistingFile);

		// allow config extras
		GetInstance().GetCLI()->allow_config_extras(true);

		stringstream ss;
		for (size_t i = 0; i < configFiles.size(); i++)
			ss << "--config " << configFiles[i] << " ";

		for (size_t i = 0; i < GetInstance()._configfiles.size(); i++)
			ss << "--config " << GetInstance()._configfiles[i] << " ";

		for (size_t i = 1; i < (size_t)argc; i++)
			ss << argv[i] << " ";

		GetInstance().GetCLI()->parse(ss.str());
	}

	CLI::App *SingletonCLI::GetCLI(const string iClassname)
	{
		return GetCLI_core(&cli, iClassname);
	}
}