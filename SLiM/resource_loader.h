#pragma once

#include "configuration_parser.h"
#include "interface_parser.h"
#include "module_parser.h"
#include "accesspoint_parser.h"

#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include "filesystem.h"
#include "error.h"

namespace slim {

	std::string get_slim_home() {
		// only unsafe if the returned pointer is used after getenv is called again, but here it can not be done, the value is saved elsewhere
#pragma warning(suppress : 4996)
		char* slim_home = std::getenv("SLIM_HOME");
		if (slim_home == nullptr) {
			throw trace("Error: SLIM_HOME environment variable is not set");
		}

		std::string home(slim_home);
		if (home.size() == 0) {
			throw trace("Error: SLIM_HOME environment variable is set, but empty");
		}

		return home;
	}

	class resource_loader {
	public:
		struct resource {
			enum type_t {NONE=0, MODULE, CONFIGURATION, INTERFACE};

			std::string name;
			fs::path location;
			type_t type;
		};

	private:
		std::vector<fs::path>& search_directories;
		std::map<std::string, resource> components;
		std::map<std::string, parsed_configuration> configurations;
		std::map<std::string, parsed_interface> interfaces;
		std::map<std::string, parsed_module> modules;
		parsed_accesspoint* accesspoint = nullptr;

		std::vector<fs::path>::iterator path_it;
		fs::recursive_directory_iterator dir_it;

		static resource::type_t get_component_type_from_extension(std::string extension) {
			static const std::map<std::string, resource::type_t> m({
				{ ".h", resource::MODULE},
				{ ".slim", resource::CONFIGURATION },
				{ ".if", resource::INTERFACE }
			});

			auto it = m.find(extension);
			return it == m.end() ? resource::NONE : it->second;
		}

		// if the returned resource 
		resource evaluate_file(std::string& component_name, fs::path& path) {
			resource c;
			c.type = resource::NONE;
			if (!fs::is_directory(path)) {

				auto ext = path.extension().string();
				c.name = path.stem().string();
				c.location = path;
				c.type = get_component_type_from_extension(ext);

				if (c.type != resource::NONE) {
					components.try_emplace(c.name, c);

					// indicate wrong component name with NONE type
					if (c.name != component_name) {
						c.type = resource::NONE;
					}
				}
			}
			return c;
		}

		resource locate_component(std::string component_name) {

			while (path_it != search_directories.end() || dir_it != fs::recursive_directory_iterator()) {

				if (dir_it == fs::end(dir_it)) {
					dir_it = fs::recursive_directory_iterator(*path_it);
					++path_it;
				}

				while (dir_it != fs::end(dir_it)) {

					auto path = (*dir_it).path();
					++dir_it;

					auto r = evaluate_file(component_name, path);
					if (r.type != resource::NONE) return r;
				}
			}

			throw trace("Error: resource is not found \"" + component_name + "\"");
		}

	public:
		resource_loader(std::vector<fs::path>& search_directories, std::string root_config_name, fs::path root_config_file) : search_directories(search_directories) {
			path_it = search_directories.begin();
			dir_it = fs::recursive_directory_iterator();
			evaluate_file(root_config_name, root_config_file);
		}

		resource find(std::string component_name) {
			resource c;

			auto it = components.find(component_name);
			if (it == components.end()) {
				c = locate_component(component_name);
			} else {
				c = it->second;
			}

			return c;
		}

		resource load(std::string component_name) {
			resource c = find(component_name);

			switch (c.type) {
			case resource::CONFIGURATION:
				load_configuration(component_name);
				break;
			case resource::INTERFACE:
				load_interface(component_name);
				break;
			case resource::MODULE:
				load_module(component_name);
				break;
			}

			return c;
		}

		parsed_configuration load_configuration(std::string component_name) {
			auto it = configurations.find(component_name);
			if (it == configurations.end()) {
				auto c = find(component_name);

				if (c.type != resource::CONFIGURATION) {
					throw trace("Error: resource \"" + component_name + "\" is not a configuration by file extension");
				}

				std::cout << "Loading: " << c.location.string() << std::endl;

				auto content = read_file(c.location.string());
				parsed_configuration parsed;
				try {
					parsed = parse_configuration(content);
				} catch (trace t) {
					t.add_scope("In file: " + c.location.string());
					throw t;
				}
				configurations.emplace(component_name, parsed);
				return parsed;
			} else {
				return it->second;
			}
		}

		parsed_interface load_interface(std::string component_name) {
			auto it = interfaces.find(component_name);
			if (it == interfaces.end()) {
				auto c = find(component_name);

				if (c.type != resource::INTERFACE) {
					throw trace("Error: resource \"" + component_name + "\" is not an interface by file extension");
				}

				std::cout << "Loading: " << c.location.string() << std::endl;

				auto content = read_file(c.location.string());
				parsed_interface parsed;
				try {
					parsed = parse_interface(content);
				} catch (trace t) {
					t.add_scope("In file: " + c.location.string());
					throw t;
				}
				interfaces.emplace(component_name, parsed);
				return parsed;
			} else {
				return it->second;
			}
		}

		parsed_module load_module(std::string component_name) {
			auto it = modules.find(component_name);
			if (it == modules.end()) {
				auto c = find(component_name);

				if (c.type != resource::MODULE) {
					throw trace("Error: resource \"" + component_name + "\" is not a module by file extension");
				}

				std::cout << "Loading: " << c.location.string() << std::endl;

				auto content = read_file(c.location.string());
				parsed_module parsed;
				try {
					parsed = parse_module(content);
				} catch (trace t) {
					t.add_scope("In file: " + c.location.string());
					throw t;
				}
				modules.emplace(component_name, parsed);
				return parsed;
			} else {
				return it->second;
			}
		}

		parsed_accesspoint load_accesspoint() {
			if (accesspoint == nullptr) {

				auto c = find("AccessPoint");

				auto content = read_file(c.location.string());
				parsed_accesspoint parsed;
				try {
					parsed = parse_accesspoint(content);
				} catch (trace t) {
					t.add_scope("In file: " + c.location.string());
					throw t;
				}
				accesspoint = new parsed_accesspoint(parsed);
				return parsed;
			} else {
				return *accesspoint;
			}
		}

	};
}