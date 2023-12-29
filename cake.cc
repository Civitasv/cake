#include "cake.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <sys/_types/_pid_t.h>
#include <unistd.h>
#include <vector>
#include "log.h"
#include "file_api.h"

#include "cxxopts.hpp"

#define CMAKE_COMMAND "cmake"

namespace
{
auto logger = Logger::Create();

char **string_vector_to_char_array(const std::vector<std::string> &vec)
{
	char **result = nullptr;
	int count = vec.size();

	result = new char *[count];
	for (int i = 0; i < count; ++i) {
		result[i] = new char[vec[i].length() +
				     1]; // +1 for the null-terminator
		strcpy(result[i], vec[i].c_str());
	}

	return result;
}

// copied from https://github.com/tsoding/nobuild/blob/a2924b01373220bf0656b86e3dac21638df08c85/nobuild.h#L647
void pid_wait(pid_t pid)
{
	for (;;) {
		int wstatus = 0;
		if (waitpid(pid, &wstatus, 0) < 0) {
			logger->Error("could not wait on command (pid ", pid,
				      "): ", strerror(errno));
		}

		if (WIFEXITED(wstatus)) {
			int exit_status = WEXITSTATUS(wstatus);
			if (exit_status != 0) {
				logger->Error("command exited with exit code ",
					      exit_status);
			}

			break;
		}

		if (WIFSIGNALED(wstatus)) {
			logger->Error("command process was terminated by ",
				      strsignal(WTERMSIG(wstatus)));
		}
	}
}
} // namespace

bool ChangeCwdTask(const std::string &cwd, Task &task)
{
	if (cwd == ".") {
		return false;
	}

	task = Task("cd", { "cd", cwd });

	return true;
}

bool QueryCodeModelTask(const std::string &build_directory, Task &task)
{
	QueryCodeModel(build_directory);

	return true;
}

bool CMakeGenerateTask(const std::string &source_dir,
		       const std::string &build_dir,
		       const std::vector<std::string> options, Task &task)
{
	std::vector<std::string> args{ CMAKE_COMMAND, "-S", source_dir, "-B",
				       build_dir };
	for (auto &option : options) {
		args.emplace_back("-D" + option);
	}
	task = Task(CMAKE_COMMAND, args);

	return true;
}

bool CMakeBuildTask(const std::string &build_dir,
		    const std::vector<std::string> options, Task &task)
{
	std::vector<std::string> args{ CMAKE_COMMAND, "--build", build_dir };
	for (auto &option : options) {
		args.emplace_back("-D" + option);
	}
	task = Task(CMAKE_COMMAND, args);

	return true;
}

void CMakeGenerate(const ExecutorConfig &config)
{
	std::stringstream cmd;
	Tasks tasks;

	Task task;
	// cwd task
	if (ChangeCwdTask(config.source_directory, task)) {
		tasks.AddTask(task);
	}
	// generate query files
	if (QueryCodeModelTask(config.build_directory, task)) {
		tasks.AddTask(task);
	}
	// generate task
	if (CMakeGenerateTask(config.source_directory, config.build_directory,
			      config.options, task)) {
		tasks.AddTask(task);
	}

	tasks.Execute();
}

void CakeBuild(const ExecutorConfig &config)
{
	std::stringstream cmd;
	Tasks tasks;

	Task task;
	// cwd task
	if (ChangeCwdTask(config.source_directory, task)) {
		tasks.AddTask(task);
	}
	// build task
	if (CMakeBuildTask(config.build_directory, config.options, task)) {
		tasks.AddTask(task);
	}

	tasks.Execute();
}

void Task::Execute()
{
	status = kProgress;

	logger->Debug("Executing ", '"', args, '"');

	// fork a new process and execute it
	pid_t c_pid = fork();
	if (c_pid < 0) {
		logger->Error("Could not fork a child process: ", cmd, " ",
			      args, " ", " -> ", strerror(errno));
	}
	if (c_pid == 0) { // child process
		if (execvp(cmd.c_str(), string_vector_to_char_array(args)) <
		    0) {
			logger->Error("Could not exec child process: ", cmd,
				      " ", args, " ||| ", strerror(errno));
		}
	}

	// wating for child process
	pid_wait(c_pid);

	status = kSuccess;
}

std::ostream &operator<<(std::ostream &os, const Tasks &tasks)
{
	os << "Printing all tasks >>> " << std::endl;
	for (auto &task : tasks.tasks) {
		os << '\t' << task.cmd << '\n';
	}
	return os;
}

void Tasks::AddTask(const Task &task)
{
	tasks.push_back(task);
}

void Tasks::Execute()
{
	status = kProgress;
	for (Task &task : tasks) {
		task.Execute();
		if (task.status != kSuccess) {
			status = kFail;
			return;
		}
	}
	status = kSuccess;
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

		ExecutorConfig config;
		if (parse_result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}
		if (parse_result.count("config")) {
			config.options = std::move(
				parse_result["config"]
					.as<std::vector<std::string> >());
		}
		if (parse_result.count("lib")) {
			config.lib = std::move(
				parse_result["lib"].as<std::string>());
		}
		if (parse_result.count("bin")) {
			config.bin = std::move(
				parse_result["bin"].as<std::string>());
		}

		CMakeGenerate(config);
		CakeBuild(config);
	} else if (strcmp(mode, "run")) {
	}

	return 0;
}
