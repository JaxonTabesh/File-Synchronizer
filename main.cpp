#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

bool printDirectoryRecursively(std::filesystem::path directory);
std::string stripTopLevelDirFromRelPath(std::filesystem::directory_entry dirEntry);
void printComparator(std::filesystem::path srcDir, std::filesystem::path targetDir);

int main() {
	// Source and target paths

	std::filesystem::path dir = "./source/";
	std::filesystem::path targetDir = "./target/";

	// Prints what the current path is

	std::cout << "Current Path: " << std::filesystem::current_path().generic_string() << std::endl;

	// Run the functions to print the dirs' contents

	printDirectoryRecursively(dir);
	printDirectoryRecursively(targetDir);

	while (true) {
		auto now = std::chrono::system_clock::now();
		std::cout << std::format("{:%H:%M:%S}", floor<std::chrono::seconds>(now)) << '\n';

		printComparator(dir, targetDir);
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::system("cls");
	}
	return 0;
}

bool printDirectoryRecursively(std::filesystem::path dir) {
	// Error checking
	if (!std::filesystem::exists(dir)) {
		std::cerr << "Path doesn't exist: " << dir << '\n';
		return false;
	}
	// Prints the regular files recursively
	std::cout << "Files in " << dir.relative_path().generic_string() << ":" << '\n';
	for (auto const& dirEntry : std::filesystem::recursive_directory_iterator{ dir }) {
		if (dirEntry.is_regular_file()) {
			std::cout << " - " << dirEntry.path().generic_string() << "\n"; // generic_string() shows "/" instead of string() which shows " to stay consistent
			std::cout << "\tSize: " << dirEntry.file_size() << " bytes" << ", Last Modified: " << dirEntry.last_write_time() << "\n"; // Prints the associated metadata
		}
	}
	return true;
}

void printComparator(std::filesystem::path srcDir, std::filesystem::path targetDir) {
	std::set<std::string> srcDirSet = {};
	std::set<std::string> tgtDirSet = {};
	// Use an unordered map for keys as paths and metadata as values for comparison
	std::unordered_map<std::string, std::pair<std::uintmax_t, std::filesystem::file_time_type>> srcMetadata = {};
	std::vector<std::string> pathsToCopy = {}; // Paths that exist in src but
	// not target or paths with differing metadata in src and target
	std::vector<std::string> pathsToDelete = {}; // Files in target but not in src
	// Insert the srcDirSet with the srcDir files
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(srcDir)) {
		if (dirEntry.is_regular_file()) {
			srcDirSet.insert(stripTopLevelDirFromRelPath(dirEntry));
			// Pair for the file_size and last write time, respectively
			std::pair<std::uintmax_t, std::filesystem::file_time_type> fsLwt = { dirEntry.file_size(), dirEntry.last_write_time() };
			// Add the dirEntry to map with the Pair of relevant metadata
			std::regex pattern("^./(.*?)/");
			std::string path = stripTopLevelDirFromRelPath(dirEntry);
			path = std::regex_replace(path, pattern, "");
			srcMetadata[stripTopLevelDirFromRelPath(dirEntry)] = fsLwt;
		}
	}
	// use the stripping function instead of dirEntry's
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(targetDir)) {
		if (dirEntry.is_regular_file()) {
			std::pair<std::uintmax_t, std::filesystem::file_time_type> fsLwt = { dirEntry.file_size(), dirEntry.last_write_time() };
			if (srcDirSet.contains(stripTopLevelDirFromRelPath(dirEntry)) && srcMetadata[stripTopLevelDirFromRelPath(dirEntry)] != fsLwt) {
				pathsToCopy.push_back(stripTopLevelDirFromRelPath(dirEntry));
			}
			else if (!srcDirSet.contains(stripTopLevelDirFromRelPath(dirEntry))) {
				pathsToDelete.push_back(stripTopLevelDirFromRelPath(dirEntry));
			}
			// else if it contains dirEntry and has equivalent metadata then no need to do anything
		}
	}

	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(targetDir)) {
		if (dirEntry.is_regular_file()) {
			tgtDirSet.insert(stripTopLevelDirFromRelPath(dirEntry));
		}
	}

	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(srcDir)) {
		if (dirEntry.is_regular_file()) {
			if (!tgtDirSet.contains(stripTopLevelDirFromRelPath(dirEntry))) {
				pathsToCopy.push_back(stripTopLevelDirFromRelPath(dirEntry));
			}
		}
	}

	// Print the collections
	std::cout << "srcDirSet:" << '\n';
	for (const auto& dir : srcDirSet) {
		std::cout << "\t" << dir << '\n';
	}
	std::cout << "tgtDirSet:" << '\n';
	for (const auto& dir : tgtDirSet) {
		std::cout << "\t" << dir << '\n';
	}
	std::cout << "pathsToCopy:" << '\n';
	for (const auto& path : pathsToCopy) {
		std::cout << "\t" << path << '\n';
	}
	std::cout << "pathsToDelete:" << '\n';
	for (const auto& path : pathsToDelete) {
		std::cout << "\t" << path << '\n';
	}
}

std::string stripTopLevelDirFromRelPath(std::filesystem::directory_entry dirEntry) {
	std::string dirEntryString = dirEntry.path().generic_string();
	std::regex pattern("^./(.*?)/");
	std::string path = std::regex_replace(dirEntryString, pattern, "/");
	// TODO: add error checking for the regex in case the format is not what is expected
	return path;
}