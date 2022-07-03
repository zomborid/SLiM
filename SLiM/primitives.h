#pragma once

#include <string>
#include <list>
#include <map>

#include "configuration_parser.h"
#include "interface_parser.h"
#include "module_parser.h"
#include "accesspoint_parser.h"

namespace slim {

	struct component_t;

	struct iface_link_t {
		std::string type;
		std::string name;
		std::string name_in_provider;
		component_t* component;
		std::vector<std::pair<component_t*,iface_link_t*> > user_ifaces;

		iface_link_t() : component(nullptr) {}

		iface_link_t(std::string type, std::string name, std::string name_in_provider) : type(type), name(name), name_in_provider(name_in_provider), component(nullptr) {}
	};

	template<class T>
	using scope = std::map<std::string, T>;

	class uid_generator {
		int id;
	public:
		uid_generator() : id(1) {}
		uid_generator(int first) : id(first) {}

		int next() {
			return id++;
		}

		int peek() {
			return id;
		}
	};

	struct component_t {

		const int uid;
		const std::string module_id;
		const bool is_module;
		const bool is_global;
		const std::string type;
		std::vector<std::string> module_tasks;
		std::map<std::string, iface_link_t> used_ifaces;		// links to provider outside component
		std::map<std::string, iface_link_t> provided_ifaces;	// links to provider inside component
		scope<component_t*> local_scope;
		scope<component_t*> naming;
		scope<std::string> parameters;

		//component_t() {}

		static int generate_uid() {
			static int id = 0;
			return ++id;
		}

		component_t(parsed_module module, bool is_global, uid_generator& generator) : uid(generator.next()), module_id("_m" + std::to_string(uid)), is_module(true), is_global(is_global), type(module.name) {
			for (auto i : module.use) {
				used_ifaces.emplace(i.second.name, iface_link_t(i.second.type, i.second.name, ""));
			}
			for (auto i : module.provide) {
				provided_ifaces.emplace(i.second.name, iface_link_t(i.second.type, i.second.name, ""));
			}

			module_tasks = module.tasks;
		}

		component_t(parsed_configuration configuration, bool is_global, uid_generator& generator) : uid(0), module_id(""), is_module(false), is_global(is_global), type(configuration.name) {
			for (auto i : configuration.use) {
				used_ifaces.emplace(i.second.name, iface_link_t(i.second.type, i.second.name, ""));
			}
			for (auto i : configuration.provide) {
				provided_ifaces.emplace(i.second.name, iface_link_t(i.second.type, i.second.name, ""));
			}
		}

		~component_t() {
			for (auto ls : local_scope) {
				delete ls.second;
			}
		}
	};

	struct application {
		scope<component_t*> modules;
		component_t* accesspoint_module = nullptr;
		parsed_accesspoint accesspoint;
		//std::map<std::string, task_t> tasks;
	};
}