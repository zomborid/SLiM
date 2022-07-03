#pragma once

#include "filesystem.h"
#include "primitives.h"
#include "resource_loader.h"
#include <fstream>
#include <cstdlib>

namespace std {
	template<class Iter, class Transform> string tokenize(Iter begin, Iter end, string spacer, Transform t) {
		stringstream ss;

		if (begin != end) {
			ss << t(*begin);
			++begin;
		}
		while (begin != end) {
			ss << spacer << t(*begin);
			++begin;
		}

		return ss.str();
	}
}

namespace slim {

	namespace wiring_base {

		static fs::path copy_sources(application& app, resource_loader& loader, fs::path output_file_base) {
			fs::path src_dir = output_file_base;
			src_dir.concat("_src");

			/* clear directory every time
			if (fs::exists(src_dir)) {
				fs::remove_all(src_dir);
			}
			fs::create_directory(src_dir);
			/*/
			// don't delete directory, just create it if needed
			if (!fs::exists(src_dir)) {
				fs::create_directory(src_dir);
			}
			/**/

			for (auto m : app.modules) {
				fs::copy_file(loader.find(m.second->type).location, src_dir / m.second->module_id, fs::copy_options::update_existing);
			}

			return src_dir;
		}

		template<class Pred> static bool copy_lines_until(std::ostream& out, std::istream& in, Pred pred) {
			std::string line;
			while (true) {
				std::getline(in, line);

				if (pred(line))
					return false;

				out << line;

				if (in.eof())
					return true;

				out << std::endl;
			}
		}

		static void generate_includes(std::ostream& file, application& app, resource_loader& loader, fs::path src_dir, bool is_copy_source) {
			std::set<std::string> included;

			file << std::endl << std::endl << "// includes start" << std::endl << std::endl;
			if (is_copy_source) {
				for (auto m : app.modules) {
					if (included.find(m.second->type) != included.end()) continue;
					included.emplace(m.second->type);
					file << "#include \"" << (src_dir / m.second->module_id) << "\"" << std::endl;
				}
			} else {
				for (auto m : app.modules) {
					if (included.find(m.second->type) != included.end()) continue;
					included.emplace(m.second->type);
					file << "#include \"" << loader.find(m.second->type).location.string() << "\"" << std::endl;
				}
			}
			file << std::endl << "// includes end" << std::endl << std::endl;
		}

		static void generate_used_interface(component_t& m, iface_link_t iface, std::ostream& file, application& app, resource_loader& loader) {
			parsed_interface parsed = loader.load_interface(iface.type);

			file << "        struct " << iface.type << "_as_" << iface.name << "{ // used interface" << std::endl;
			for (auto c : parsed.commands) {
				bool has_return_value = c.return_type != "void";
				file << "            static " << c.return_type << " " << c.name << "("; // command signature
				file << std::tokenize(c.parameters.begin(), c.parameters.end(), ", ", [](auto p) {return p.type + " " + p.name; }); // parameter list of signature
				file << "){" << (has_return_value ? "return " : "") << iface.component->module_id << ".Command(" << iface.name_in_provider << "," << c.name << ")("; // call of command function in provider
				file << std::tokenize(c.parameters.begin(), c.parameters.end(), ", ", [](auto p) {return p.name; }); // parameters of call
				file << ");}" << std::endl;
			}
			file << "        };" << std::endl;
		}

		static std::string generate_call_to_signal(parsed_interface::function signal, component_t* user, iface_link_t* link) {
			return user->module_id + ".Signal(" + link->name + "," + signal.name + ")("
				+ std::tokenize(signal.parameters.begin(), signal.parameters.end(), ", ", [](auto p) {return p.name; })
				+ ")";
		}

		static void generate_provided_interface(component_t& m, iface_link_t iface, std::ostream& file, application& app, resource_loader& loader) {
			parsed_interface parsed = loader.load_interface(iface.type);

			file << "        struct " << iface.type << "_as_" << iface.name << "{ // provided interface" << std::endl;
			for (auto s : parsed.signals) {
				bool has_return_value = s.return_type != "void";
				file << "            static " << s.return_type << " " << s.name << "(";
				file << std::tokenize(s.parameters.begin(), s.parameters.end(), ", ", [](auto p) {return p.type + " " + p.name; }) << "){" << std::endl;
				
				if (has_return_value) {
					file << "                return ";
					file << std::tokenize(iface.user_ifaces.begin(), iface.user_ifaces.end(), "\n | ", [&](auto use) {
						return "                " + generate_call_to_signal(s, use.first, use.second);
					});
					file << ";" << std::endl;
				} else {
					file << std::tokenize(iface.user_ifaces.begin(), iface.user_ifaces.end(), "\n", [&](auto use) {
						return "                " + generate_call_to_signal(s, use.first, use.second) + ";";
					});
					file << std::endl;
				}
				file << "            }" << std::endl;
			}
			file << "        };" << std::endl;
		}

