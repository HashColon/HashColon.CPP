#include <HashColon/Helper/CLI11_extended.hpp>


bool CLI::detail::lexical_cast(std::string input, bool& output)
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

CLI::Option* CLI::BooleanOption::add_option(
	CLI::App* cli,
	std::string name,
	bool &variable, ///< The variable to set
	std::string description) 
{

	CLI::callback_t fun = [&variable](CLI::results_t res) { return detail::lexical_cast(res[0], variable); };

	CLI::Option *opt = cli->add_option(name, fun, description, false);
	opt->type_name("BOOL");
	return opt;
}