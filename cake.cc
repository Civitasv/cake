#include "cake.h"

#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
#include "file_api.h"
#include "common.h"

#include "cxxopts.hpp"

#define CMAKE_COMMAND "cmake"
#define MAKEFILE_COMMAND "mkfile"

using nlohmann::json;

static MetaData meta;

static
bool QueryCodeModelTask(const BuildConfig &config, Task &task)
{
	std::string build_directory = config.build_directory;
	std::function<bool()> fn = [build_directory]() {
		if (MakeQueryCodeModelFile(build_directory)) {
			return true;
		}
		return false;
	};

	task = Task(fn);

	return true;
}

static
bool CMakeGenerateTask(const BuildConfig &config, Task &task)
{
	std::string source_directory = config.source_directory;
	std::string build_directory = config.build_directory;
	std::vector<std::string> options = config.options;

	std::function<bool()> fn = [source_directory, build_directory, options]() {
		std::vector<std::string> args{ CMAKE_COMMAND, "-S", source_directory, "-B", build_directory };
		for (auto &option : options) {
			args.emplace_back("-D" + option);
		}
		RunCmdSync(CMAKE_COMMAND, args);
		return true;
	};
	task = Task(fn);

	return true;
}

static
bool CMakeResolveMetaDataTask(const BuildConfig &config, Task &task)
{
	std::string build_directory = config.build_directory;

	std::function<bool()> fn = [build_directory]() ->bool {
		ReplyIndexV1 reply_index = ResolveReplyIndexFile(build_directory);
		CodemodelV2 codemodel_v2 = ResolveCodemodelFile(build_directory, reply_index);
		json targets = codemodel_v2["configurations"][0]["targets"];
		for (json::iterator it = targets.begin(); it != targets.end(); ++it) {
			std::string target_json_file = (*it)["jsonFile"].template get<std::string>();
			Target target = ResolveTargetFile(build_directory, target_json_file);
			meta.libs[target["name"]] = target;

			if (target["name"] == "EXECUTABLE")
			{
				meta.bins[target["name"]] = target;
			}
		}

		return true;
	};
	
	task = Task(fn);
	return true;
}

static
bool CMakeBuildTask(const BuildConfig &config, Task &task)
{
	std::string build_directory = config.build_directory;
	std::string lib = config.lib;
	std::string bin = config.bin;
	std::vector<std::string> options = config.options;

	std::function<bool()> fn = [build_directory, lib, bin, options]() {
		std::vector<std::string> args{ CMAKE_COMMAND, "--build", build_directory };

		if (!lib.empty())
		{
			if (meta.libs.count(lib) == 0)
			{
				logger->Error(lib, " is not avaliable, the avaliable libs are: [", meta.Libs(), "]");
				return false;
			}
			args.push_back("--target");
			args.push_back(lib);
		} else if (!bin.empty())
		{
			if (meta.libs.count(bin) == 0)
			{
				logger->Error(bin, " is not avaliable, the avaliable binaries are: [", meta.Bins(), "]");
				return false;
			}
			args.push_back("--target");
			args.push_back(bin);
		} else
		{
			args.push_back("--target");
			args.push_back("all");
		}

		for (auto &option : options) {
			args.emplace_back("-D" + option);
		}

		RunCmdSync(CMAKE_COMMAND, args);

		return true;
	};

	task = Task(fn);

	return true;
}

void CakeBuild(const BuildConfig &config)
{
	std::stringstream cmd;
	Tasks tasks;

	Task task;
	// generate query files
	if (QueryCodeModelTask(config, task))
	{
		tasks.AddTask(task);
	}
	// generate task
	if (CMakeGenerateTask(config, task))
	{
		tasks.AddTask(task);
	}
	// metadata
	if (CMakeResolveMetaDataTask(config, task))
	{
		tasks.AddTask(task);
	}
	// build task
	if (CMakeBuildTask(config, task))
	{
		tasks.AddTask(task);
	}

	tasks.Execute();
}

int main(int argc, char **argv)
{
	if (argc == 1) { // then it is `cake` itself
		printf("A wrapper for cmake, avaliable commands\n");
		printf("Usage:\n");
		printf("  cake [build/run] [OPTION...]");
		return 0;
	}

	// firstly, check the mode
	char *mode = argv[1];
	if (strcmp(mode, "build") == 0) {
		cxxopts::Options options(
			"cake build",
			"Compile local packages and all of their dependencies");
		// clang-format off
		options.add_options()
		// target selection options
		("lib", "Build the package's library", cxxopts::value<std::string>())
		("bin", "Build the specified binary", cxxopts::value<std::vector<std::string>>())
		// common options
		("config", "Set configuration value", cxxopts::value<std::vector<std::string>>())
		("help", "Help");
		// clang-format on

		auto parse_result = options.parse(argc - 1, argv + 1);

		BuildConfig config;
		if (parse_result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}
		if (parse_result.count("config")) {
			config.options = std::move(parse_result["config"].as<std::vector<std::string>>());
		}
		if (parse_result.count("lib")) {
			config.lib = std::move(parse_result["lib"].as<std::string>());
		}
		if (parse_result.count("bin")) {
			config.bin = std::move(parse_result["bin"].as<std::string>());
		}

		CakeBuild(config);
	} else if (strcmp(mode, "run")) {
	}

	return 0;
}
