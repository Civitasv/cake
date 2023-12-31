#ifndef CAKE_H_
#define CAKE_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "cmake/file_api.h"

/// ====================== APIs ==========================

struct BuildConfig {
	std::string source_directory = "."; ///< current working directory
	std::string build_directory = "out/cmake"; ///< where do you want to store the build files
	bool vcpkg_support = false; ///< if support vcpkg
	std::string vcpkg_toochain_file = "./packages/vcpkg/scripts/buildsystems/vcpkg.cmake"; ///< vcpkg toolchain file
	std::string vcpkg_executable_file = "./packages/vcpkg/vcpkg"; /// vcpkg executable file
	std::string vcpkg_manifest_directory = "./packages/"; ///< vcpkg manifest file
	std::string vcpkg_packages_directory = "./packages/vcpkg_packages"; ///< vcpkg manifest file
	std::string lib; ///< which library to build.
	std::string bin; ///< which binary to build.
	std::vector<std::string> options; ///< build options passed to cake(actually cmake).
};

struct RunConfig {
	std::string bin; ///< which binary to run.
	std::vector<std::string> args; ///< run options passed to the binary.
};

struct InstallConfig {
	bool vcpkg_support = false; ///< whether support vcpkg.
	std::string port; ///< which library to install.
	bool sync; ///< install all libraries in vcpkg.json.
	std::vector<std::string> options; /// install options passed to the vcpkg.
	std::string vcpkg_manifest_directory = "./packages/"; ///< vcpkg manifest file
	std::string vcpkg_packages_directory = "./packages/vcpkg_packages"; ///< vcpkg manifest file
};

struct CreateConfig {
	std::string name; ///< which name to create.
	std::string type; ///< which template to create.
	std::vector<std::string> options; /// create options passed to the cake.
	
	std::string template_basic_directory = "./template/basic";
	std::string template_vcpkg_directory = "./template/vcpkg";
};

struct MetaData {
	std::vector<std::string> Libs()
	{
		std::vector<std::string> names;
		for (auto& item: libs)
		{
			names.push_back(item.first);
		}

		return names;
	}

	std::vector<std::string> Bins()
	{
		std::vector<std::string> names;
		for (auto& item: bins)
		{
			names.push_back(item.first);
		}

		return names;
	}
	std::unordered_map<std::string, Target> libs;
	std::unordered_map<std::string, Target> bins;
};

#endif // CAKE_H_
