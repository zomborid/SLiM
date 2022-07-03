#pragma once

#include <set>

#include "primitives.h"
#include "resource_loader.h"

namespace slim {

	struct builder_bundle {
		resource_loader& loader;
		scope<component_t*>& global_scope;
		uid_generator& generator;
	};

	class module_builder {
		builder_bundle bundle;
		scope<component_t*>& global_scope;
		resource_loader& loader;
		parsed_module parsed;
		component_t* obj;

		void check_naming(std::string module_type) {
			if (parsed.name != module_type) {
				auto t = trace("Error: component name not matches file name");
				t.add_scope("Expected \"" + module_type + "\", got \"" + parsed.name + "\"");
				throw t;
			}

			std::set<std::string> names;
			auto check_name_uniqueness = [&](auto res) {
				if (!names.emplace(res.first).second) {
					throw trace("Error: duplicate identifier \"" + res.first + "\"");
				}
			};

			for (auto task : parsed.tasks) {
				if (!names.emplace(task).second) {
					throw trace("Error: duplicate identifier \"" + task + "\"");
				}
			}

			std::for_each(parsed.use.begin(), parsed.use.end(), check_name_uniqueness);
			std::for_each(parsed.provide.begin(), parsed.provide.end(), check_name_uniqueness);
		}

		void load_dependencies() {
			// load interfaces
			for (auto i : parsed.use) {
				loader.load(i.second.type);
			}
			for (auto i : parsed.provide) {
				loader.load(i.second.type);
			}
		}

		// belsõ összeköttetések helyessége, nem foglalkozunk a külsõ dependenciákkal
		// minden definiált command vagy signal megvan-e valósítva
		// interface-ek függvényei definiáltak-e az interface leíróban
		// command és signal iránya konzisztens-e az interfésszel
		void check_internal_routing() {

			// check used interface integrity
			for (auto v : parsed.use) {
				auto iface = loader.load_interface(v.second.type);
				auto signals = parsed.signals[v.first];

				// is every signal implemented in used interface
				for (auto signal : iface.signals) {
					auto it = signals.find(signal.name);
					if (it == signals.end()) {
						throw trace("Error: implementation not found for signal " + v.first + "." + signal.name);
					}

					if (it->second.parameters.size() != signal.parameters.size()) {
						throw trace("Error: mismatch in number of parameters for signal " + v.first + "." + signal.name);
					}

					// remove from local instance for easy check of unexpected extra signals
					signals.erase(it);
				}

				// is every signal that is implemented, defined by the given interface
				// given that every implemented signal is erased, we should only look for remaining ones
				if (signals.size() > 0) {
					// there are more signal implementations for interface than expected
					auto extra = signals.begin()->second;
					throw trace("Error: unexpected signal " + extra.iface + "." + extra.name);
				}
			}

			// check provided interface integrity
			for (auto v : parsed.provide) {
				auto iface = loader.load_interface(v.second.type);
				auto commands = parsed.commands[v.first];

				// is every command implemented in provided interface
				for (auto command : iface.commands) {
					auto it = commands.find(command.name);
					if (it == commands.end()) {
						throw trace("Error: implementation not found for command " + v.first + "." + command.name);
					}

					if (it->second.parameters.size() != command.parameters.size()) {
						throw trace("Error: mismatch in number of parameters for command " + v.first + "." + command.name);
					}

					// remove from local instance for easy check of unexpected extra commands
					commands.erase(it);
				}

				// is every command that is implemented, defined by the given interface
				// given that every implemented command is erased, we should only look for remaining ones
				if (commands.size() > 0) {
					// there are more signal implementations for interface than expected
					auto extra = commands.begin()->second;
					throw trace("Error: unexpected command " + extra.iface + "." + extra.name);
				}
			}

			// check that implemented commands belong to provided interfaces
			for (auto v : parsed.commands) {
				if (parsed.provide.find(v.first) == parsed.provide.end()) {
					throw trace("Error: interface is not provided for implemented command " + v.first + "." + v.second.begin()->first);
				}
			}

			// check that implemented signals belong to used interfaces
			for (auto v : parsed.signals) {
				if (parsed.use.find(v.first) == parsed.use.end()) {
					throw trace("Error: interface is not used for implemented signal " + v.first + "." + v.second.begin()->first);
				}
			}
		}

