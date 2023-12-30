#ifndef CAKE_H_
#define CAKE_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "file_api.h"

/// ====================== APIs ==========================

struct BuildConfig {
	std::string source_directory = "."; ///< current working directory
	std::string build_directory = "out"; ///< where do you want to store the build files
	std::string lib; ///< which library to build.
	std::string bin; ///< which binary to build.
	std::vector<std::string> options; /// build options passed to cake(actually cmake).
};

struct RunConfig {
	std::string bin; ///< which binary to run.
	std::vector<std::string> options; /// run options passed to the binary.
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
