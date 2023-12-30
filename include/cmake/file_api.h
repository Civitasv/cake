#ifndef CAKE_FILE_API_H_
#define CAKE_FILE_API_H_

#include <string>
#include "utility/json.h"

#define CMAKE_FILE_API ".cmake/api/v1"
#define QUERY "query"
#define REPLY "reply"

#define CODEMODEL_FILE "codemodel-v2"

using ReplyIndexV1 = nlohmann::json;
using CodemodelV2 = nlohmann::json;
using Target = nlohmann::json;

bool MakeQueryCodeModelFile(const std::string &build_directory);

/// Read the `index-*.json`.
ReplyIndexV1 ResolveReplyIndexFile(const std::string &build_directory);

/// Read the `codemodel-v2-*.json`
CodemodelV2 ResolveCodemodelFile(const std::string &build_directory, const ReplyIndexV1& reply_index);

/// Read the `target-*.json`
Target ResolveTargetFile(const std::string &build_directory, const std::string &target_json_file);

#endif // CAKE_FILE_API_H_
