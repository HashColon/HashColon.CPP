#pragma once
#ifndef HASHCOLON_HELPER_CLI_EXTENDED_HPP
#define HASHCOLON_HELPER_CLI_EXTENDED_HPP

#include <string>
#include <algorithm>
#include <HashColon/Helper/CLI11.hpp>

namespace CLI::detail
{
	bool lexical_cast(std::string input, bool& output);	
}

namespace CLI::BooleanOption
{
	Option *add_option(
		App* cli,
		std::string name,
		bool &variable, ///< The variable to set
		std::string description = "");
}

#endif