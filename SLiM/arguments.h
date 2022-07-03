#pragma once

#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>
#include <string>


class arguments : public std::map<std::string, std::string> {
	std::vector<std::string> flag_keys;
	std::vector<std::string> parameter_keys;
	std::vector<std::string> required_keys;

	static std::string extract_key(std::string key) {

		if (key.size() == 0 || key[0] != '-') return "";
		
		if ((key.size() == 1) || (key.size() == 2 && key[1] == '-')) {
			return "";
		}

		return key.substr(key[1] == '-' ? 2 : 1);
	}

	void print_missed_key_error(std::ostream& err) {
		err << "Error: required command line parameter '-" << missed_keys.front() << "' is not found" << std::endl;
	}

	void print_unknown_key_error(std::ostream& err) {
		err << "Error: command line option '-" << unknown_keys.front() << "' is not recognized" << std::endl;
	}

	void print_missed_parameter_of_key_error(std::ostream& err) {
		err << "Error: command line option '-" << missed_parameter_of_keys.front() << "' is missing a parameter" << std::endl;
	}

public:
	std::vector<std::string> missed_keys;				// keys that are required but were not present
	std::vector<std::string> missed_parameter_of_keys;	// encountered keys but no parameter was present while it was expected
	std::vector<std::string> unknown_keys;				// encountered key that is not a stated flag or parameter

	std::vector<std::string> unkeyed_parameters;		// parameters without recognized preceeding key
														// not counts as an error

	// passed keys are processed as flags
	// keys passed here can not be parameter keys
	void set_flag_keys(std::list<std::string> keyset) {
		flag_keys.clear();
		std::copy(keyset.begin(), keyset.end(), std::back_inserter(flag_keys));
	}

	// passed keys should have parameters after them
	// keys passed here can not be flag keys
	void set_parameter_keys(std::list<std::string> keyset) {
		parameter_keys.clear();
		std::copy(keyset.begin(), keyset.end(), std::back_inserter(parameter_keys));
	}

	// passed keys are required
	// keys should be present in parameter keys
	void set_required_keys(std::list<std::string> keyset) {
		required_keys.clear();
		std::copy(keyset.begin(), keyset.end(), std::back_inserter(required_keys));
	}

	// returns true if any expectation was violated
	bool parse(int argc, char** argv, void (*print_usage)(std::ostream&) = nullptr, std::ostream& err = std::cerr) {
		bool error = false;

		std::vector<std::string> vec;

		clear();
		missed_keys.clear();
		missed_parameter_of_keys.clear();
		unknown_keys.clear();
		unkeyed_parameters.clear();

		for (int i = 0; i < argc != 0; ++i) {
			vec.push_back(std::string(argv[i]));
		}

		std::copy(required_keys.begin(), required_keys.end(), std::back_inserter(missed_keys));

		for (unsigned int i = 0; i < vec.size(); ++i) {

			std::string key = extract_key(vec[i]);

			if (key.size() > 0) { // key

				if (std::find(flag_keys.begin(), flag_keys.end(), key) != flag_keys.end()) { // flag key
					emplace(key, "");

				} else if (std::find(parameter_keys.begin(), parameter_keys.end(), key) != parameter_keys.end()) { // parameter key

					std::remove(missed_keys.begin(), missed_keys.end(), key);

					if (i + 1 < vec.size()) { // vector has next element, it can be parameter value
						auto next = extract_key(vec[i + 1]);

						if (next.size() > 0) { // next element is a key, not a value
							missed_parameter_of_keys.push_back(key);
							if (!error) print_missed_parameter_of_key_error(err);
							error = true;

						} else { // next element is a parameter value
							emplace(key, vec[i + 1]);
							++i;
						}

					} else {
						missed_parameter_of_keys.push_back(key);
						if (!error) print_missed_parameter_of_key_error(err);
						error = true;
					}

				} else {
					unknown_keys.push_back(key);
					if (!error) print_unknown_key_error(err);
					error = true;
				}

			} else { // unkeyed parameter
				unkeyed_parameters.push_back(vec[i]);
			}
		}

		if (missed_keys.size() > 0) {
			if (!error) print_missed_key_error(err);
			error = true;
		}

		if (error && print_usage != nullptr) {
			print_usage(err);
		}

		return error;
	}


};

