#pragma once
#ifndef HASHCOLON_HELPER_CLI11_JSONFORMATTER_HPP
#define HASHCOLON_HELPER_CLI11_JSONFORMATTER_HPP

#include <string>
#include <HashColon/Helper/CLI11.hpp>
#include <HashColon/Helper/Json.hpp>


namespace CLI
{
	class ConfigJson : public Config
	{
	public:
		std::string to_config(const CLI::App *app, bool default_also, bool, std::string) const override {

			HASHCOLON::Helper::Json j;

			for (const CLI::Option *opt : app->get_options({})) 
			{
				// Only process option with a long-name and configurable
				if (!opt->get_lnames().empty() && opt->get_configurable()) 
				{
					std::string name = opt->get_lnames()[0];

					// Non-flags
					if (opt->get_type_size() != 0) 
					{
						// If the option was found on command line
						if (opt->count() == 1)
						{
							j[name] = opt->results().at(0);							
						}							
						else if (opt->count() > 1)
						{
							j[name] = opt->results();
						}
						// If the option has a default and is requested by optional argument
						else if (default_also && !opt->get_defaultval().empty())
						{
							j[name] = opt->get_defaultval();
						}							
					}
					// Flag, one passed
					else if (opt->count() == 1) 
					{
						j[name] = true;						
					}
					// Flag, multiple passed
					else if (opt->count() > 1) 
					{
						j[name] = opt->count();						
					}
					// Flag, not present
					else if (opt->count() == 0 && default_also) 
					{
						j[name] = false;
					}
				}
			}

			for (const CLI::App *subcom : app->get_subcommands({}))
			{
				j[subcom->get_name()] = HASHCOLON::Helper::Json(to_config(subcom, default_also, false, ""));
			}

			return j.dump();
		}
	private:
		std::string conversion_error_string (
			std::string name, std::vector<std::string> prefixs) const
		{
			std::string prefixStr = "";
			for (std::string& aPrefix : prefixs)
				prefixStr += aPrefix;

			return "Failed to convert " + prefixStr + '.' + name;
		}

	public:
		std::vector<CLI::ConfigItem> from_config(std::istream &input) const override {
			HASHCOLON::Helper::Json j;
			input >> j;
			return _from_config(j);
		}

		std::vector<CLI::ConfigItem>
			_from_config(HASHCOLON::Helper::Json j, std::string name = "", std::vector<std::string> prefix = {}) const {
			std::vector<CLI::ConfigItem> results;

			if (j.is_object()) {
				for (HASHCOLON::Helper::Json::iterator item = j.begin(); item != j.end(); ++item) {
					auto copy_prefix = prefix;
					if (!name.empty())
						copy_prefix.push_back(name);
					auto sub_results = _from_config(*item, item.key(), copy_prefix);
					results.insert(results.end(), sub_results.begin(), sub_results.end());
				}
			}
			else if (!name.empty()) {
				results.emplace_back();
				CLI::ConfigItem &res = results.back();
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
					for (auto& ival : j)
					{
						if (ival.is_boolean())
						{
							res.inputs.push_back(ival.get<bool>() ? "true" : "false");
						}
						else if (ival.is_number())
						{
							std::stringstream ss;
							ss << ival.get<double>();
							res.inputs.push_back(ss.str());
						}
						else if (ival.is_string())
						{
							res.inputs.push_back(ival);
						}
						else
						{
							throw CLI::ConversionError(
								conversion_error_string(name, prefix) + "\nArrays/objects are not allowed in arrays for configuration scheme.");
						}
					}
						
				}
				else {
					throw CLI::ConversionError(conversion_error_string(name, prefix));
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