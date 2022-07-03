#pragma once

#include <string>
#include <iostream>
#include <iterator>
#include <list>

class trace : public std::list<std::string> {
	std::string reason;
public:
	trace() : reason("No reason specified") {}
	trace(const std::string err) : reason(err) {}

	void add_scope(std::string scope) {
		push_back(scope);
	}

	friend std::ostream& operator<<(std::ostream &os, const trace& o) {
		os << o.reason << std::endl;
		std::copy(o.begin(), o.end(), std::ostream_iterator<std::string>(os, "\n"));
		return os;
	}
};