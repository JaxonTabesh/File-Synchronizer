#pragma once

#include "DirectoryState.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class FileSyncManager {
private:
	DirectoryState sourcePath;
	DirectoryState targetPath;
	std::vector<std::filesystem::path> filesToCopy; // Paths that exist in source but
	// not synced or paths with differing metadata in source and synced
	std::vector<std::filesystem::path> filesToDelete; // Files in synced but not in source
	// Dirs to create/delete in synced
	std::vector<std::filesystem::path> dirsToCopy;
	std::vector<std::filesystem::path> dirsToDelete;

	std::filesystem::path stripOuterDirectory(std::filesystem::path dirEntry, bool isSourceEntry);
	void updateCollections();
	void fillFilesToCopyDelete();
	void fillDirCopyDeleteSets();
	void printCollections();

public:
	FileSyncManager(std::filesystem::path source, std::filesystem::path synced);
	void beginOneWaySync(bool dryRun = false);
	void beginOneWayComparing();
	static void oneTimeBackup(std::filesystem::path source, std::filesystem::path backup, bool dryRun = false);
	static void printDirectoryRecursively(std::filesystem::path path);
};