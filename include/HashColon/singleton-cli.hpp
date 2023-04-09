#ifndef _HG_HASHCOLON_SINGLETONCLI
#define _HG_HASHCOLON_SINGLETONCLI

#include <HashColon/header>

// std libraries
#include <exception>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
// modified external libraries
#include <CLI11/CLI11.hpp>
#include <CLI11/CLI11_JsonSupport.hpp>

namespace HashColon
{
    /// @brief Singleton class to support CLI11 everywhere.
    class SingletonCLI
    {
    private:
        /// @brief Singleton instance
        static inline std::shared_ptr<SingletonCLI> _instance = nullptr;

        /// @brief once_flag to secure only-one instantiation
        static inline std::once_flag _onlyOne;

        /// @brief Name of the app
        static inline std::string _appName = "";

        /// @brief Description of the app
        static inline std::string _appDescription = "";

        /// @brief Internal CLI11 object
        CLI::App cli;

        /// @brief A list of additional configuration files
        std::vector<std::string> _configFiles;

        /// @brief Constructor with thread_it
        /// @param id Thread ID. This should be unique among the threads
        SingletonCLI(size_t id) : cli(_appDescription, _appName){};

        /// @brief Copy constructor
        /// @param rs Copied object
        SingletonCLI(const SingletonCLI &rs) { _instance = rs._instance; };

        /// @brief Assignment operator
        /// @param rs Copied object
        /// @return
        SingletonCLI &operator=(const SingletonCLI &rs);

        /// @brief Returns internal CLI11 object of a given option group
        /// @details If the option group of the given name does not exist, this function creates the missing options groups.
        /// @param app root App
        /// @param iClassname name of the option group
        /// @return App of the option group
        CLI::App *GetCLI_core(CLI::App *app, const std::string iClassname);

    public:
        /// @brief types of the configuration files.
        enum class ConfigurationFileType
        {
            toml,
            ini
#ifdef HASHCOLON_CONFIGURATION_BY_JSON
            ,
            json
#endif
        };

        /// @brief destructor
        ~SingletonCLI(){};

        /// @brief Gets singleton instance.
        /// @param thread_id Unique id of the thread
        /// @return Singleton CLI object
        static SingletonCLI &GetInstance(size_t thread_id = 0);

        /**
         * @brief Initiated the SingletonCLI
         * @details
         *  - Set name/description of the app,
         *  - Instantiate the singleton
         *  - Set configuration file type
         * @param configType
         * @param appDescription
         * @param appName
         * @return
         */
        static SingletonCLI &Initialize(
            ConfigurationFileType configType = ConfigurationFileType::toml,
            std::string appDescription = "", std::string appName = "");

        /// @brief Add a configuration file
        /// @param configFileName name/path of the configuration file
        /// @return
        SingletonCLI &AddConfigFile(std::string configFileName);

        /// @brief Add a multiple configuration files
        /// @param configFileNames name/paths of the configuration files
        /// @return
        SingletonCLI &AddConfigFile(std::vector<std::string> configFileNames);

        /// @brief Get additional config file list
        /// @return
        const std::vector<std::string> &GetConfigFileList();

        void Parse(int argc, char **argv,
                   std::vector<std::string> configFiles);

        CLI::App *GetCLI(const std::string iClassname = "");
    };

    /// @brief Helper function to get environment variant name from config-namespace & variable name
    /// @param configNamespace config namespace
    /// @param varname variable name
    /// @return Environment variable style name in string
    /// @details Example: config.name => CONFIG_NAME, Log.logstream => LOG_LOGSTREAM
    std::string GetEnvName(std::string configNamespace, std::string varname);
}

#endif
