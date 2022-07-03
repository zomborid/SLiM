#include "configuration_parser.h"

#include "remove_comments.h"
#include "../SLiM/error.h"

#include <string>
#include <iostream>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/adapted/struct.hpp>
//#include <boost/tuple/tuple.hpp>
//#include <boost/spirit/home/x3/support/ast/variant.hpp>

namespace std {
	template<typename T> static inline std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
		out << "{";
		std::size_t last = v.size() - 1;
		for (std::size_t i = 0; i < v.size(); ++i) {
			out << v[i];
			if (i != last)
				out << ", ";
		}
		out << "}";
		return out;
	}

	
}

namespace x3 = boost::spirit::x3;

namespace slim {
	namespace ast {

		using identifier = std::string;
		using type_t = std::string;
		using template_value = std::string;

		using configuration_name = identifier;
		struct template_parameter {
			identifier name;
			template_value value;
		};


		using template_parameters = std::vector<template_parameter>;
		using template_parameters_no_value = std::vector<identifier>;

		struct component_declaration {
			boost::optional<std::string> instantiation;
			identifier type;
			template_parameters parameters;
			boost::optional<identifier> name;

			operator parsed_configuration::component_declaration() {
				auto res = parsed_configuration::component_declaration();
				res.instatiated = (bool)instantiation;
				res.type = type;

				auto parameter_conversion = [](template_parameter p) {
					return std::make_pair(p.name, p.value);
				};

				for (auto param : parameters) {
					if (!res.parameters.emplace(param.name, param.value).second) {
						throw trace("Error: duplicate component parameter \"" + param.name + "\"");
					}
				}

				std::transform(parameters.begin(), parameters.end(), std::inserter(res.parameters, res.parameters.end()), parameter_conversion);

				res.name = name ? name.get() : type;
				return res;
			}
		};

		using interface_declaration = std::vector<identifier>;

		using use_block = std::vector<interface_declaration>;
		using components_block = std::vector<component_declaration>;
		using provide_block = std::vector<interface_declaration>;

		using interface_statement = std::vector<identifier>;

		struct routing_statement {
			interface_statement user;
			interface_statement provider;

			operator parsed_configuration::routing_statement() {

				parsed_configuration::interface_statement u;
				parsed_configuration::interface_statement p;

				u.external = user.size() == 1;
				u.component = u.external ? "" : user[0];
				u.iface = u.external ? user[0] : user[1];

				p.external = provider.size() == 1;
				p.component = p.external ? "" : provider[0];
				p.iface = p.external ? provider[0] : provider[1];

				return parsed_configuration::routing_statement{ u, p };
			}
		};

		using routing_block = std::vector<routing_statement>;

		struct configuration {
			configuration_name name;
			template_parameters_no_value parameters;
			use_block use;
			components_block components;
			provide_block provide;
			routing_block routing;
		};

		parsed_configuration convert(configuration data) {
			parsed_configuration config;

			auto iface_conversion = [](interface_statement v) {
				auto c = parsed_configuration::interface_declaration{ v[0], v.size() > 1 ? v[1] : v[0] };
				return std::make_pair(c.name, c);
			};

			auto component_conversion = [](component_declaration v) {
				auto c = (parsed_configuration::component_declaration) v;
				return std::make_pair(c.name, c);
			};

			config.name = data.name;
			config.parameters = data.parameters;
			std::transform(data.use.begin(), data.use.end(), std::inserter(config.use, config.use.end()), iface_conversion);
			std::transform(data.components.begin(), data.components.end(), std::inserter(config.components, config.components.end()), component_conversion);
			std::transform(data.provide.begin(), data.provide.end(), std::inserter(config.provide, config.provide.end()), iface_conversion);
			config.routing = parsed_configuration::routing_block(data.routing.begin(), data.routing.end());

			return config;
		}
	}
}

namespace ast = slim::ast;

BOOST_FUSION_ADAPT_STRUCT(
	ast::template_parameter,
	(ast::identifier, name)
	(ast::template_value, value)
)

BOOST_FUSION_ADAPT_STRUCT(
	ast::component_declaration,
	(boost::optional<std::string>, instantiation)
	(ast::identifier, type)
	(ast::template_parameters, parameters)
	(boost::optional<ast::identifier>, name)
)

