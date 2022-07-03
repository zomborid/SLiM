
#include <iostream>
#include "filesystem.h"
#include "error.h"

#include "resource_loader.h"
#include "application_builder.h"
#include "generate_wiring.h"

#include "arguments.h"

// version information
std::string	build_version	= "1.0";
long		build_number	= BUILD_DATE;



arguments args;
bool is_copy_source;
std::string root_config_name;
fs::path root_config_file;
std::vector<fs::path> search_paths;
fs::path output_file_base;

void print_version() {
	std::cout
		<< "SLiM version " << build_version << " (build " << build_number << ")" << std::endl
		<< "Created by D\xa0niel Zombori" << std::endl;
}

void print_usage(std::ostream& out) {
	out << "Usage: [-options] <root configuration>" << std::endl;
	out << "Options:" << std::endl;
	out << "    -i" << std::endl;
	out << "        configuration and module search paths" << std::endl;
	out << "        a ; separated string of directories" << std::endl;
	out << "    -p" << std::endl;
	out << "        path file, contains additional search paths in separate lines" << std::endl;
	out << "        the -i parameter has the search priority" << std::endl;
	out << "    -o" << std::endl;
	out << "        specifies the output file path and names of .h and .cpp files" << std::endl;
	out << "        specifying \"../cpp/wiring.h\" will create the output files" << std::endl;
	out << "        in the cpp folder starting with \"wiring\"" << std::endl;
	out << "        specifying \"../cpp/wiring\" has the same result" << std::endl;
	out << "        by default it is set to the root configuration file" << std::endl;
	out << "    --help" << std::endl;
	out << "        prints out this text" << std::endl;
	out << "    --no-sys" << std::endl;
	out << "        not includes $(SLIM_HOME)/system directory by default" << std::endl;
	out << "    --version -v" << std::endl;
	out << "        prints version" << std::endl;
	out << "    --copy-sources" << std::endl;
	out << "        to avoid errors caused by sources outside the CPP include paths" << std::endl;
	out << "        the sources will be copied to the <root configuration>_src folder" << std::endl;
}

bool parse_arguments(int argc, char** argv) {
	args.set_flag_keys({"help","version","v","copy-sources","no-sys"});
	args.set_parameter_keys({ "i","p","o" });
	args.set_required_keys({ });
	return args.parse(argc, argv, print_usage);
}

namespace {

	bool acquire_root_config_name_and_path() {
		if (args.unkeyed_parameters.size() < 2) {
			std::cerr << "Error: root configuration file is not specified" << std::endl;
			print_usage(std::cerr);
			return true;
		}

		if (args.unkeyed_parameters.size() > 2) {
			std::cerr << "Error: too much arguments specified" << std::endl;
			print_usage(std::cerr);
			return true;
		}

		root_config_file = fs::path(args.unkeyed_parameters[1]);
		root_config_name = root_config_file.stem().string();

		if (!fs::exists(root_config_file)) {
			std::cerr << "Error: root configuration file does not exist" << std::endl;
			std::cerr << "Not found: " << root_config_file << std::endl;
			return true;
		}

		return false;
	}

	bool acquire_output_file_base() {

		output_file_base = fs::path(root_config_file);
		auto it_o = args.find("o");
		if (it_o != args.end()) {
			output_file_base = fs::path(it_o->second);
		}

		output_file_base.replace_extension();

		return false;
	}

	bool process_search_paths_parameter(std::string content) {
		
		size_t it = 0, last = 0;
		while ((it = content.find_first_of(';', last)) != content.npos) {
			auto str = content.substr(last, it - last);
			if (str.size() > 0) search_paths.push_back(str);
			last = ++it;
		}

		search_paths.push_back(content.substr(last));

		return false;
	}

	bool process_path_file(std::string content) {

		size_t it = 0, last = 0;
		while ((it = content.find_first_of("\r\n", last)) != content.npos) {
			auto str = content.substr(last, it - last);
			if (str.size() > 0) search_paths.push_back(str);
			last = ++it;
		}

		auto str = content.substr(last);
		if (str.size() > 0) search_paths.push_back(str);

		return false;
	}

	bool acquire_priority_search_list() {

		// include paths, added firstly thus it has the highest priority
		auto it_i = args.find("i");
		if (it_i != args.end()) {
			if (process_search_paths_parameter(it_i->second))
				return true;
		}

		// path file
		auto it_p = args.find("p");
		if (it_p != args.end()) {
			if (!fs::exists(it_p->second)) {
				std::cerr << "Error: project path file does not exist" << std::endl;
				std::cerr << "Not found: " << it_p->second << std::endl;
				return true;
			}

			std::string path_file = read_file(it_p->second);
			if (process_path_file(path_file))
				return true;
		}

		// if no path is given, path to the root config is used
		if (search_paths.size() == 0) {
			search_paths.push_back(root_config_file.parent_path());
		}

		// include $(SLIM_HOME)/system directory if needed, with lowest priority
		if (args.find("no-sys") == args.end()) {
			fs::path sys_path(slim::get_slim_home());
			sys_path /= "system";
			search_paths.push_back(sys_path);
		}

		// check path validity
		for (auto it = search_paths.begin(); it != search_paths.end(); ++it) {
			auto path = *it;
			if (!fs::exists(path)) {
				std::cerr << "Error: include path does not exist" << std::endl;
				std::cerr << "Not found: " << path << std::endl;
				return true;
			}

			if (!fs::is_directory(path)) {
				std::cerr << "Error: include path is not a directory" << std::endl;
				std::cerr << "Path: " << path << std::endl;
				return true;
			}
		}

		return false;
	}
}

bool process_arguments() {

	if (acquire_root_config_name_and_path())
		return true;

	if (acquire_output_file_base())
		return true;

	if (acquire_priority_search_list())
		return true;

	is_copy_source = args.find("copy-source") != args.end();

	return false;
}

int execute(int argc, char** argv) {

	if (parse_arguments(argc, argv)) {
		return 1;
	}

	if (args.find("v") != args.end() || args.find("version") != args.end()) {
		print_version();
		return 0;
	}

	if (args.find("help") != args.end()) {
		print_usage(std::cout);
		return 0;
	}

	if (process_arguments()) {
		return 1;
	}

	slim::resource_loader loader(search_paths, root_config_name, root_config_file);
	auto app = slim::build_application(root_config_name, loader);

	auto cpp_path = slim::generate_wiring(app, loader, output_file_base, is_copy_source);
	auto h_path = slim::generate_header(app, loader, output_file_base);

	std::cout << "Done building project " << root_config_name << std::endl;

	return 0;
}

int main(int argc, char** argv) {

	try {
		return execute(argc, argv);
	} catch (trace t) {
		std::cerr << t;
		return 1;
	}

	// unreachable
	return 0;
}