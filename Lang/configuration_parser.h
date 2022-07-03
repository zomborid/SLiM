#pragma once

#include <vector>
#include <map>

namespace slim {

	struct parsed_configuration {

		using identifier = std::string;
		using value = std::string;

		using configuration_name = identifier;
		using template_parameters = std::map<identifier, value>;

		struct component_declaration {
			bool instatiated;
			identifier type;
			identifier name;
			template_parameters parameters;

			identifier identifier() {
				return instatiated ? name : type;
			}
		};

		struct interface_declaration {
			identifier type;
			identifier name;
		};

		using use_block = std::map<identifier, interface_declaration>;
		using components_block = std::map<identifier, component_declaration>;
		using provide_block = std::map<identifier, interface_declaration>;

		struct interface_statement {
			bool external;
			identifier component;
			identifier iface;
		};

		struct routing_statement {
			interface_statement user;
			interface_statement provider;
		};

		using routing_block = std::vector<routing_statement>;

		configuration_name name;
		std::vector<identifier> parameters;
		use_block use;
		components_block components;
		provide_block provide;
		routing_block routing;
	};

	parsed_configuration parse_configuration(std::string str);
}