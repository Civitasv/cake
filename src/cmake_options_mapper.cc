#include "cmake_options_mapper.h"

std::unordered_map<std::string, std::string> generate_options_mapper = {
	{ "compile_commands", "-DCMAKE_EXPORT_COMPILE_COMMANDS=1" }
};

std::unordered_map<std::string, std::string> build_options_mapper = {

};
