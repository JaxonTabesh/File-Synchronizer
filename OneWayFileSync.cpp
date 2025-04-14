#include <filesystem>
#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>

class OneWayFileSync {
private:
	std::filesystem::path sourcePath;
	std::filesystem::path backupPath;
	std::set<std::string> sourceFilesSet = {};
	std::set<std::string> backupFilesSet = {};
	std::set<std::string> sourceDirsSet = {};
	std::set<std::string> backupDirsSet = {};
	// Use an unordered map for keys as paths and metadata as values for comparison
	std::unordered_map<std::string, std::pair<std::uintmax_t, std::filesystem::file_time_type>> sourceMetadata = {};
	std::vector<std::string> filesToCopy = {}; // Paths that exist in source but
	// not backup or paths with differing metadata in source and backup
	std::vector<std::string> filesToDelete = {}; // Files in backup but not in source
	// Dirs to create/delete in backup
	std::vector<std::string> dirsToCopy = {};
	std::vector<std::string> dirsToDelete = {};

	// stripTopLevelDirectoryFromRelativePath
	static std::string stripTopLvlDirFrmRelPath(std::filesystem::directory_entry dirEntry) {
		std::string dirEntryString = dirEntry.path().generic_string();
		std::regex pattern("^./(.*?)/");
		std::string path = std::regex_replace(dirEntryString, pattern, "/");
		// TODO: add error checking for the regex in case the format is not what is expected
		return path;
	}
	void clearCollections() {
		sourceFilesSet.clear();
		backupFilesSet.clear();
		sourceDirsSet.clear();
		backupDirsSet.clear();
		sourceMetadata.clear();
		filesToCopy.clear();
		filesToDelete.clear();
		dirsToCopy.clear();
		dirsToDelete.clear();
	}
public:
	// Constructor requires a source directory and a backup directory in either string or std::filesystem::path
	OneWayFileSync(std::filesystem::path source, std::filesystem::path backup) : sourcePath(source), backupPath(backup) {
	}
	void fillFilesToCopyDelete() {
		// use the stripping function instead of dirEntry's
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(backupPath)) {
			if (dirEntry.is_regular_file()) {
				std::pair<std::uintmax_t, std::filesystem::file_time_type> fsLwt = { dirEntry.file_size(), dirEntry.last_write_time() };
				if (sourceFilesSet.contains(stripTopLvlDirFrmRelPath(dirEntry)) && sourceMetadata[stripTopLvlDirFrmRelPath(dirEntry)] != fsLwt) {
					filesToCopy.push_back(stripTopLvlDirFrmRelPath(dirEntry));
				}
				else if (!sourceFilesSet.contains(stripTopLvlDirFrmRelPath(dirEntry))) {
					filesToDelete.push_back(stripTopLvlDirFrmRelPath(dirEntry));
				}
				// else if it contains dirEntry and has equivalent metadata then no need to do anything
			}
		}
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath)) {
			if (dirEntry.is_regular_file()) {
				if (!backupFilesSet.contains(stripTopLvlDirFrmRelPath(dirEntry))) {
					filesToCopy.push_back(stripTopLvlDirFrmRelPath(dirEntry));
				}
			}
		}

	}
	void fillSourceMetadata() {
		// Insert the sourceFilesSet with the sourceDir files
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath)) {
			if (dirEntry.is_regular_file()) {
				// Pair for the file_size and last write time, respectively
				std::pair<std::uintmax_t, std::filesystem::file_time_type> fsLwt = { dirEntry.file_size(), dirEntry.last_write_time() };
				// Add the dirEntry to map with the Pair of relevant metadata
				sourceMetadata[stripTopLvlDirFrmRelPath(dirEntry)] = fsLwt;
			}
		}
	}
	void fillFileSets() {
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath)) {
			if (dirEntry.is_regular_file()) {
				sourceFilesSet.insert(stripTopLvlDirFrmRelPath(dirEntry));
			}
		}
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(backupPath)) {
			if (dirEntry.is_regular_file()) {
				backupFilesSet.insert(stripTopLvlDirFrmRelPath(dirEntry));
			}
		}
	}
	void fillDirCopyDeleteSets() {
		// Fill the dirsToDelete and dirsToCopy vectors
		for (const auto& dir : std::filesystem::recursive_directory_iterator(backupPath)) {
			if (dir.is_directory() && !sourceDirsSet.contains(stripTopLvlDirFrmRelPath(dir))) {
				dirsToDelete.push_back(stripTopLvlDirFrmRelPath(dir));
			}
		}
		for (const auto& dir : std::filesystem::recursive_directory_iterator(sourcePath)) {
			if (dir.is_directory() && !backupDirsSet.contains(stripTopLvlDirFrmRelPath(dir))) {
				dirsToCopy.push_back(stripTopLvlDirFrmRelPath(dir));
			}
		}

	}
	void fillDirSets() {
		// Store the directories for source/backup dirs
		for (const auto& dir : std::filesystem::recursive_directory_iterator(sourcePath)) {
			if (dir.is_directory()) {
				sourceDirsSet.insert(stripTopLvlDirFrmRelPath(dir));
			}
		}
		for (const auto& dir : std::filesystem::recursive_directory_iterator(backupPath)) {
			if (dir.is_directory()) {
				backupDirsSet.insert(stripTopLvlDirFrmRelPath(dir));
			}
		}
	}
	void printDirectoryRecursively(std::string sourceOrBackup) {
		std::filesystem::path path;
		if (sourceOrBackup == "source") {
			path = sourcePath;
		}
		else if (sourceOrBackup == "backup") {
			path = backupPath;
		}
		// Error checking
		if (!std::filesystem::exists(path)) {
			std::cerr << "Path doesn't exist: " << path << '\n';
			return;
		}
		// Prints the regular files recursively
		std::cout << "Files in " << path.relative_path().generic_string() << ":" << '\n';
		for (auto const& dirEntry : std::filesystem::recursive_directory_iterator{ path }) {
			if (dirEntry.is_regular_file()) {
				std::cout << " - " << dirEntry.path().generic_string() << "\n"; // generic_string() shows "/" instead of string() which shows " to stay consistent
				std::cout << "\tSize: " << dirEntry.file_size() << " bytes" << ", Last Modified: " << dirEntry.last_write_time() << "\n"; // Prints the associated metadata
			}
		}
	}
	void printComparing() {
		while (true) {
			auto now = std::chrono::system_clock::now();
			std::cout << std::format("{:%H:%M:%S}", floor<std::chrono::seconds>(now)) << '\n';
			clearCollections();
			fillDirSets();
			fillDirCopyDeleteSets();
			fillFileSets();
			fillSourceMetadata();
			fillFilesToCopyDelete();

			printCollections();
			std::this_thread::sleep_for(std::chrono::seconds(5));
			std::system("cls");
		}

	}
	void printCollections() {
		// Print the collections
		std::cout << "sourceFilesSet:" << '\n';
		for (const auto& dir : sourceFilesSet) {
			std::cout << "\t" << dir << '\n';
		}
		std::cout << "backupFilesSet:" << '\n';
		for (const auto& dir : backupFilesSet) {
			std::cout << "\t" << dir << '\n';
		}
		std::cout << "filesToCopy:" << '\n';
		for (const auto& path : filesToCopy) {
			std::cout << "\t" << path << '\n';
		}
		std::cout << "filesToDelete:" << '\n';
		for (const auto& path : filesToDelete) {
			std::cout << "\t" << path << '\n';
		}
		std::cout << "sourceDirsSet" << '\n';
		for (const auto& dir : sourceDirsSet) {
			std::cout << "\t" << dir << '\n';
		}
		std::cout << "backupDirsSet" << '\n';
		for (const auto& dir : backupDirsSet) {
			std::cout << "\t" << dir << '\n';
		}
		std::cout << "dirsToCopy" << '\n';
		for (const auto& dir : dirsToCopy) {
			std::cout << "\t" << dir << '\n';
		}
		std::cout << "dirsToDelete" << '\n';
		for (const auto& dir : dirsToDelete) {
			std::cout << "\t" << dir << '\n';
		}
	}
};