		module_builder(std::string module_type, parsed_module parsed, bool is_global, scope<std::string>& parameters, builder_bundle bundle) : bundle(bundle), global_scope(bundle.global_scope), loader(bundle.loader), parsed(parsed), obj(new component_t(parsed, is_global, bundle.generator)) {

			try {
				check_naming(module_type);
				obj->parameters = parameters;
				load_dependencies();
				check_internal_routing();
				std::cout << "Built module: " << module_type << std::endl;
			} catch (trace t) {
				t.add_scope("In file: " + loader.find(module_type).location.string());
				throw t;
			}
		}

		friend class configuration_builder;

	public:
	};

	class configuration_builder {
		builder_bundle bundle;
		scope<component_t*>& global_scope;
		resource_loader& loader;
		parsed_configuration parsed;
		component_t* obj;


		void check_existence_of_parameters(std::vector<std::string>& expected, scope<std::string> provided) {
			for (auto param : expected) {
				auto it = provided.find(param);
				if (it == provided.end()) {
					throw trace("Error: parameter \"" + param + "\" is required but was not specified");
				}

				provided.erase(it);
			}

			if (provided.size() > 0) {
				throw trace("Error: parameter \"" + provided.begin()->first + "\" is not expected");
			}
		}

		// if a parameter value matches the name of a parameter from outside, copy the value received from outside
		void integrate_outer_parameter_list(const scope<std::string>& outer, scope<std::string>& local) {
			for (auto& param : local) {
				auto it = outer.find(param.second);
				if (it != outer.end()) {
					param.second = it->second;
				}
			}
		}

		void check_naming(std::string configuration_type) {
			if (parsed.name != configuration_type) {
				auto t = trace("Error: component name not matches file name");
				t.add_scope("Expected \"" + configuration_type + "\", got \"" + parsed.name + "\"");
				throw t;
			}

			std::set<std::string> names;
			auto check_name_uniqueness = [&](auto res) {
				if (!names.emplace(res.first).second) {
					throw trace("Error: duplicate identifier \"" + res.first + "\"");
				}
			};

			std::set<std::string> parameter_names;
			for (auto param : parsed.parameters) {
				if (!parameter_names.emplace(param).second) {
					throw trace("Error: duplicate parameter name \"" + param + "\"");
				}
			}

			std::for_each(parsed.use.begin(), parsed.use.end(), check_name_uniqueness);
			std::for_each(parsed.components.begin(), parsed.components.end(), check_name_uniqueness);
			std::for_each(parsed.provide.begin(), parsed.provide.end(), check_name_uniqueness);
		}

		void load_dependencies() {

			// load interfaces
			for (auto i : parsed.use) {
				loader.load(i.second.type);
			}
			for (auto i : parsed.provide) {
				loader.load(i.second.type);
			}

			// load used components
			for (auto p : parsed.components) {

				auto c = p.second;

				auto component_type = loader.load(c.type).type;

				if (c.name == "AccessPoint") {
					if (component_type != resource_loader::resource::type_t::MODULE) {
						throw trace("Error: the AccessPoint copmonent must be a module");
					}

					if (c.instatiated) {
						throw trace("Error: the AccessPoint copmonent can not be instantiated");
					}
				}

				switch (component_type) {
				case resource_loader::resource::type_t::CONFIGURATION:
					if (c.instatiated && obj->local_scope.find(c.identifier()) == obj->local_scope.end()){
						auto config = loader.load_configuration(c.type);
						check_existence_of_parameters(config.parameters, c.parameters);
						integrate_outer_parameter_list(obj->parameters, c.parameters);
						configuration_builder builder(c.type, config, false, c.parameters, bundle);
						obj->local_scope.emplace(c.identifier(), builder.obj);
					} else if (!c.instatiated && global_scope.find(c.identifier()) == global_scope.end()) {
						auto config = loader.load_configuration(c.type);
						check_existence_of_parameters(config.parameters, c.parameters);
						integrate_outer_parameter_list(obj->parameters, c.parameters);
						configuration_builder builder(c.type, config, true, c.parameters, bundle);
						global_scope.emplace(c.identifier(), builder.obj);
					}
					break;
				case resource_loader::resource::type_t::MODULE:
					if (c.instatiated && obj->local_scope.find(c.identifier()) == obj->local_scope.end()) {
						auto module = loader.load_module(c.type);
						check_existence_of_parameters(module.parameters, c.parameters);
						integrate_outer_parameter_list(obj->parameters, c.parameters);
						module_builder builder(c.type, module, false, c.parameters, bundle);
						obj->local_scope.emplace(c.identifier(), builder.obj);
					} else if (!c.instatiated && global_scope.find(c.identifier()) == global_scope.end()) {
						auto module = loader.load_module(c.type);
						check_existence_of_parameters(module.parameters, c.parameters);
						integrate_outer_parameter_list(obj->parameters, c.parameters);
						module_builder builder(c.type, module, true, c.parameters, bundle);
						global_scope.emplace(c.identifier(), builder.obj);
					}
					break;
				}

				if (c.instatiated) {
					obj->naming[c.name] = obj->local_scope[c.identifier()];
				} else {
					obj->naming[c.name] = global_scope[c.identifier()];
				}
				
				// end for
			}
		}

