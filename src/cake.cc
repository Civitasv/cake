#include "cake.h"

#include "manifest/manifest.h"
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
	bool vcpkg_support,
	const std::string &vcpkg_toolchain_file,
	const std::string &vcpkg_manifest_directory,
	const std::string &vcpkg_packages_directory,
	const std::vector<std::string> &options,
	Task &task
)
{
	std::function<bool()> fn = [source_directory, build_directory, vcpkg_support, vcpkg_toolchain_file, vcpkg_manifest_directory, vcpkg_packages_directory, options]() {
		std::vector<std::string> args {
			CMAKE_COMMAND,
			"-S", source_directory,
			"-B", build_directory
		};
		if (vcpkg_support) {
			args.push_back("-DCMAKE_TOOLCHAIN_FILE=" + vcpkg_toolchain_file);
			args.push_back("-DVCPKG_MANIFEST_DIR=" + vcpkg_manifest_directory);
			args.push_back("-DVCPKG_INSTALLED_DIR=" + vcpkg_packages_directory);
			args.push_back("-DVCPKG_MANIFEST_INSTALL=OFF"); // don't automatically install dependencies
		}
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
bool RunTargetTask(const std::string &build_directory, const std::string &bin, const std::vector<std::string> &bin_args, Task &task)
{
	std::function<bool()> fn = [build_directory, bin, bin_args]() {
		if (meta.bins.count(bin) == 0)
		{
			logger->Error(bin, " is not avaliable, the avaliable binaries are: [", meta.Bins(), "]");
			return false;
		}
		std::string binpath = build_directory + "/" + meta.bins[bin]["artifacts"][0]["path"].template get<std::string>();
		
		std::vector<std::string> args{ binpath };
		for (auto &arg: bin_args) {
			args.push_back(arg);
		}
		RunCmdSync(binpath, args);
		return true;
	};

	task = Task(fn);
	return true;
}

static
bool DebugTargetTask(const std::string &source_directory, const std::string &build_directory, const std::string &debugger, const std::string &bin, const std::vector<std::string> &bin_args, Task &task)
{
	std::function<bool()> fn = [source_directory, build_directory, debugger, bin, bin_args]() {
		if (meta.bins.count(bin) == 0)
		{
			logger->Error(bin, " is not avaliable, the avaliable binaries are: [", meta.Bins(), "]");
			return false;
		}
		std::string binpath = build_directory + "/" + meta.bins[bin]["artifacts"][0]["path"].template get<std::string>();

		std::vector<std::string> args;
		if (debugger == "gdb")
		{
			args.push_back(debugger);
			args.push_back("--args");
			args.push_back(binpath);

			for (auto &arg: bin_args) {
				args.push_back(arg);
			}
		} else if (debugger == "lldb")
		{
			args.push_back(debugger);
			args.push_back("--");
			args.push_back(binpath);

			for (auto &arg: bin_args) {
				args.push_back(arg);
			}
		} else if (debugger == "code")
		{
			args.push_back("code");
			args.push_back(source_directory);
			args.push_back("--profile");
			args.push_back("Cake");
		}

		RunCmdSync(debugger, args);
		return true;
	};

	task = Task(fn);
	return true;
}

static
bool VcpkgInstallLibraryTask(const std::string &port,
							 bool sync,
							 const std::vector<std::string> &options,
							 const std::string &vcpkg_manifest_directory,
							 const std::string &vcpkg_packages_directory,
							 Task &task)
{
	std::function<bool()> fn = [port, sync, options, vcpkg_manifest_directory, vcpkg_packages_directory]() {
		std::string vcpkg_path = "./packages/vcpkg/vcpkg";
		
		if (sync)
		{
			std::vector<std::string> args = {
				vcpkg_path,
				"install",
				"--x-manifest-root=" + vcpkg_manifest_directory,
				"--x-install-root=" + vcpkg_packages_directory
			};
			for (auto &option: options)
			{
				args.push_back(option);
			}
			RunCmdSync(vcpkg_path, args);
		} else {
			std::vector<std::string> args = {
				vcpkg_path,
				"add",
				"port",
				port,
				"--x-manifest-root=" + vcpkg_manifest_directory
			};
			for (auto &option: options)
			{
				args.push_back(option);
			}
			RunCmdSync(vcpkg_path, args);
		}

		return true;
	};

	task = Task(fn);
	return true;
}

static
bool TemplateCreateTask(const std::string &type, const std::string &name, const std::string &template_basic_directory, const std::string &template_vcpkg_directory, Task &task)
{
	std::function<bool()> fn = [type, name, template_basic_directory, template_vcpkg_directory]() {
		std::string project_name = name.empty() ? type : name;

		std::string temp_project_name = "temp_cake";
		RunCmdSync("mkdir", { "mkdir", temp_project_name });
		RunCmdSync("git", { "git", "init", temp_project_name });
		RunCmdSync("git", { "git", "-C", temp_project_name, "remote", "add", "origin", "https://github.com/Civitasv/cake"});
		RunCmdSync("git", { "git", "-C", temp_project_name, "config", "core.sparseCheckout", "true" }); 
		WriteContentToFile("template/" + type, "./" + temp_project_name + "/.git/info/sparse-checkout");
		RunCmdSync("git", { "git", "-C", temp_project_name, "pull", "origin", "main" });

		RunCmdSync("mv", { "mv", temp_project_name + "/template/" + type, "./" + project_name });
		RunCmdSync("rm", { "rm", "-rf", temp_project_name });

		RunCmdSync("git", { "git", "init", project_name });
		if (type == "vcpkg")
		{
			RunCmdSync("git", { "git", "-C", project_name, "submodule", "add", "git@github.com:microsoft/vcpkg", "./packages/vcpkg" });
			RunCmdSync("sh", { "sh", "./" + project_name + "/packages/vcpkg/bootstrap-vcpkg.sh" });
			RunCmdSync("./" + project_name + "/packages/vcpkg/vcpkg", { "./" + project_name +"/packages/vcpkg/vcpkg", "x-update-baseline", "--add-initial-baseline" });
		}

		return true;
	};

	task = Task(fn);
	return true;
}

static
bool DocsCreateTask(Task &task)
{
	std::function<bool()> fn = []() {
		RunCmdSync("doxygen", { "doxygen", "Doxyfile" });
		return true;
	};

	task = Task(fn);
	return true;
}

void CakeBuild(const BuildConfig &config)
{
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
		config.vcpkg_support,
		config.vcpkg_toochain_file,
		config.vcpkg_manifest_directory,
		config.vcpkg_packages_directory,
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
	Tasks tasks;

	Task task;
	// metadata
	if (CMakeResolveMetaDataTask(build_config.build_directory, task))
	{
		tasks.AddTask(task);
	}
	// run task
	if (RunTargetTask(build_config.build_directory, run_config.bin, run_config.args, task))
	{
		tasks.AddTask(task);
	}

	tasks.Execute();
}

void CakeDebug(const BuildConfig &build_config, const DebugConfig &debug_config)
{
	Tasks tasks;

	Task task;
	// metadata
	if (CMakeResolveMetaDataTask(build_config.build_directory, task))
	{
		tasks.AddTask(task);
	}
	// debug task
	if (DebugTargetTask(build_config.source_directory, build_config.build_directory, debug_config.debugger, debug_config.bin, debug_config.args, task))
	{
		tasks.AddTask(task);
	}

	tasks.Execute();
}

void CakeInstall(const InstallConfig &install_config)
{
	if (!install_config.vcpkg_support)
	{
		logger->Error("You should open vcpkg support");
	}

	Tasks tasks;

	Task task;
	// install task
	if (VcpkgInstallLibraryTask(install_config.port, install_config.sync, install_config.options, install_config.vcpkg_manifest_directory, install_config.vcpkg_packages_directory, task))
	{
		tasks.AddTask(task);
	}

	tasks.Execute();
}

void CakeCreate(const CreateConfig &create_config)
{
	std::stringstream cmd;
	Tasks tasks;

	Task task;
	// install task
	if (TemplateCreateTask(create_config.type, create_config.name, create_config.template_basic_directory, create_config.template_vcpkg_directory, task))
	{
		tasks.AddTask(task);
	}

	tasks.Execute();
}

void CakeDocs()
{
	std::stringstream cmd;
	Tasks tasks;

	Task task;
	if (DocsCreateTask(task))
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
		printf("  cake [build|run|debug|install|create|docs] [OPTION...]");
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
		("vcpkg", "Whether support vcpkg", cxxopts::value<bool>())
		("config", "Set configuration value", cxxopts::value<std::vector<std::string>>())
		("help", "Help");
		// clang-format on

		auto parse_result = options.parse(argc - 1, argv + 1);

		BuildConfig config = ParseBuildConfigFromManifest();
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
		if (parse_result.count("vcpkg")) {
			config.vcpkg_support = parse_result["vcpkg"].as<bool>();
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
		("args", "Args passed to binary", cxxopts::value<std::vector<std::string>>())
		("help", "Help");
		// clang-format on

		auto parse_result = options.parse(argc - 1, argv + 1);

		BuildConfig build_config = ParseBuildConfigFromManifest();
		RunConfig run_config = ParseRunConfigFromManifest();
		if (parse_result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}
		if (parse_result.count("bin")) {
			run_config.bin = std::move(parse_result["bin"].as<std::string>());
		}
		if (parse_result.count("args")) {
			run_config.args = std::move(parse_result["args"].as<std::vector<std::string>>());
		}

		CakeRun(build_config, run_config);
	} else if (strcmp(mode, "debug") == 0) {
		cxxopts::Options options(
			"cake debug",
			"Debug a binary of the local package.");
		// clang-format off
		options.add_options()
		// target selection options
		("debugger", "Specify the debugger", cxxopts::value<std::string>())
		("bin", "Debug the specified binary", cxxopts::value<std::string>())
		("args", "Args passed to binary", cxxopts::value<std::vector<std::string>>())
		("help", "Help");
		// clang-format on

		auto parse_result = options.parse(argc - 1, argv + 1);

		BuildConfig build_config = ParseBuildConfigFromManifest();
		DebugConfig debug_config = ParseDebugConfigFromManifest();
		if (parse_result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}
		if (parse_result.count("debugger")) {
			debug_config.debugger = std::move(parse_result["debugger"].as<std::string>());
		}
		if (parse_result.count("bin")) {
			debug_config.bin = std::move(parse_result["bin"].as<std::string>());
		}
		if (parse_result.count("args")) {
			debug_config.args = std::move(parse_result["args"].as<std::vector<std::string>>());
		}

		CakeDebug(build_config, debug_config);
	}else if (strcmp(mode, "install") == 0) {
		cxxopts::Options options(
			"cake install",
			"Install a library using vcpkg.");
		// clang-format off
		options.add_options()
		("port", "Add port to vcpkg.json", cxxopts::value<std::string>())
		("sync", "Install all libraries in vcpkg.json")
		// common options
		("config", "Set configuration value", cxxopts::value<std::vector<std::string>>())
		("help", "Help");
		// clang-format on

		auto parse_result = options.parse(argc - 1, argv + 1);

		InstallConfig install_config = ParseInstallConfigFromManifest();
		if (parse_result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}
		if (parse_result.count("port")) {
			install_config.port = std::move(parse_result["port"].as<std::string>());
		}
		if (parse_result.count("sync")) {
			install_config.sync = true;
		}
		if (parse_result.count("config")) {
			install_config.options = std::move(parse_result["config"].as<std::vector<std::string>>());
		}

		CakeInstall(install_config);
	} else if (strcmp(mode, "create") == 0) {
		cxxopts::Options options(
			"cake create",
			"Create a template");
		// clang-format off
		options.add_options()
		("template", "Specify template type", cxxopts::value<std::string>())
		("name", "Specify project name", cxxopts::value<std::string>())
		// common options
		("config", "Set configuration value", cxxopts::value<std::vector<std::string>>())
		("help", "Help");
		// clang-format on

		auto parse_result = options.parse(argc - 1, argv + 1);

		CreateConfig create_config;
		if (parse_result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}
		if (parse_result.count("template")) {
			create_config.type = std::move(parse_result["template"].as<std::string>());
		}
		if (parse_result.count("name")) {
			create_config.name = std::move(parse_result["name"].as<std::string>());
		}

		CakeCreate(create_config);
	} else if (strcmp(mode, "docs") == 0) {
		cxxopts::Options options(
			"cake docs",
			"Create docs");
		// clang-format off
		options.add_options()
		("help", "Help");
		// clang-format on

		auto parse_result = options.parse(argc - 1, argv + 1);

		if (parse_result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}

		CakeDocs();
	}

	return 0;
}
