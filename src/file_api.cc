#include <fstream>
#include <regex>

#include "common.h"
#include "file_api.h"

bool MakeQueryCodeModelFile(const std::string &build_directory)
{
	std::string dir = build_directory + "/" + CMAKE_FILE_API + "/" + QUERY;
	if (MakeDirectory(dir)) {
		if (MakeFile(dir + "/" + CODEMODEL_FILE)) {
			return true;
		}
	}
	return false;
}

static
std::vector<std::string> FindfilesMatchPattern(const std::filesystem::path &dir, const std::regex &file_pattern)
{
	std::vector<std::string> files;
	for (const auto &entry : std::filesystem::directory_iterator(dir)) {
		if (std::regex_match(entry.path().filename().string(), file_pattern)) {
			files.push_back(entry.path().string());
		}
	}
	
	return files;
}

ReplyIndexV1 ResolveReplyIndexFile(const std::string &build_directory)
{
	using nlohmann::json;

	std::regex file_pattern("(index-[0-9a-zA-Z-]+.json)");
	std::filesystem::path dir =
		build_directory + "/" + CMAKE_FILE_API + "/" + REPLY + "/";

	std::vector<std::string> files = FindfilesMatchPattern(dir, file_pattern);
	std::sort(files.begin(), files.end());

	// we select the last one
	std::string the_file = files[files.size() - 1];

	return json::parse(std::ifstream(the_file));
}


CodemodelV2 ResolveCodemodelFile(const std::string &build_directory, const ReplyIndexV1 &reply_index)
{
	using nlohmann::json;

	auto reply = reply_index["reply"];
	auto codemodel_v2 = reply["codemodel-v2"];
	auto jsonfile = codemodel_v2["jsonFile"].template get<std::string>();

	std::string dir =
		build_directory + "/" + CMAKE_FILE_API + "/" + REPLY + "/";
	return json::parse(std::ifstream(dir + jsonfile));
}

Target ResolveTargetFile(const std::string &build_directory, const std::string &target_json_file)
{
	using nlohmann::json;

	std::string filepath =
		build_directory + "/" + CMAKE_FILE_API + "/" + REPLY + "/" + target_json_file;
	return json::parse(std::ifstream(filepath));
}
