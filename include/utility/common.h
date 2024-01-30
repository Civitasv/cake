#ifndef CAKE_HELPER_H_
#define CAKE_HELPER_H_

#include <functional>
#include <string>

#include "log/log.h"

extern std::shared_ptr<Logger> logger;

enum class Status { kProgress = 0, kSuccess, kFail };

/// A task is just a function.
struct Task {
	Task()
	{
	}
	Task(std::function<bool()> the_function)
		: the_function(the_function)
	{
	}

	Status status; ///< task status
	
	std::function<bool()> the_function; ///< the function need to be executed
	
	/// Execute a Task
	void Execute();
};

/// Tasks are array of task.
struct Tasks {
	Status status; ///< tasks status
	std::vector<Task> tasks; ///< a series of task

	void AddTask(const Task &task);
	void Execute();
};

/// Make a new process run a cmd, synchonized.
bool RunCmdSync(const std::string &cmd, const std::vector<std::string> &args);

/// Make a new directory, if not exists.
bool MakeDirectory(std::string path);

/// Make an empty file, if not exists.
bool MakeFile(std::string path);

/// Write content to file.
bool WriteContentToFile(const std::string &content, const std::string &file);

/// Whether file exists.
bool FileExists(const std::string& file);

#endif // CAKE_HELPER_H_