		void determine_task_numbering(application& app, std::map<std::string, int>& task_numbering) {
			int tid = (int)task_numbering.size();
			for (auto m : app.modules) {
				for (auto task : m.second->module_tasks) {
					task_numbering[m.second->module_id + "." + task] = tid;
					++tid;
				}
			}
		}

		void generate_task_id_type(std::ostream& file, std::map<std::string, int>& task_numbering) {
			if (task_numbering.size() < 255) {
				file << "using tid_t = uint8_t;" << std::endl;
				//file << "#define _SLIM_TID_EMPTY 255" << std::endl;
			} else if (task_numbering.size() < 65535){
				// if the number of tasks reaches 65535, it will not work
				// slim is designed for low infrastructure applications, not massive projects
				file << "using tid_t = uint16_t;" << std::endl;
				//file << "#define _SLIM_TID_EMPTY 65535" << std::endl;
			} else {
				throw trace("Error: too much tasks encountered (" + std::to_string(task_numbering.size())+ ") which exceeds the maximum 65535");
			}
		}

		bool isnumber(const std::string& str) {
			if (str.size() == 0) return false;
			auto it = str.cbegin();
			if (*it == '-') ++it;
			if (it == str.cend()) return false;
			while (it != str.cend() && *it >= '0' && *it <= '9') ++it;
			return it == str.cend();
		}

		void generate_module(component_t& m, std::map<std::string, int>& task_numbering, std::ostream& file, application& app, resource_loader& loader) {
			file << "    " << std::endl;
			file << "    struct " << m.module_id << "_t{" << std::endl;

			for (auto task : m.module_tasks) {
				int tid = task_numbering[m.module_id + "." + task];
				file << "        static constexpr tid_t _task_" << task << "_id = " << tid << ";" << std::endl;
			}
			file << "        " << std::endl;

			for (auto param : m.parameters) {
				if (isnumber(param.second)) {
					file << "        static constexpr auto param_" << param.first << " = " << param.second << ";" << std::endl;
				} else {
					file << "        using param_" << param.first << " = " << param.second << ";" << std::endl;
				}
			}
			file << "        " << std::endl;

			for (auto iface : m.used_ifaces) {
				generate_used_interface(m, iface.second, file, app, loader);
			}

			for (auto iface : m.provided_ifaces) {
				generate_provided_interface(m, iface.second, file, app, loader);
			}

			file << "    };" << std::endl;
			file << "    static _m_" << m.type << "<" << m.module_id << "_t> " << m.module_id << ";" << std::endl;
		}

		void generate_accesspoint_bridging(std::ostream& file, application& app) {
			file << "" << std::endl;
			if (app.accesspoint_module != nullptr) {
				file << "#define AP _slim_app::" << app.accesspoint_module->module_id << std::endl;
			}

			file << "namespace slim_ap {" << std::endl;

			if (app.accesspoint_module != nullptr) {
				for (auto pair : app.accesspoint.functions) {
					auto& f = pair.second;
					bool has_return_value = f.return_type != "void";
					file << "    " << f.return_type << " " << f.name << "("; // function signature
					file << std::tokenize(f.parameters.begin(), f.parameters.end(), ", ", [](auto p) {return p.type + " " + p.name; }); // parameter list of signature
					file << "){" << (has_return_value ? "return " : "") << "AP." << f.name << "("; // call of function
					file << std::tokenize(f.parameters.begin(), f.parameters.end(), ", ", [](auto p) {return p.name; }); // parameters of call
					file << ");}" << std::endl;
				}
			}

			file << "}" << std::endl;
		}

		void generate_accesspoint_declarations(std::ostream& file, application& app) {
			file << "namespace slim_ap {" << std::endl;
			if (app.accesspoint_module != nullptr) {
				for (auto pair : app.accesspoint.functions) {
					auto& f = pair.second;
					file << "    " << f.return_type << " " << f.name << "("; // function signature
					file << std::tokenize(f.parameters.begin(), f.parameters.end(), ", ", [](auto p) {return p.type + " " + p.name; }); // parameter list of signature
					file << ");" << std::endl;
				}
			}
			file << "}" << std::endl;
		}