BOOST_FUSION_ADAPT_STRUCT(
	ast::routing_statement,
	(ast::interface_statement, user)
	(ast::interface_statement, provider)
)

BOOST_FUSION_ADAPT_STRUCT(
	ast::configuration,
	(ast::configuration_name, name)
	(ast::template_parameters_no_value, parameters)
	(ast::use_block, use)
	(ast::components_block, components)
	(ast::provide_block, provide)
	(ast::routing_block, routing)
)

namespace std {

	static inline std::ostream& operator <<(std::ostream& os, ast::template_parameter const& p) {
		//return os << std::get<0>(p) << "=" << std::get<1>(p);
		return os << p.name << "=" << p.value;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::template_parameters const& parameters) {

		if (parameters.size() > 0) {
			os << "<";
			os << parameters[0];
			for (int c = 1; c < parameters.size(); c++) {
				os << ", " << parameters[c];
			}
			os << ">";
		}

		return os;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::component_declaration const& i) {
		os << (i.instantiation ? "new " : "") << i.type;
		os << i.parameters;
		os << (i.name ? " as " + i.name.get() : "");
		return os;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::interface_statement const& is) {
		if (is.size() == 2) {
			return os << is[0] << "." << is[1];
		} else {
			return os << is[0];
		}
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::routing_statement const& rs) {
		return os << rs.user << " -> " << rs.provider;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::configuration const& c) {
		os << c.name;
		
		if (c.parameters.size() > 0) {
			os << "<";
			os << c.parameters[0];
			for (int i = 1; i < c.parameters.size(); i++) {
				os << ", " << c.parameters[i];
			}
			os << ">";
		}

		os << ":\nuse: " << c.use
			<< "\ncomponents" << c.components
			<< "\nprovide" << c.provide
			<< "\nrouting" << c.routing
			;
	}
}

namespace slim {

	namespace configuration_grammar {

		struct configuration_class {
			template <typename Iterator, typename Exception, typename Context>
			x3::error_handler_result on_error(Iterator& first, Iterator const& last, Exception const& x, Context const& context) {

				std::string message = "Error: expecting " + x.which() + " here:";
				std::cout << message << "\n";

				std::stringstream ss;

				auto curr = first;
				auto iter = x.where();

				int row = 1;
				int prefix_size = 0;
				while (curr != iter) {
					ss << (*curr == '\t' ? ' ' : *curr);
					++prefix_size;
					if (*curr == '\r' || *curr == '\n') {
						prefix_size = 0;
						ss.str("");
						++row;
					}
					++curr;
				}

				int char_count = 0;
				for (; char_count < 10; ++char_count) {
					if (iter == last || *iter == '\r' || *iter == '\n') break;
					ss << *iter;
					++iter;
				}
				ss << "\n";
				for (; prefix_size > 0; --prefix_size) {
					ss << '-';
				}

				ss << "^";
				for (; char_count > 1; --char_count) {
					ss << '-';
				}
				ss << "\n";

				ss << "In row: " << row << "\n";

				std::cerr << ss.str();

				return x3::error_handler_result::fail;
			}
		};

		x3::rule<configuration_class, ast::configuration> configuration = "configuration";

		auto ws_opt = x3::omit[*x3::space];
		auto ws_req = x3::omit[+x3::space];

		auto identifier_start = x3::alpha | x3::char_('_');
		auto identifier_body = x3::alnum | x3::char_('_');
		auto identifier
			= x3::rule<struct identifier_class, std::string>{}
			= identifier_start > *identifier_body;

		auto number
			= x3::rule<struct integer_class, std::string>{}
			= -(x3::char_('-') | x3::char_('+')) >> x3::digit > *(x3::alnum | '.');

		static x3::rule<struct type_class, ast::type_t> type{ "type" };
		
		auto templated_type // % operator can not be used if from a % b b has to be kept!
			= identifier >> -(ws_opt >> x3::char_('<') >> ((ws_opt >> (type | number)) > *(ws_opt >> x3::char_(',') > (ws_opt > (type | number)))) > ws_opt > x3::char_('>'));

		auto pointers
			= x3::rule<struct pointers_class, std::string>{}
			= +(x3::char_("*&") > ws_opt);

		auto scope_separator // prevents omitting
			= x3::rule<struct scope_separator_class, std::string>{}
			= x3::lit("::");

