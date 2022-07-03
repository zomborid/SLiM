#pragma once

#include <vector>

namespace slim {

	struct parsed_interface {

		using identifier = std::string;

		using interface_name = identifier;

		using type_t = std::string;

		struct parameter {
			type_t type;
			identifier name;
		};

		using parameter_list = std::vector<parameter>;

		struct function {
			type_t return_type;
			identifier name;
			parameter_list parameters;
		};

		using signal_block = std::vector<function>;
		using command_block = std::vector<function>;

		interface_name name;
		signal_block signals;
		command_block commands;
	};

	parsed_interface parse_interface(std::string str);
}