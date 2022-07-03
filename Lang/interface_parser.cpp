#include "interface_parser.h"

#include "remove_comments.h"
#include "../SLiM/error.h"

#include <string>
#include <iostream>

//#define BOOST_SPIRIT_X3_DEBUG

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/adapted/struct.hpp>

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

		using identifier = parsed_interface::identifier;

		using interface_name = parsed_interface::interface_name;

		using type_t = parsed_interface::type_t;
		using parameter = parsed_interface::parameter;

		using parameter_list = parsed_interface::parameter_list;

		using function = parsed_interface::function;

		using signal_block = parsed_interface::signal_block;
		using command_block = parsed_interface::command_block;

		using parsed_interface = parsed_interface;
	}
}

namespace ast = slim::ast;

BOOST_FUSION_ADAPT_STRUCT(
	ast::parameter,
	(ast::type_t, type)
	(ast::identifier, name)
)

BOOST_FUSION_ADAPT_STRUCT(
	ast::function,
	(ast::type_t, return_type)
	(ast::identifier, name)
	(ast::parameter_list, parameters)
)

BOOST_FUSION_ADAPT_STRUCT(
	ast::parsed_interface,
	(ast::interface_name, name)
	(ast::command_block, commands)
	(ast::signal_block, signals)
)

namespace std {
	static inline std::ostream& operator <<(std::ostream& os, ast::parameter const& p) {
		return os << p.type << "|" << p.name;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::function const& f) {
		return os << "\n" << f.return_type << " " << f.name << "(" << f.parameters << ")" << std::endl;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::parsed_interface const& pi) {
		return os
			<< pi.name
			<< ":\n\nsignals " << pi.signals
			<< "\n\ncommands " << pi.commands
			;
	}
}

namespace slim {

	namespace interface_grammar {

		static x3::rule<struct parsed_interface_class, ast::parsed_interface> parsed_interface{ "parsed_interface" };

		auto ws = x3::space;
		auto ws_opt = x3::omit[*ws];
		auto ws_req = x3::omit[+ws];

		auto identifier_start = x3::alpha | x3::char_('_');
		auto identifier_body = x3::alnum | x3::char_('_');
		auto identifier
			= x3::rule<struct identifier_class, ast::identifier>{}
			= identifier_start >> *identifier_body;

		static x3::rule<struct type_class, ast::type_t> type{ "type" };

		auto templated_type // % operator can not be used if from a % b b has to be kept!
			= identifier >> -(*ws >> x3::char_('<') >> ((*ws >> (type | +x3::alnum)) >> *(*ws >> x3::char_(',') >> (*ws >> (type | +x3::alnum)))) >> *ws >> x3::char_('>'));

		auto pointers
			= x3::char_("*&") % *ws;

		auto scope_separator// prevents omitting
			= x3::rule<struct scope_separator_class, std::string>{}
			= x3::lit("::");

		auto type_def
			= -scope_separator >> *ws >> templated_type >> *(*ws >> scope_separator >> *ws >> templated_type) >> -(*ws >> pointers);

		BOOST_SPIRIT_DEFINE(type);

		auto parameter
			= x3::rule<struct parameter_class, ast::parameter>{}
			= type >> ws_opt >> identifier;

		auto parameter_list
			= x3::rule<struct parameter_list_class, ast::parameter_list>{}
			= parameter % (ws_opt >> ',' >> ws_opt);

		auto function
			= x3::rule<struct function_class, ast::function>{}
			= type >> ws_req >> identifier >> ws_opt >> '(' >> ws_opt >> -parameter_list >> ws_opt >> ')' >> ws_opt >> ';';



		auto interface_name
			= x3::rule<struct interface_name_class, ast::interface_name>{}
			= x3::lit("interface") >> ws_req >> identifier >> ws_opt >> ';';

		auto signal_block
			= x3::rule<struct signal_block_class, ast::signal_block>{}
			= x3::lit("signal") >> ws_opt >> '{'
			//>> ws_opt % function
			>> *(ws_opt >> function) >> ws_opt
			>> '}';

		auto command_block
			= x3::rule<struct command_block_class, ast::command_block>{}
			= x3::lit("command") >> ws_opt >> '{'
			//>> ws_opt % function
			>> *(ws_opt >> function) >> ws_opt
			>> '}';

		auto parsed_interface_def
			=  ws_opt >> interface_name
			>> ws_opt >> command_block
			>> ws_opt >> signal_block
			>> ws_opt;

		BOOST_SPIRIT_DEFINE(parsed_interface);
	}

	auto grammar = interface_grammar::parsed_interface;

	parsed_interface parse_interface(std::string str) {
		str = remove_comments(str);

		auto f = str.begin(), l = str.end();

		ast::parsed_interface data;

		if (parse(f, l, slim::grammar, data)) {
			return data;
		} else {
			throw trace("Error: interface parse failed");
		}
	}
}
