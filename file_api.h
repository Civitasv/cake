#ifndef CAKE_FILE_API_H_
#define CAKE_FILE_API_H_

#include <string>
#define CMAKE_FILE_API ".cmake/api"
#define QUERY "query"
#define REPLY "reply"

#define QUERY_OPTION "codemodel-v2"

void QueryCodeModel(const std::string &build_directory);

Json RetrieveTargets(const std::string &build_directory);
#endif // CAKE_FILE_API_H_
