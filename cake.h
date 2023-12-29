// -*- C++ -*-
//
// cake.h - v0.01 - public domain - Civitasv, 2023
//
// USAGE
//
// In *ONE* source file, put:
//
//     #define CMAKE_IMPLEMENTATION
//     #include "cake.h"
//
// Other source files should just include cake.h
//

#ifndef CAKE_H_
#define CAKE_H_

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

/// ====================== APIs ==========================

struct ExecutorConfig {
	std::string source_directory = "."; ///< current working directory
	std::string build_directory =
		"out"; ///< where do you want to store the build files
	std::string lib;
	std::string bin;
	std::vector<std::string> options;
};

enum Status { kProgress = 0, kSuccess, kFail };

struct Task {
	Task()
	{
	}
	Task(const std::string &cmd)
		: cmd(cmd)
	{
	}
	Task(const std::string &cmd, const std::vector<std::string> &args)
		: cmd(cmd)
		, args(args)
	{
	}

	Status status; ///< task status
	std::string cmd; ///< which cmd to execute
	std::vector<std::string> args; // arguments

	void Execute();
};

struct Tasks {
	Status status; ///< tasks status
	std::vector<Task> tasks; ///< a series of task

	friend std::ostream &operator<<(std::ostream &os, const Tasks &tasks);
	void AddTask(const Task &task);
	void Execute();
};

/// Used to generate specified build system.
void CMakeGenerate(const ExecutorConfig &config);

#endif // CAKE_H_
