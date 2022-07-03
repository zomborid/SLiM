#include "module_parser.h"

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

		using identifier = parsed_module::identifier;

		using module_name = parsed_module::module_name;

		using type_t = parsed_module::type_t;
		using parameter = parsed_module::parameter;

		using parameter_list = parsed_module::parameter_list;

		//using function = parsed_module::function;

		using interface_declaration = std::vector<identifier>;
		struct interface_function_statement{
			type_t return_type;
			identifier direction;
			identifier iface;
			identifier name;
			parameter_list parameters;
		};

		using task_statement = identifier;
		struct parameter_statement {
			identifier name;
		};

		using statement = boost::variant<interface_function_statement, interface_declaration, task_statement, parameter_statement>;

		struct module {
			identifier name;
			std::vector<statement> statements;
		};


		struct select_visitor : public boost::static_visitor<int> {
			interface_function_statement& fs; interface_declaration& is; task_statement& ts; parameter_statement& ps;
			select_visitor(interface_function_statement& fs, interface_declaration& is, task_statement& ts, parameter_statement& ps) : fs(fs), is(is), ts(ts), ps(ps) {}

			int operator()(interface_function_statement& v) { fs = v; return v.direction == "Command" ? 0 : (v.direction == "Signal" ? 1 : -1); }

			// find == 0 checks if the string starts with the given literal, it might have the ending: ...As
			int operator()(interface_declaration& v) { is = v; return v[0].find("ProvideInterface") == 0 ? 2 : (v[0].find("UseInterface") == 0 ? 3 : -1); }

			int operator()(task_statement& v) { ts = v; return 4; }

			int operator()(parameter_statement& v) { ps = v; return 5; }
		};

		parsed_module convert(module data) {
			parsed_module module;

			interface_function_statement fs;
			interface_declaration is;
			task_statement ts;
			parameter_statement ps;
			select_visitor sv(fs, is, ts, ps);

			auto iface_conversion = [](interface_declaration v) {
				auto i = parsed_module::interface_declaration{ v[1], v.size() > 2 ? v[2] : v[1] };
				return std::make_pair(i.name, i);
			};

			module.name = data.name;

			for (auto v : data.statements) {
				switch (v.apply_visitor(sv)) {
				case 0:
					module.commands[fs.iface][fs.name] = parsed_module::function{fs.return_type, fs.iface, fs.name, fs.parameters};
					break;
				case 1:
					module.signals[fs.iface][fs.name] = parsed_module::function{ fs.return_type, fs.iface, fs.name, fs.parameters };
					break;
				case 2:
					module.provide.insert(iface_conversion(is));
					break;
				case 3:
					module.use.insert(iface_conversion(is));
					break;
				case 4:
					module.tasks.push_back(ts);
					break;
				case 5:
					module.parameters.push_back(ps.name);
					break;
				}
			}

			return module;
		}
	}
}

namespace ast = slim::ast;

BOOST_FUSION_ADAPT_STRUCT(
	ast::parameter,
	(ast::type_t, type)
	(ast::identifier, name)
)

BOOST_FUSION_ADAPT_STRUCT(
	ast::parameter_statement,
	(ast::identifier, name)
)

BOOST_FUSION_ADAPT_STRUCT(
	ast::interface_function_statement,
	(ast::type_t, return_type)
	(ast::identifier, direction)
	(ast::identifier, iface)
	(ast::identifier, name)
	(ast::parameter_list, parameters)
)

BOOST_FUSION_ADAPT_STRUCT(
	ast::module,
	(ast::module_name, name)
	(std::vector<ast::statement>, statements)
)

namespace std {
	static inline std::ostream& operator <<(std::ostream& os, ast::parameter const& p) {
		return os << p.type << " " << p.name;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::parameter_statement const& p) {
		return os << "\nParameter " << p.name;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::interface_declaration const& is) {
		return os << "\n" << is[0] << " " << is[1];
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::interface_function_statement const& f) {
		return os << "\n" << f.return_type << " " << f.direction << " " << f.iface << "." << f.name << "(" << f.parameters << ")" << std::endl;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::module const& pm) {
		return os
			<< pm.name
			<< ":\nstatements: " << pm.statements
			;
	}
}

namespace slim {

	namespace module_grammar {

		static x3::rule<struct module_class, ast::module> module{ "module" };

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
			= identifier >> -(ws_opt >> x3::char_('<') >> ((ws_opt >> (type | +x3::alnum)) >> *(ws_opt >> x3::char_(',') >> (ws_opt >> (type | +x3::alnum)))) >> ws_opt >> x3::char_('>'));

