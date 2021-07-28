#ifndef HASHCOLON_HELPER_CLI11_CLI_EXTENDED_HPP
#define HASHCOLON_HELPER_CLI11_CLI_EXTENDED_HPP

#include <string>
#include <algorithm>
#include <variant>
#include <stack>
#include <vector>
#include <CLI11_modified/CLI11.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>

#include <iostream>

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
	private:
		rapidjson::Value _jsonStr(const std::string& s, rapidjson::Document::AllocatorType& alloc) const
		{
			rapidjson::Value re;
			//re.SetString(s, alloc);
			re.SetString(s.c_str(), (rapidjson::SizeType)s.length(), alloc);
			return re;
		}

		void _setValue(const std::string& s, const std::string& type,
			rapidjson::Value& v, rapidjson::Document::AllocatorType& alloc) const
		{
			using namespace std;
			namespace Json = rapidjson;
			if (type == "INT")
			{
				int64_t a;
				if (detail::lexical_cast(s, a))
					v.SetInt64(a);
			}
			else if (type == "UINT")
			{
				uint64_t a;
				if (detail::lexical_cast(s, a))
					v.SetUint64(a);
			}
			else if (type == "FLOAT")
			{
				double a;
				if (detail::lexical_cast(s, a))
					v.SetDouble(a);
			}
			else if (type == "BOOLEAN")
			{
				bool a;
				if (detail::lexical_cast(s, a))
					v.SetBool(a);
			}
			else
			{
				string a;
				if (detail::lexical_cast(s, a))
					v.SetString(a.c_str(), (rapidjson::SizeType)a.length(), alloc);
			}
		}

		void _setArray(const std::vector<std::string>& s, const std::string& type,
			rapidjson::Value& v, rapidjson::Document::AllocatorType& alloc) const
		{
			using namespace std;
			namespace Json = rapidjson;
			v.SetArray();
			for (size_t i = 0; i < s.size(); i++)
			{
				Json::Value j;
				_setValue(s[i], type, j, alloc);
				v.PushBack(j, alloc);
			}
		}

		void _to_config(const CLI::App* app, bool default_also,
			rapidjson::Value& v, rapidjson::Document::AllocatorType& alloc) const
		{
			using namespace std;
			namespace Json = rapidjson;

			for (const CLI::Option* opt : app->get_options({})) {

				// Only process option with a long-name and configurable
				if (!opt->get_lnames().empty() && opt->get_configurable()) {
					string name = opt->get_lnames()[0];
					stringstream ss(opt->get_type_name());
					string type;
					getline(ss, type, ':');
					Json::Value j;

					// Non-flags
					if (opt->get_type_size() != 0)
					{
						// if value (not array)
						if (opt->count() == 1)
						{
							_setValue(opt->results().at(0), type, j, alloc);
						}
						// if array
						else if (opt->count() > 1)
						{
							_setArray(opt->results(), type, j, alloc);
						}
						else if (default_also && !opt->get_default_str().empty())
						{
							_setValue(opt->get_default_str(), type, j, alloc);
						}
					}
					// Flag, one passed
					else if (opt->count() == 1) {
						j.SetBool(true);
					}
					// Flag, multiple passed
					else if (opt->count() > 1) {
						j.SetUint((rapidjson::SizeType)opt->count());
					}
					// Flag, not present
					else if (opt->count() == 0 && default_also) {
						j.SetBool(false);
					}
					v.AddMember(_jsonStr(name, alloc), j, alloc);
				}
			}

			for (const CLI::App* subcom : app->get_subcommands({}))
			{
				Json::Value subcomname = _jsonStr(subcom->get_name(), alloc);
				Json::Value subjson(Json::kObjectType);

				_to_config(subcom, default_also, subjson, alloc);
				v.AddMember(subcomname, subjson, alloc);
			}
		}

	public:
		std::string to_config(const CLI::App* app, bool default_also, bool, std::string) const override
		{
			using namespace std;
			namespace Json = rapidjson;
			Json::Document doc;
			Json::Document::AllocatorType& alloc = doc.GetAllocator();
			Json::Value& docv = doc;
			docv.SetObject();

			_to_config(app, default_also, docv, alloc);

			Json::StringBuffer buffer;
			Json::PrettyWriter<Json::StringBuffer> writer(buffer);
			doc.Accept(writer);
			return buffer.GetString();
		}

	private:
		bool GetStr_forConfigItem(const rapidjson::Value& j, std::string& re) const
		{
			using namespace std;
			namespace Json = rapidjson;
			assert(!j.IsObject());
			assert(!j.IsArray());

#define ELSE_IF_J_IS(type_name) \
				else if (j.Is##type_name ()) { \
					stringstream ss; \
					ss << j.Get##type_name (); re =  ss.str(); \
				}

			if (j.IsBool()) {
				re = j.IsTrue() ? "true" : "false";
			}
			ELSE_IF_J_IS(Int)
				ELSE_IF_J_IS(Int64)
				ELSE_IF_J_IS(Uint)
				ELSE_IF_J_IS(Uint64)
				ELSE_IF_J_IS(Float)
				ELSE_IF_J_IS(Double)
			else if (j.IsString())
				re = j.GetString();
			else
				return false;
			return true;
		}

		std::vector<CLI::ConfigItem>
			_from_config(const rapidjson::Value& j, std::string name = "", std::vector<std::string> prefix = {}) const
		{
			using namespace std;
			namespace Json = rapidjson;

			std::vector<CLI::ConfigItem> results;


			if (j.IsObject())
			{
				for (Json::Value::ConstMemberIterator item = j.MemberBegin();
					item != j.MemberEnd(); ++item)
				{
					auto copy_prefix = prefix;
					if (!name.empty())
						copy_prefix.push_back(name);

					auto sub_results = _from_config(item->value, item->name.GetString(), copy_prefix);
					results.insert(results.end(), sub_results.begin(), sub_results.end());
				}
			}
			else if (!name.empty()) {
				results.emplace_back();
				CLI::ConfigItem& res = results.back();
				res.name = name;
				res.parents = prefix;

				string re_str;
								
				if (j.IsArray())
				{
					for (auto& arrItem : j.GetArray())
					{
						string item_str;
						if (GetStr_forConfigItem(arrItem, item_str))
							res.inputs.push_back(item_str);
					}
				}
				else if (GetStr_forConfigItem(j, re_str))
					res.inputs = { re_str };
				else {
					throw CLI::ConversionError("Failed to convert " + name);
				}
			}
			else {
				throw CLI::ConversionError("You must make all top level values objects in json!");
			}

			return results;
		}

	public:
		std::vector<CLI::ConfigItem> from_config(std::istream& input) const override
		{
			using namespace std;
			namespace Json = rapidjson;

			Json::IStreamWrapper isw(input);
			Json::Document doc;
			doc.ParseStream(isw);
			assert(doc.IsObject());

			return _from_config(doc);
		}
	};

}

//namespace CLI::detail
//{
//	bool lexical_conversion(const std::vector<std::string>& strings, std::vector<bool>& output) 
//	{
//		bool retval = true;
//		output.clear(); output.reserve(strings.size());
//		for (const auto& elem : strings)
//		{
//			output.emplace_back(false);
//			try
//			{
//				output.back() = to_flag_value(elem) == 1 ? true : false;
//			}
//			catch (...)
//			{
//				retval = false;
//			}			
//		}
//		return retval;
//	}
//}

#endif