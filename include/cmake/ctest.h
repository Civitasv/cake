#ifndef CAKE_CTEST_H_
#define CAKE_CTEST_H_

#include <string>
#include <vector>
#include "utility/json.h"

std::vector<std::string> ListAllTests(const std::string &build_directory);

#endif  // CAKE_CTEST_H_
