#ifndef HASHCOLON_HELPER_CLI11_CLI_EXTENDED_HPP
#define HASHCOLON_HELPER_CLI11_CLI_EXTENDED_HPP

#include <string>
#include <algorithm>
#include <variant>
#include <HashColon/Helper/ext/CLI11/CLI11.hpp>
#include <rapidjson/document.h>

#include "nlohmann/json.hpp"

// Enables boolean option for CLI11: DEPRECATED for ver1.9.1?
/*namespace CLI::detail
{
	bool lexical_cast(std::string input, bool& output)
	{
		std::transform(input.begin(), input.end(), input.end(), ::tolower);
		if ((input == "true") || (input == "on") || (input == "yes") || (input == "1"))
		{
			output = true; return true;
		}
		else if ((input == "false") || (input == "off") || (input == "no") || (input == "0"))
		{
			output = false; return true;
		}
		else
		{
			return false;
		}
	}
}

namespace CLI::BooleanOption
{
	Option* add_option(
		App* cli,
		std::string name,
		bool& variable, ///< The variable to set
		std::string description = "")
	{
		CLI::callback_t fun = [&variable](CLI::results_t res) { return detail::lexical_cast(res[0], variable); };
		CLI::Option* opt = cli->add_option(name, fun, description, false);
		opt->type_name("BOOL");
		return opt;
	}
}*/

namespace CLI
{
	class ConfigJson : public Config
	{	
    public:
        std::string to_config(const CLI::App* app, bool default_also, bool, std::string) const override {

            nlohmann::json j;

            for (const CLI::Option* opt : app->get_options({})) {

                // Only process option with a long-name and configurable
                if (!opt->get_lnames().empty() && opt->get_configurable()) {
                    std::string name = opt->get_lnames()[0];

                    // Non-flags
                    if (opt->get_type_size() != 0) {

                        // If the option was found on command line
                        if (opt->count() == 1)
                            j[name] = opt->results().at(0);
                        else if (opt->count() > 1)
                            j[name] = opt->results();

                        // If the option has a default and is requested by optional argument
                        else if (default_also && !opt->get_default_str().empty())
                            j[name] = opt->get_default_str();

                        // Flag, one passed
                    }
                    else if (opt->count() == 1) {
                        j[name] = true;

                        // Flag, multiple passed
                    }
                    else if (opt->count() > 1) {
                        j[name] = opt->count();

                        // Flag, not present
                    }
                    else if (opt->count() == 0 && default_also) {
                        j[name] = false;
                    }
                }
            }

            for (const CLI::App* subcom : app->get_subcommands({}))
                j[subcom->get_name()] = nlohmann::json(to_config(subcom, default_also, false, ""));

            return j.dump(4);
        }

        std::vector<CLI::ConfigItem> from_config(std::istream& input) const override {
            nlohmann::json j;
            input >> j;
            return _from_config(j);
        }

        std::vector<CLI::ConfigItem>
            _from_config(nlohmann::json j, std::string name = "", std::vector<std::string> prefix = {}) const {
            std::vector<CLI::ConfigItem> results;

            if (j.is_object()) {
                for (nlohmann::json::iterator item = j.begin(); item != j.end(); ++item) {
                    auto copy_prefix = prefix;
                    if (!name.empty())
                        copy_prefix.push_back(name);
                    auto sub_results = _from_config(*item, item.key(), copy_prefix);
                    results.insert(results.end(), sub_results.begin(), sub_results.end());
                }
            }
            else if (!name.empty()) {
                results.emplace_back();
                CLI::ConfigItem& res = results.back();
                res.name = name;
                res.parents = prefix;
                if (j.is_boolean()) {
                    res.inputs = { j.get<bool>() ? "true" : "false" };
                }
                else if (j.is_number()) {
                    std::stringstream ss;
                    ss << j.get<double>();
                    res.inputs = { ss.str() };
                }
                else if (j.is_string()) {
                    res.inputs = { j.get<std::string>() };
                }
                else if (j.is_array()) {
                    for (std::string ival : j)
                        res.inputs.push_back(ival);
                }
                else {
                    throw CLI::ConversionError("Failed to convert " + name);
                }
            }
            else {
                throw CLI::ConversionError("You must make all top level values objects in json!");
            }

            return results;
        }
	};

}

#endif