		// belsõ összeköttetések helyessége, nem foglalkozunk a külsõ dependenciákkal
		// interface-ek típusai megegyeznek-e a drótozásnál
		// interface kapcsolatok tárolása pointerekkel
		void resolve_internal_routing() {
			for (auto r : parsed.routing) {
				std::string user_iface_type;
				std::string provider_iface_type;
				iface_link_t* user = nullptr;
				component_t* provider = nullptr;

				if (r.user.external) {
					auto iface_it = obj->provided_ifaces.find(r.user.iface);
					if (iface_it == obj->provided_ifaces.end()) {
						throw trace("Error: component does not provide external interface " + r.user.iface);
					}
					user = &iface_it->second;
				} else {
					auto component_it = obj->naming.find(r.user.component);
					if (component_it == obj->naming.end()) {
						throw trace("Error: no component was declared with name " + r.user.component);
					}
					auto iface_it = component_it->second->used_ifaces.find(r.user.iface);
					if (iface_it == component_it->second->used_ifaces.end()) {
						throw trace("Error: component " + r.user.component + " does not use interface " + r.user.iface);
					}
					user = &iface_it->second;
				}
				user_iface_type = user->type;

				if (r.provider.external) { // interface is connected through a use link in this configuration
					provider = obj;
					auto iface_it = provider->used_ifaces.find(r.provider.iface);
					if (iface_it == provider->used_ifaces.end()) {
						throw trace("Error: component does not use external interface " + r.provider.iface);
					}
					provider_iface_type = parsed.use[r.provider.iface].type;
				} else { // interface is connected to a component in this configuration (local or global)
					auto component_it = obj->naming.find(r.provider.component);
					if (component_it == obj->naming.end()) {
						throw trace("Error: no component was declared with name " + r.provider.component);
					}
					provider = component_it->second;
					auto iface_it = provider->provided_ifaces.find(r.provider.iface);
					if (iface_it == provider->provided_ifaces.end()) {
						throw trace("Error: component " + r.provider.component + " does not provide interface " + r.provider.iface);
					}
					provider_iface_type = iface_it->second.type;
				}

				if (user_iface_type != provider_iface_type) {
					throw trace("Error: interface type mismatch, " + user_iface_type + " -> " + provider_iface_type);
				}

				// check that link is not used by multiple routing statements
				if (user->component != nullptr) {
					std::string iface = r.user.external ? r.user.iface : (r.user.component + "." + r.user.iface);
					throw trace("Error: used interface is already connected " + iface);
				}

				// make resolvable link
				user->component = provider;
				user->name_in_provider = r.provider.iface;
			}

			// check if necessary links are connected for provide interfaces, used ones will only be connected later
			for (auto provide : obj->provided_ifaces) {
				if (provide.second.component == nullptr) {
					std::string iface = provide.second.type + (provide.second.name == provide.second.type ? "" : (" as " + provide.second.name));
					throw trace("Error: missing link for provided interface " + iface);
				}
			}

			// check if necessary links are connected for used interfaces in local components, provided ones have the internal check
			// global components can only be examined after full build
			for (auto c : obj->local_scope) {
				for (auto use : c.second->used_ifaces) {
					if (use.second.component == nullptr) {
						std::string iface = use.second.type + (use.second.name == use.second.type ? "" : (" as " + use.second.name));
						throw trace("Error: missing link for used interface " + iface + " of local component " + c.first);
					}
				}
			}
		}