		void generate_module_wiring(std::map<std::string, int>& task_numbering, std::ostream& file, application& app, resource_loader& loader) {
			file << std::endl << std::endl << "// wiring start" << std::endl << std::endl;

			// main struct
			file << "struct _slim_app {" << std::endl;
			file << "    " << std::endl;

			for (auto m : app.modules) {
				file << "    struct " << m.second->module_id << "; // " << m.second->type << std::endl;
			}

			for (auto m : app.modules) {
				generate_module(*m.second, task_numbering, file, app, loader);
			}

			file << "    " << std::endl;
			file << "};" << std::endl;

			file << "" << std::endl;

			// static variable declarations
			for (auto m : app.modules) {
				file << "_m_" << m.second->type << "<_slim_app::" << m.second->module_id << "_t> _slim_app::" << m.second->module_id << ";" << std::endl;
			}
		}

		void generate_task_wiring(std::map<std::string, int>& task_numbering, std::ostream& file, application& app, resource_loader& loader) {
			file << "" << std::endl;

			// task function links
			{
				for (auto task : task_numbering) {
					file << "static _task_return_t _task" << task.second << "(){return _slim_app::" << task.first << "();}" << std::endl;
				}
			}

			file << "" << std::endl;

			// task pointer vector
			{
				file << "static constexpr int _task_count = " << task_numbering.size() << ";" << std::endl;
				file << "static std::array<_task_t, _task_count> _task_table = {" << std::endl;
				int task_number = 0;
				for (auto task : task_numbering) {
					file << "    _task" << task_number << (task_number < task_numbering.size() - 1 ? "," : "") << std::endl;
					++task_number;
				}
				file << "};" << std::endl;
			}

			file << "" << std::endl;
			file << std::endl << std::endl << "// wiring end" << std::endl << std::endl;
		}
	};

	fs::path generate_wiring(application& app, resource_loader& loader, fs::path output_file_base, bool is_copy_source) {

		fs::path cpp_file = output_file_base;
		cpp_file.concat(".cpp");

		fs::path cpp_base_file(get_slim_home());
		cpp_base_file /= "resources";
		cpp_base_file /= "wiring_base.cpp";

		fs::path output_directory = cpp_file.parent_path();

		if (!fs::exists(output_directory)) {
			fs::create_directories(output_directory);
		}

		std::ofstream cpp_stream(cpp_file.string(), std::ofstream::out);
		std::ifstream cpp_base_stream(cpp_base_file.string(), std::ifstream::in);

		fs::path src_dir;
		if (is_copy_source) {
			src_dir = wiring_base::copy_sources(app, loader, output_directory);
		}

		std::map<std::string, int> task_numbering;
		wiring_base::determine_task_numbering(app, task_numbering);

		// insert typedef for task id type
		wiring_base::copy_lines_until(cpp_stream, cpp_base_stream, [](std::string l) {
			return l.substr(0, 9) == "//{tid_t}";
		});

		wiring_base::generate_task_id_type(cpp_stream, task_numbering);

		// write header section of file
		wiring_base::copy_lines_until(cpp_stream, cpp_base_stream, [](std::string l) {
			return l.substr(0, 10) == "//{wiring}";
		});

		wiring_base::generate_includes(cpp_stream, app, loader, src_dir, is_copy_source);
		wiring_base::generate_module_wiring(task_numbering, cpp_stream, app, loader);
		wiring_base::generate_accesspoint_bridging(cpp_stream, app);
		wiring_base::generate_task_wiring(task_numbering, cpp_stream, app, loader);

		// write footer section of file
		wiring_base::copy_lines_until(cpp_stream, cpp_base_stream, [](std::string l) {
			return false;
		});

		std::cout << "Wiring generated successfully" << std::endl;
		
		return cpp_file;
	}

	fs::path generate_header(application& app, resource_loader& loader, fs::path output_file_base) {

		fs::path h_file = output_file_base;
		h_file.concat(".h");

		fs::path h_base_file(get_slim_home());
		h_base_file /= "resources";
		h_base_file /= "header_base.h";

		fs::path output_directory = h_file.parent_path();

		if (!fs::exists(output_directory)) {
			fs::create_directories(output_directory);
		}

		std::ofstream h_stream(h_file.string(), std::ofstream::out);
		std::ifstream h_base_stream(h_base_file.string(), std::ifstream::in);

		// write header section of file
		wiring_base::copy_lines_until(h_stream, h_base_stream, [](std::string l) {
			return l.substr(0, 10) == "//{wiring}";
		});

		wiring_base::generate_accesspoint_declarations(h_stream, app);

		// write footer section of file
		wiring_base::copy_lines_until(h_stream, h_base_stream, [](std::string l) {
			return false;
		});

		std::cout << "Header generated successfully" << std::endl;

		return h_file;
	}
}