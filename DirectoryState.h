#pragma once

#include <algorithm>
#include <filesystem>
#include <set>
#include <unordered_map>

struct FileInfo
{
	std::uintmax_t size;
	std::filesystem::file_time_type lastWriteTime;
	bool operator==(const FileInfo& other) const;

	FileInfo(std::uintmax_t s = 0, std::filesystem::file_time_type lwt = {}) : size(s), lastWriteTime(lwt) {}
};

struct DirectoryState
{
	std::filesystem::path absPath;
	std::set<std::filesystem::path> files;
	std::set<std::filesystem::path> dirs;
	std::unordered_map<std::filesystem::path, FileInfo> metadata;

	DirectoryState(const std::filesystem::path path);
	void scan();
};