#include "accesspoint_parser.h"

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

		using identifier = parsed_accesspoint::identifier;

		using type_t = parsed_accesspoint::type_t;
		using parameter = parsed_accesspoint::parameter;

		using parameter_list = parsed_accesspoint::parameter_list;

		using function = parsed_accesspoint::function;
		using functions_t = std::vector<function>;

		using accesspoint = functions_t;

		parsed_accesspoint convert(accesspoint data) {
			parsed_accesspoint accesspoint;

			for (auto& f : data) {
				accesspoint.functions.emplace(f.name, f);
			}

			return accesspoint;
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
	ast::function,
	(ast::type_t, return_type)
	(ast::identifier, name)
	(ast::parameter_list, parameters)
)

namespace std {
	static inline std::ostream& operator <<(std::ostream& os, ast::parameter const& p) {
		return os << p.type << " " << p.name;
	}

	static inline std::ostream& operator <<(std::ostream& os, ast::function const& f) {
		return os << "\n" << f.return_type << "." << f.name << "(" << f.parameters << ")" << std::endl;
	}
}

namespace slim {

	namespace accesspoint_grammar {

		struct accesspoint_class {
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
					ss << *curr;
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

		static x3::rule<accesspoint_class, ast::accesspoint> accesspoint{ "accesspoint" };

		auto ws = x3::space;
		auto ws_opt = x3::omit[*ws];
		auto ws_req = x3::omit[+ws];

		auto identifier_start = x3::alpha | x3::char_('_');
		auto identifier_body = x3::alnum | x3::char_('_');
		auto identifier
			= x3::rule<struct identifier_class, ast::identifier>{}
		= identifier_start > *identifier_body;

		static x3::rule<struct type_class, ast::type_t> type{ "type" };

		auto templated_type // % operator can not be used if from a % b b has to be kept!
			= identifier >> -(ws_opt >> x3::char_('<') >> ((ws_opt >> (type | +x3::alnum)) > *(ws_opt >> x3::char_(',') > (ws_opt > (type | +x3::alnum)))) > ws_opt > x3::char_('>'));

		auto pointers
			= x3::rule<struct pointers_class, std::string>{}
			= +(x3::char_("*&") > ws_opt);

		auto scope_separator // prevents omitting
			= x3::rule<struct scope_separator_class, std::string>{}
			= x3::lit("::");

		auto type_def
			= -(scope_separator > ws_opt) >> templated_type >> *(ws_opt >> scope_separator > ws_opt > templated_type) >> -(ws_opt >> pointers);

		BOOST_SPIRIT_DEFINE(type);

		auto parameter
			= x3::rule<struct parameter_class, ast::parameter>{}
		= type > ws_opt > identifier;

		auto parameter_list
			= x3::rule<struct parameter_list_class, ast::parameter_list>{}
		= parameter % (ws_opt >> ',' > ws_opt);

		auto exposed_function
			= x3::rule<struct function_class, ast::function>{}
			= type >> ws_req >> x3::lit("Expose") >> ws_opt >> '(' >> ws_opt >> identifier >> ws_opt >> ')'
			>> ws_opt >> '(' > ws_opt > -parameter_list > ws_opt > ')';


		auto accesspoint_def
			= *(x3::omit[*(x3::char_ - exposed_function)] >> exposed_function)
			>> x3::omit[*(x3::char_ - exposed_function)];

		BOOST_SPIRIT_DEFINE(accesspoint);
	}

	auto grammar = accesspoint_grammar::accesspoint;

	parsed_accesspoint parse_accesspoint(std::string str) {
		str = remove_comments(str);

		auto f = str.begin(), l = str.end();

		ast::accesspoint data;

		if (parse(f, l, slim::grammar, data)) {
			return ast::convert(data);
		} else {
			throw trace("Error: accesspoint parse failed");
		}
	}
}