		configuration_builder(std::string configuration_type, parsed_configuration parsed, bool is_global, scope<std::string>& parameters, builder_bundle bundle) : bundle(bundle), global_scope(bundle.global_scope), loader(bundle.loader), parsed(parsed), obj(new component_t(parsed, is_global, bundle.generator)) {

			try {
				check_naming(configuration_type);
				obj->parameters = parameters;
				load_dependencies();
				resolve_internal_routing();
				std::cout << "Built configuration: " << configuration_type << std::endl;
			} catch (trace t) {
				t.add_scope("In file: " + loader.find(configuration_type).location.string());
				throw t;
			}
		}

		friend application build_application(std::string, resource_loader&);
	public:
	};

	namespace app_build {

		void validate_global_scope_connections(scope<component_t*>& global_scope) {
			for (auto c : global_scope) {
				for (auto use : c.second->used_ifaces) {
					if (use.second.component == nullptr) {
						std::string iface = use.second.type + (use.second.name == use.second.type ? "" : (" as " + use.second.name));
						throw trace("Error: missing link for used interface " + iface + " of global component " + c.first);
					}
				}
			}
		}

		iface_link_t resolve_link_chain(iface_link_t link) {
			while (!link.component->is_module) {
				auto it = link.component->used_ifaces.find(link.name_in_provider);
				iface_link_t* next;
				if (it != link.component->used_ifaces.end()) {
					next = &it->second;
				} else {
					next = &link.component->provided_ifaces[link.name_in_provider];
				}
				link.name_in_provider = next->name_in_provider;
				link.component = next->component;
			}

			return link;
		}

		void resolve_scope_connectivity(application& app, scope<component_t*>& scope) {
			// az összeköttetési lánc redukálása direkt összeköttetésekre

			for (auto c : scope) {
				if (c.second->is_module) {
					auto& m = *c.second;

					for (auto& link : m.used_ifaces) {
						link.second = resolve_link_chain(link.second);
						link.second.component->provided_ifaces[link.second.name_in_provider].user_ifaces.push_back(
							std::make_pair(c.second, &link.second)
						);
					}


					app.modules.emplace(m.module_id, &m);
				} else {
					resolve_scope_connectivity(app, c.second->local_scope);
				}
			}
		}

		// reduce interface connections to direct links between modules
		void resolve_connectivity(application& app, scope<component_t*>& global_scope, component_t* obj) {
			resolve_scope_connectivity(app, global_scope);
			resolve_scope_connectivity(app, obj->local_scope);
		}

		void add_system_module(builder_bundle b) {
			try {
				parsed_module startup = b.loader.load_module("System");
				auto m = new component_t(startup, true, b.generator);
				b.global_scope[m->type] = m;
			} catch (trace t) {
				t.add_scope("In build: System module is required, include system directory in your build path");
				throw t;
			}
		}

		void process_accesspoint(application& app, resource_loader& loader) {
			auto it = std::find_if(app.modules.cbegin(), app.modules.cend(), [](const auto pair) {
				return pair.second->type == "AccessPoint";
			});

			if (it != app.modules.cend()) {
				app.accesspoint_module = it->second;
				std::cout << "Access point found" << std::endl;

				app.accesspoint = loader.load_accesspoint();

			} else {
				std::cout << "No access point found" << std::endl;
			}
		}
	}

	application build_application(std::string root_config_type, resource_loader& loader) {

		auto parsed = loader.load_configuration(root_config_type);

		scope<component_t*> global_scope;
		
		uid_generator generator(1);
		builder_bundle bundle{loader, global_scope, generator};

		// module #1 should always be the System module, so it is callable through _slim_app::_m1
		app_build::add_system_module(bundle);

		configuration_builder builder(root_config_type, parsed, false, scope<std::string>(), bundle);
		application app;

		app_build::validate_global_scope_connections(global_scope);
		app_build::resolve_connectivity(app, global_scope, builder.obj);
		app_build::process_accesspoint(app, loader);

		std::cout << "Application built successfully" << std::endl;

		return app;
	}
}