		auto pointers
			= x3::rule<struct pointers_class, std::string>{}
			= +(x3::char_("*&") >> ws_opt);

		auto scope_separator // prevents omitting
			= x3::rule<struct scope_separator_class, std::string>{}
			= x3::lit("::");

		auto type_def
			= -(scope_separator >> ws_opt) >> templated_type >> *(ws_opt >> scope_separator >> ws_opt >> templated_type) >> -(ws_opt >> pointers);

		BOOST_SPIRIT_DEFINE(type);

		auto parameter
			= x3::rule<struct parameter_class, ast::parameter>{}
		= type >> ws_opt >> identifier;

		auto parameter_list
			= x3::rule<struct parameter_list_class, ast::parameter_list>{}
		= parameter % (ws_opt >> ',' >> ws_opt);

		auto keyword_command // prevents omitting
			= x3::rule<struct keyword_command_class, std::string>{}
			= x3::lit("Command");

		auto keyword_signal // prevents omitting
			= x3::rule<struct keyword_signal_class, std::string>{}
			= x3::lit("Signal");

		auto interface_function_name
			= (keyword_command | keyword_signal) >> ws_opt >> '(' >> ws_opt >> identifier >> ws_opt >> ',' >> ws_opt >> identifier >> ws_opt >> ')';

		auto interface_function_statement
			= x3::rule<struct interface_function_statement_class, ast::interface_function_statement>{}
			= type >> ws_req >> interface_function_name >> ws_opt >> '(' >> ws_opt >> -parameter_list >> ws_opt >> ')';



		auto keyword_use_interface // prevents omitting
			= x3::rule<struct keyword_use_interface_class, std::string>{}
			= x3::lit("UseInterface");

		auto keyword_provide_interface // prevents omitting
			= x3::rule<struct keyword_provide_interface_class, std::string>{}
			= x3::lit("ProvideInterface");

		auto keyword_use_interface_as // prevents omitting
			= x3::rule<struct keyword_use_interface_class, std::string>{}
			= x3::lit("UseInterfaceAs");

		auto keyword_provide_interface_as // prevents omitting
			= x3::rule<struct keyword_provide_interface_class, std::string>{}
			= x3::lit("ProvideInterfaceAs");

		auto direct_interface_statement
			= x3::rule<struct direct_interface_statement_class, ast::interface_declaration>{}
			= (keyword_use_interface | keyword_provide_interface) >> ws_opt >> '(' >> ws_opt >> identifier >> ws_opt >> ')';

		auto renamed_interface_statement
			= x3::rule<struct renamed_interface_statement_class, ast::interface_declaration>{}
			= (keyword_use_interface_as | keyword_provide_interface_as) >> ws_opt >> '(' >> ws_opt >> identifier >> ws_opt >> ',' >> ws_opt >> identifier >> ws_opt >> ')';

		auto interface_statement
			= x3::rule<struct interface_statement_class, ast::interface_declaration>{}
			= (renamed_interface_statement | direct_interface_statement);

		auto task_statement
			= x3::rule<struct task_statement_class, ast::task_statement>{}
			= x3::lit("Task") >> ws_opt >> '(' >> ws_opt >> identifier >> ws_opt >> ')' >> ws_opt >> '{';

		auto parameter_statement
			= x3::rule<struct parameter_statement_class, ast::parameter_statement>{}
			= (x3::lit("TypeParameter") | x3::lit("Parameter")) >> ws_opt >> '(' >> ws_opt >> identifier >> ws_opt >> ')' >> ws_opt >> ';';

		auto module_name
			= x3::rule<struct module_name_class, ast::module_name>{}
			= x3::lit("Module") >> ws_opt >> '(' >> ws_opt >> identifier >> ws_opt >> ')';

		auto statement
			= x3::rule<struct statement_class, ast::statement>{}
			= (interface_statement | interface_function_statement | task_statement | parameter_statement);

		auto module_def
			= x3::omit[*(x3::char_ - module_name)] >> module_name
			>> *(x3::omit[*(x3::char_ - statement)] >> statement)
			>> x3::omit[*(x3::char_ - statement)];

		BOOST_SPIRIT_DEFINE(module);
	}

	auto grammar = module_grammar::module;

	parsed_module parse_module(std::string str) {
		str = remove_comments(str);

		auto f = str.begin(), l = str.end();

		ast::module data;

		if (parse(f, l, slim::grammar, data)) {
			return ast::convert(data);
		} else {
			throw trace("Error: module parse failed");
		}
	}
}
