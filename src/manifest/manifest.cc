#include "manifest/manifest.h"
#include "utility/toml.hpp"
#include <string>

Manifest ParseManifest()
{
	return toml::parse_file(MANIFEST_FILE);
}


BuildConfig ParseBuildConfigFromManifest()
{
	Manifest manifest = ParseManifest();
	BuildConfig config;

	bool vcpkg_support = manifest["profile"]["vcpkg"].value_or(false);
	config.vcpkg_support = vcpkg_support;

	bool generate_compile_commands = manifest["profile"]["compile_commands"].value_or(false);
	config.options.push_back("CMAKE_EXPORT_COMPILE_COMMANDS=1");

	std::string c_compiler = manifest["profile"]["c_compiler"].value_or("gcc");
	config.options.push_back("CMAKE_C_COMPILER:FILEPATH=" + c_compiler);

	std::string cxx_compiler = manifest["profile"]["cxx_compiler"].value_or("g++");
	config.options.push_back("CMAKE_CXX_COMPILER:FILEPATH=" + cxx_compiler);

	std::string linker = manifest["profile"]["linker"].value_or("ld");
	config.options.push_back("CMAKE_LINKER=" + linker);

	std::string build_type = manifest["profile"]["build-type"].value_or("Debug");
	config.options.push_back("CMAKE_BUILD_TYPE=" + build_type);

	return config;
}

RunConfig ParseRunConfigFromManifest()
{
	Manifest manifest = ParseManifest();
	RunConfig config;

	config.bin = manifest["package"]["default-run"].value_or("");

	return config;
}

DebugConfig ParseDebugConfigFromManifest()
{
	Manifest manifest = ParseManifest();
	DebugConfig config;

	config.bin = manifest["package"]["default-run"].value_or("");
	config.debugger = manifest["profile"]["debugger"].value_or("gdb");

	return config;
}

InstallConfig ParseInstallConfigFromManifest()
{
	Manifest manifest = ParseManifest();
	InstallConfig config;

	bool vcpkg_support = manifest["profile"]["vcpkg"].value_or(false);
	config.vcpkg_support = vcpkg_support;

	return config;
}