		auto type_def
			= -(scope_separator > ws_opt) >> templated_type >> *(ws_opt >> scope_separator > ws_opt > templated_type) >> -(ws_opt >> pointers);

		BOOST_SPIRIT_DEFINE(type);

		auto template_value
			= x3::rule<struct template_value_class, std::string>{}
			= (type | number);

		auto template_parameter
			= x3::rule<struct template_parameter_class, ast::template_parameter>{}
			= identifier > ws_opt > '=' > ws_opt > template_value;

		auto template_parameters
			= x3::rule<struct template_parameters_class, ast::template_parameters>{}
			= template_parameter > *(ws_opt >> ',' > ws_opt > template_parameter);

		auto template_parameters_no_value
			= x3::rule<struct template_parameters_no_value_class, ast::template_parameters_no_value>{}
			= identifier > *(ws_opt >> ',' > ws_opt > identifier);

		auto instantiation
			= x3::rule<struct instantiation_class, std::string>{}
			= x3::lit("new");

		auto component_declaration
			= x3::rule<struct component_declaration_class, ast::component_declaration>{}
			= -(instantiation > ws_req)
			> identifier
			> -(x3::lit("<") >> ws_opt >> template_parameters > ws_opt > '>')
			> -(ws_req >> x3::lit("as") > ws_req > identifier);

		auto interface_declaration
			= x3::rule<struct resource_declaration_class, ast::interface_declaration>{}
			= identifier > -(ws_req >> x3::lit("as") > ws_req > identifier);


		auto configuration_name
			= x3::lit("configuration") > ws_req > identifier;

		auto configuration_parameters
			= -(x3::lit("<") > ws_opt > template_parameters_no_value > ws_opt > '>' > ws_opt) > ';';

		auto use_block
			= x3::rule<struct use_block_class, ast::use_block>{}
			= x3::lit("use") > ws_opt > '('
			> *(ws_opt >> interface_declaration > ws_opt > (',' | &(ws_opt > ')')))
			> ws_opt > ')';

		auto components_block
			= x3::rule<struct components_block_class, ast::components_block>{}
			= x3::lit("components") > ws_opt > '(' > ws_opt
			> -(component_declaration > *(ws_opt >> ',' > ws_opt > component_declaration))
			> ws_opt > ')';
		/*
		auto components_block
			= x3::rule<struct components_block_class, ast::components_block>{}
			= x3::lit("components") > ws_opt > '('
			> *(ws_opt >> component_declaration > ws_opt > (',' | &(x3::lit(")"))))
			> ws_opt > ')';*/

		auto provide_block
			= x3::rule<struct provide_block_class, ast::provide_block>{}
			= x3::lit("provide") > ws_opt > '('
			> *(ws_opt >> interface_declaration > ws_opt > (',' | &(ws_opt > ')')))
			//	>> *(ws_opt >> interface_declaration >> ws_opt >> ',')	// alternative for the lookahead based solution
			//	>> -(ws_opt >> interface_declaration)	
			> ws_opt > ')';



		auto interface_statement
			= x3::rule<struct interface_statement_class, ast::interface_statement>{}
			= identifier > -('.' > identifier);

		auto routing_statement
			= x3::rule<struct routing_statement_class, ast::routing_statement>{}
			= interface_statement > ws_opt > x3::lit("->") > ws_opt > interface_statement > ws_opt > ';';

		auto routing_block
			= x3::rule<struct routing_block_class, ast::routing_block>{}
			= x3::lit("routing") > ws_opt > '{'
			> ws_opt > *(routing_statement > ws_opt)
			> '}';




		auto configuration_def
			= ws_opt > configuration_name
			> ws_opt > configuration_parameters
			> ws_opt > use_block
			> ws_opt > components_block
			> ws_opt > provide_block
			> ws_opt > routing_block
			> ws_opt;

		BOOST_SPIRIT_DEFINE(configuration);
	}

	auto grammar = configuration_grammar::configuration;

	parsed_configuration parse_configuration(std::string str) {
		str = remove_comments(str);

		auto f = str.cbegin(), l = str.cend();

		ast::configuration data;

		if (parse(f, l, slim::grammar, data)) {

			return ast::convert(data);

		} else {
			throw trace("Error: configuration parse failed");
		}

	}
}
