#include "remove_comments.h"

std::string remove_comments(std::string content) {
	size_t off = 0;
	size_t it_double_quote;
	size_t it_escaped_double_quote;
	size_t it_one_line;
	size_t it_multiline;
	size_t first;

	auto const min = [](auto a, auto b) {return a > b ? b : a; };

	while (1) {
		it_double_quote = content.find('"', off);
		it_one_line = content.find("//", off);
		it_multiline = content.find("/*", off);

		first = min(it_double_quote, min(it_one_line, it_multiline));

		if (first == content.npos) {// no match
									// end of content
			break;

		} else if (first == it_double_quote) {// ignore string literals

			off = first + 1;

			//	cases:	1		2			3	4	
			// 			"lit"	"\"\"\"..."	'"'	'\"' 
			if (content[first + 1] == '\'') { // case 3,4
				off = first + 2;

			} else { // case 1,2

				while (1) {
					it_double_quote = content.find('"', off);
					it_escaped_double_quote = content.find("\\\"", off);

					first = min(it_double_quote, it_escaped_double_quote);

					if (first == content.npos) {// no match
						// error, in the middle of string literal
						return content;
					} else if (first == it_double_quote) {
						off = first + 1;
						break;
					} else {
						off = first + 2;
					}
				}

			}

		} else if (first == it_one_line) {
			size_t it_nl = content.find("\n", first + 1);
			content.erase(first, it_nl - first + 1);
			off = first;

		} else if (first == it_multiline) {
			size_t it_close = content.find("*/", first + 1);
			content.erase(first, it_close - first + 2);
			off = first;
		}
	}

	return content;
}