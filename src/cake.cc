#include "cake.h"

#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
#include "cmake/file_api.h"
#include "utility/common.h"

#include "utility/cxxopts.hpp"

#define CMAKE_COMMAND "cmake"

using nlohmann::json;

static MetaData meta;

static
bool QueryCodeModelTask(const std::string &build_directory, Task &task)
{
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
bool CMakeGenerateTask(
	const std::string &source_directory,
	const std::string &build_directory,
	const std::string &vcpkg_toolchain_file,
	const std::string &vcpkg_manifest_file,
	const std::vector<std::string> &options,
	Task &task
)
{
	std::function<bool()> fn = [source_directory, build_directory, vcpkg_toolchain_file, vcpkg_manifest_file, options]() {
		std::vector<std::string> args{
			CMAKE_COMMAND,
			"-S", source_directory,
			"-B", build_directory,
			"-DCMAKE_TOOLCHAIN_FILE=" + vcpkg_toolchain_file,
			"-DVCPKG_MANIFEST_DIR=" + vcpkg_manifest_file,
		};
		for (const std::string &option : options) {
			args.push_back("-D" + option);
		}
		RunCmdSync(CMAKE_COMMAND, args);
		return true;
	};
	task = Task(fn);

	return true;
}

static
bool CMakeResolveMetaDataTask(const std::string &build_directory, Task &task)
{
	std::function<bool()> fn = [build_directory]() ->bool {
		ReplyIndexV1 reply_index = ResolveReplyIndexFile(build_directory);
		CodemodelV2 codemodel_v2 = ResolveCodemodelFile(build_directory, reply_index);
		json targets = codemodel_v2["configurations"][0]["targets"];
		for (json::iterator it = targets.begin(); it != targets.end(); ++it) {
			std::string target_json_file = (*it)["jsonFile"].template get<std::string>();
			Target target = ResolveTargetFile(build_directory, target_json_file);
			meta.libs[target["name"]] = target;

			if (target["type"] == "EXECUTABLE")
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
bool CMakeBuildTask(
	const std::string &build_directory,
	const std::string &lib,
	const std::string &bin,
	Task &task
)
{
	std::function<bool()> fn = [build_directory, lib, bin]() {
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
			if (meta.bins.count(bin) == 0)
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
		
		RunCmdSync(CMAKE_COMMAND, args);

		return true;
	};

	task = Task(fn);

	return true;
}

static
bool RunTargetTask(const std::string &build_directory, const std::string &bin, Task &task)
{
	std::function<bool()> fn = [build_directory, bin]() {
		if (meta.bins.count(bin) == 0)
		{
			logger->Error(bin, " is not avaliable, the avaliable binaries are: [", meta.Bins(), "]");
			return false;
		}
		std::string binpath = build_directory + "/" + meta.bins[bin]["artifacts"][0]["path"].template get<std::string>();
		
		std::vector<std::string> args{ binpath };
		RunCmdSync(binpath, args);
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
	if (QueryCodeModelTask(config.build_directory, task))
	{
		tasks.AddTask(task);
	}
	// generate task
	if (CMakeGenerateTask(
		config.source_directory,
		config.build_directory,
		config.vcpkg_toochain_file,
		config.vcpkg_manifest_file,
		config.options,
		task))
	{
		tasks.AddTask(task);
	}
	// metadata
	if (CMakeResolveMetaDataTask(config.build_directory, task))
	{
		tasks.AddTask(task);
	}
	// build task
	if (CMakeBuildTask(config.build_directory, config.lib, config.bin, task))
	{
		tasks.AddTask(task);
	}

	tasks.Execute();
}

void CakeRun(const BuildConfig &build_config, const RunConfig &run_config)
{
	std::stringstream cmd;
	Tasks tasks;

	Task task;
	// generate query files
	if (QueryCodeModelTask(build_config.build_directory, task))
	{
		tasks.AddTask(task);
	}
	// generate task
	if (CMakeGenerateTask(
		build_config.source_directory,
		build_config.build_directory,
		build_config.vcpkg_toochain_file,
		build_config.vcpkg_manifest_file,
		build_config.options,
		task))
	{
		tasks.AddTask(task);
	}
	// metadata
	if (CMakeResolveMetaDataTask(build_config.build_directory, task))
	{
		tasks.AddTask(task);
	}
	// build task
	if (CMakeBuildTask(build_config.build_directory, build_config.lib, build_config.bin, task))
	{
		tasks.AddTask(task);
	}
	// run task
	if (RunTargetTask(build_config.build_directory, run_config.bin, task))
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
	} else if (strcmp(mode, "run") == 0) {
		cxxopts::Options options(
			"cake run",
			"Run a binary of the local package.");
		// clang-format off
		options.add_options()
		// target selection options
		("bin", "Run the specified binary", cxxopts::value<std::string>())
		("help", "Help");
		// clang-format on

		auto parse_result = options.parse(argc - 1, argv + 1);

		BuildConfig build_config;
		RunConfig run_config;
		if (parse_result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}
		if (parse_result.count("bin")) {
			run_config.bin = std::move(parse_result["bin"].as<std::string>());
			build_config.bin = std::move(parse_result["bin"].as<std::string>()); 
		}

		CakeRun(build_config, run_config);
	}

	return 0;
}
