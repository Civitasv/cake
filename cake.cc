#include "cake.hpp"
#include "cxxopts.hpp"

#include <cstdio>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

#define CMAKE_COMMAND "cmake"

namespace {
std::string make_a_cmd(const std::string &cmd,
                       const std::vector<std::string> &args) {
  std::stringstream ss;
  ss << cmd;
  for (auto &arg : args) {
    ss << " ";
    ss << arg;
  }

  return ss.str();
}

void execute_a_cmd(const char *cmd) {
  printf("Executing [%s]\n", cmd);
  FILE *fd = popen(cmd, "r");

  char content[1024];
  while (fgets(content, 1024, fd)) {
    printf("%s", content);
  }
  pclose(fd);
}
} // namespace

bool ChangeCwdTask(const std::string &cwd, Task &task) {
  if (cwd == ".") {
    return false;
  }
  task = Task(make_a_cmd("cd", {cwd}));

  return true;
}

bool CMakeGenerateTask(const std::string &source_dir,
                       const std::string &build_dir,
                       const std::vector<std::string> options, Task &task) {
  std::vector<std::string> args{"-S", source_dir, "-B", build_dir};
  for (auto &option : options) {
    args.emplace_back("-D" + option);
  }
  task = Task(make_a_cmd(CMAKE_COMMAND, args));

  return true;
}

bool CMakeBuildTask(const std::string &build_dir,
                    const std::vector<std::string> options, Task &task) {
  std::vector<std::string> args{"--build", build_dir};
  for (auto &option : options) {
    args.emplace_back("-D" + option);
  }
  task = Task(make_a_cmd(CMAKE_COMMAND, args));

  return true;
}

void CMakeGenerate(const Config &config) {
  std::stringstream cmd;
  Tasks tasks;

  Task task;
  // cwd task
  if (ChangeCwdTask(config.source_directory, task)) {
    tasks.AddTask(task);
  }
  // generate task
  if (CMakeGenerateTask(config.source_directory, config.build_directory,
                        config.options, task)) {
    tasks.AddTask(task);
  }

  tasks.Execute();
}

void CMakeBuild(const Config &config) {
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

void Task::Execute() {
  status = kProgress;

  execute_a_cmd(cmd.c_str());

  status = kSuccess;
}

std::ostream &operator<<(std::ostream &os, const Tasks &tasks) {
  os << "Printing all tasks >>> " << std::endl;
  for (auto &task : tasks.tasks) {
    os << '\t' << task.cmd << '\n';
  }
  return os;
}

void Tasks::AddTask(const Task &task) { tasks.push_back(task); }

void Tasks::Execute() {
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

int main(int argc, char **argv) {
  cxxopts::Options options("cake", "Make cmake simpler");
  // clang-format off
  options.add_options()
    ("c,cwd", "Current working directory", cxxopts::value<std::string>())
    ("g,generate", "Generate")
    ("b,build", "Build")
    ("o,options", "Options passed to CMake", cxxopts::value<std::vector<std::string>>())
    ("h,help", "Help");
  // clang-format on

  auto parse_result = options.parse(argc, argv);

  Config config;
  if (parse_result.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }
  if (parse_result.count("cwd")) {
    config.source_directory = std::move(parse_result["cwd"].as<std::string>());
  }

  if (parse_result.count("options")) {
    config.options =
        std::move(parse_result["options"].as<std::vector<std::string>>());
  }

  if (parse_result.count("generate")) {
    CMakeGenerate(config);
    return 0;
  }

  if (parse_result.count("build")) {
    CMakeBuild(config);
    return 0;
  }

  return 0;
}
