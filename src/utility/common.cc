#include <unistd.h>

#include "utility/common.h"

#include <fstream>
#include <stdio.h>
#include <sys/stat.h>

auto logger = Logger::Create();

static
char **string_vector_to_char_array(const std::vector<std::string> &vec)
{
	char **result = nullptr;
	int count = vec.size();

	result = new char *[count + 1];
	for (int i = 0; i < count; ++i) {
		result[i] = new char[vec[i].length() + 1]; // +1 for the null-terminator
		strcpy(result[i], vec[i].c_str());
	}

	result[count] = (char *)0; //< it should be null-terminator

	return result;
}

// copied from https://github.com/tsoding/nobuild/blob/a2924b01373220bf0656b86e3dac21638df08c85/nobuild.h#L647
static
void pid_wait(pid_t pid)
{
	for (;;) {
		int wstatus = 0;
		if (waitpid(pid, &wstatus, 0) < 0) {
			logger->Error("could not wait on command (pid ", pid, "): ", strerror(errno));
		}

		if (WIFEXITED(wstatus)) {
			int exit_status = WEXITSTATUS(wstatus);
			if (exit_status != 0) {
				logger->Error("command exited with exit code ", exit_status);
			}

			break;
		}

		if (WIFSIGNALED(wstatus)) {
			logger->Error("command process was terminated by ",
				      strsignal(WTERMSIG(wstatus)));
		}
	}
}

////////////////////// Task /////////////////////////////////

void Task::Execute()
{
	status = Status::kProgress;

	if (the_function)
	{
		if (!the_function())
		{
			status = Status::kFail;
			return;
		}
	}

	status = Status::kSuccess;
}


////////////////////// Tasks /////////////////////////////////

void Tasks::AddTask(const Task &task)
{
	tasks.push_back(task);
}

void Tasks::Execute()
{
	status = Status::kProgress;
	for (Task &task : tasks) {
		task.Execute();
		if (task.status != Status::kSuccess) {
			status = Status::kFail;
			return;
		}
	}
	status = Status::kSuccess;
}

////////////////////// Others /////////////////////////////////
bool RunCmdSync(const std::string &cmd, const std::vector<std::string> &args)
{
	logger->Debug("Executing ", '"', args, '"');

	// fork a new process and execute it
	pid_t c_pid = fork();
	if (c_pid < 0) {
		logger->Error(
			"Could not fork a child process: ",
			args,
			" ",
			" -> ",
			strerror(errno)
		);
	}
	if (c_pid == 0) { // child process
		if (execvp(cmd.c_str(), string_vector_to_char_array(args)) == -1) {
			logger->Error(
				"Could not exec child process: ",
				args,
				" ||| ",
				strerror(errno)
			);
		}
	}

	// wating for child process
	pid_wait(c_pid);

	return true;
}

static bool do_mkdir(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
            return false;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        return false;
    }
    return true;
}

bool MakeDirectory(std::string path) {
	std::string build;
	for (size_t pos = 0; (pos = path.find('/')) != std::string::npos;) {
		build += path.substr(0, pos + 1);
		do_mkdir(build);
		path.erase(0, pos + 1);
	}
	if (!path.empty()) {
		build += path;
		do_mkdir(build);
	}
	return true;
}

bool MakeFile(std::string path) {
	std::ofstream output(path);

	output.close();
	return true;
}
