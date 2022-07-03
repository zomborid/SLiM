#pragma once

#include <vector>
#include <map>

namespace slim {

	struct parsed_accesspoint {

		using identifier = std::string;
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

		using functions_t = std::map<identifier, function>;

		functions_t functions;
	};

	parsed_accesspoint parse_accesspoint(std::string str);
}