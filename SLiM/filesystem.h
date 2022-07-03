#pragma once

#if __cplusplus >= 201703L
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "error.h"
#include <fstream>
#include <sstream>

std::string read_file(std::string file) {
	try {
		std::ifstream in(file, std::ifstream::in);
		return static_cast<std::stringstream const&>(std::stringstream() << in.rdbuf()).str();
	} catch (...) {
		throw trace("Error: failed to open file: \"" + file + "\"");
	}

	// unreachable
	return std::string();
}