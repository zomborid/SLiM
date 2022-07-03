#pragma once

#include <vector>
#include <map>

namespace slim {

	struct parsed_module {

		using identifier = std::string;

		using module_name = identifier;

		using type_t = std::string;

		struct parameter {
			type_t type;
			identifier name;
		};

		using parameter_list = std::vector<parameter>;

		struct function {
			type_t return_type;
			identifier iface;
			identifier name;
			parameter_list parameters;
		};

		struct interface_declaration {
			identifier type;
			identifier name;
		};

		using functions = std::map<identifier, function>;

		using used_interfaces = std::map<identifier, interface_declaration>;
		using provided_interfaces = std::map<identifier, interface_declaration>;
		using signal_implementations = std::map<identifier, functions>;
		using command_implementations = std::map<identifier, functions>;
		using task_definitions = std::vector<identifier>;
		using parameter_declarations = std::vector<identifier>;

		module_name name;
		used_interfaces use;
		provided_interfaces provide;
		signal_implementations signals;
		command_implementations commands;
		task_definitions tasks;
		parameter_declarations parameters;
	};

	parsed_module parse_module(std::string str);
}