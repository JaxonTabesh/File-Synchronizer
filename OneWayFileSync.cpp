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
	std::filesystem::path syncedPath;
	std::set<std::string> sourceFilesSet = {};
	std::set<std::string> syncedFilesSet = {};
	std::set<std::string> sourceDirsSet = {};
	std::set<std::string> syncedDirsSet = {};
	// Use an unordered map for keys as paths and metadata as values for comparison
	std::unordered_map<std::string, std::pair<std::uintmax_t, std::filesystem::file_time_type>> sourceMetadata = {};
	std::vector<std::string> filesToCopy = {}; // Paths that exist in source but
	// not synced or paths with differing metadata in source and synced
	std::vector<std::string> filesToDelete = {}; // Files in synced but not in source
	// Dirs to create/delete in synced
	std::vector<std::string> dirsToCopy = {};
	std::vector<std::string> dirsToDelete = {};

	// stripTopLevelDirectoryFromRelativePath
	std::string stripTopLvlDirFrmRelPath(std::filesystem::directory_entry dirEntry, bool isSourceEntry)
	{
		//if (isSourceEntry){}
		// TODO: use more built-in/safer tooling
		//std::string dirEntryString = dirEntry.path().generic_string();
		// TODO: POTENTIALLY USE TEMPORARILY TO MAKE THE FUNCTION SAFER
		std::string dirEntryString = dirEntry.path().relative_path().generic_string();

		// TODO: Update to more modular method. Currently, this will not function properly with more than 2 of the same named nested dirs
		std::string stem = ".*?";
		if (isSourceEntry)
		{
			stem += sourcePath.stem().generic_string();
		}
		else
		{
			stem += syncedPath.stem().generic_string();
		}
		stem += "/";

		std::regex pattern(stem);
		std::string path = std::regex_replace(dirEntryString, pattern, "");
		//// TODO: add error checking for the regex in case the format is not what is expected
		return path;
	}
	static std::string combineRelativePaths(std::string path1, std::string path2)
	{
		// TODO: use built-in, safe tooling
		return path1 + "/" + path2;
	}
	void clearCollections()
	{
		sourceFilesSet.clear();
		syncedFilesSet.clear();
		sourceDirsSet.clear();
		syncedDirsSet.clear();
		sourceMetadata.clear();
		filesToCopy.clear();
		filesToDelete.clear();
		dirsToCopy.clear();
		dirsToDelete.clear();
	}
	void updateCollections()
	{
		clearCollections();
		fillDirSets();
		fillDirCopyDeleteSets();
		fillFileSets();
		fillSourceMetadata();
		fillFilesToCopyDelete();
	}
	void fillFilesToCopyDelete()
	{
		// use the stripping function instead of dirEntry's
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(syncedPath))
		{
			if (dirEntry.is_regular_file())
			{
				std::pair<std::uintmax_t, std::filesystem::file_time_type> fsLwt = { dirEntry.file_size(), dirEntry.last_write_time() };
				if (sourceFilesSet.contains(stripTopLvlDirFrmRelPath(dirEntry, false)) && sourceMetadata[stripTopLvlDirFrmRelPath(dirEntry, false)] != fsLwt)
				{
					filesToCopy.push_back(stripTopLvlDirFrmRelPath(dirEntry, false));
				}
				else if (!sourceFilesSet.contains(stripTopLvlDirFrmRelPath(dirEntry, false)))
				{
					filesToDelete.push_back(stripTopLvlDirFrmRelPath(dirEntry, false));
				}
				// else if it contains dirEntry and has equivalent metadata then no need to do anything
			}
		}
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath))
		{
			if (dirEntry.is_regular_file())
			{
				if (!syncedFilesSet.contains(stripTopLvlDirFrmRelPath(dirEntry, true)))
				{
					filesToCopy.push_back(stripTopLvlDirFrmRelPath(dirEntry, true));
				}
			}
		}

	}
	void fillSourceMetadata()
	{
		// Insert the sourceFilesSet with the sourceDir files
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath))
		{
			if (dirEntry.is_regular_file())
			{
				// Pair for the file_size and last write time, respectively
				std::pair<std::uintmax_t, std::filesystem::file_time_type> fsLwt = { dirEntry.file_size(), dirEntry.last_write_time() };
				// Add the dirEntry to map with the Pair of relevant metadata
				sourceMetadata[stripTopLvlDirFrmRelPath(dirEntry, true)] = fsLwt;
			}
		}
	}
	void fillFileSets() {
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath))
		{
			if (dirEntry.is_regular_file())
			{
				sourceFilesSet.insert(stripTopLvlDirFrmRelPath(dirEntry, true));
			}
		}
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(syncedPath))
		{
			if (dirEntry.is_regular_file())
			{
				syncedFilesSet.insert(stripTopLvlDirFrmRelPath(dirEntry, false));
			}
		}
	}
	void fillDirCopyDeleteSets()
	{
		// Fill the dirsToDelete and dirsToCopy vectors
		for (const auto& dir : std::filesystem::recursive_directory_iterator(syncedPath))
		{
			if (dir.is_directory() && !sourceDirsSet.contains(stripTopLvlDirFrmRelPath(dir, false)))
			{
				dirsToDelete.push_back(stripTopLvlDirFrmRelPath(dir, false));
			}
		}
		for (const auto& dir : std::filesystem::recursive_directory_iterator(sourcePath))
		{
			if (dir.is_directory() && !syncedDirsSet.contains(stripTopLvlDirFrmRelPath(dir, true)))
			{
				dirsToCopy.push_back(stripTopLvlDirFrmRelPath(dir, true));
			}
		}
	}
	void fillDirSets()
	{
		// Store the directories for source/synced dirs
		for (const auto& dir : std::filesystem::recursive_directory_iterator(sourcePath))
		{
			if (dir.is_directory())
			{
				sourceDirsSet.insert(stripTopLvlDirFrmRelPath(dir, true));
			}
		}
		for (const auto& dir : std::filesystem::recursive_directory_iterator(syncedPath))
		{
			if (dir.is_directory())
			{
				syncedDirsSet.insert(stripTopLvlDirFrmRelPath(dir, false));
			}
		}
	}
	void printCollections()
	{
		//std::cout << "sourceFilesSet:" << "\n";
		//for (const auto& dir : sourceFilesSet)
		//{
		//	std::cout << "\t" << dir << "\n";
		//}
		//std::cout << "syncedFilesSet:" << "\n";
		//for (const auto& dir : syncedFilesSet)
		//{
		//	std::cout << "\t" << dir << "\n";
		//}
		std::cout << "filesToCopy:" << "\n";
		for (const auto& path : filesToCopy)
		{
			std::cout << "\t" << path << "\n";
		}
		std::cout << "filesToDelete:" << "\n";
		for (const auto& path : filesToDelete)
		{
			std::cout << "\t" << path << "\n";
		}
		//std::cout << "sourceDirsSet:" << "\n";
		//for (const auto& dir : sourceDirsSet)
		//{
		//	std::cout << "\t" << dir << "\n";
		//}
		//std::cout << "syncedDirsSet:" << "\n";
		//for (const auto& dir : syncedDirsSet)
		//{
		//	std::cout << "\t" << dir << "\n";
		//}
		std::cout << "dirsToCopy:" << "\n";
		for (const auto& dir : dirsToCopy)
		{
			std::cout << "\t" << dir << "\n";
		}
		std::cout << "dirsToDelete:" << "\n";
		for (const auto& dir : dirsToDelete)
		{
			std::cout << "\t" << dir << "\n";
		}
	}
public:
	// Constructor requires a source directory and a synced directory in either string or std::filesystem::path
	OneWayFileSync(std::filesystem::path source, std::filesystem::path synced) : sourcePath(source), syncedPath(synced) {}
	void beginSync(bool dryRun = false)
	{
		// TODO: use built-in tooling for copying recursively. Use more copyOptions instead of custom code.
		while (true)
		{
			updateCollections();

			// delete extra directories in cloud_synced
			for (const auto& dir : dirsToDelete)
			{
				std::string path = combineRelativePaths(syncedPath.generic_string(), dir);
				if (std::filesystem::directory_entry(path).exists())
				{
					if (!dryRun)
					{
						std::filesystem::remove_all(path);
					}
					std::cout << "Deleted directory: \"" << "\"" << path << "\n";
				}
			}
			// delete files in cloud_synced
			for (const auto& file : filesToDelete)
			{
				std::string path = combineRelativePaths(syncedPath.generic_string(), file);
				if (std::filesystem::directory_entry(path).is_regular_file())
				{
					if (!dryRun)
					{
						std::filesystem::remove(path);
					}
					std::cout << "Deleted file: \"" << path << "\"" << "\n";
				}
			}
			// create directories to cloud_synced
			for (const auto& dir : dirsToCopy)
			{
				std::string path = combineRelativePaths(syncedPath.generic_string(), dir);
				if (!std::filesystem::directory_entry(path).exists())
				{
					if (!dryRun)
					{
						std::filesystem::create_directory(path);
					}
					std::cout << "Created directory: \"" << path << "\"" << "\n";
				}
			}
			// copy files to cloud_synced
			for (const auto& file : filesToCopy)
			{
				std::string fromPath = combineRelativePaths(sourcePath.generic_string(), file);
				std::string toPath = combineRelativePaths(syncedPath.generic_string(), file);
				if (std::filesystem::directory_entry(fromPath).is_regular_file() && (/*If toPath exists then check if equivalent with fromPath*/ std::filesystem::is_regular_file(toPath) ? !std::filesystem::equivalent(fromPath, toPath) : true))
				{
					if (!dryRun)
					{
						std::filesystem::copy(fromPath, toPath, std::filesystem::copy_options::overwrite_existing);
					}
					std::cout << "Copied file from: \"" << fromPath << "\"" << "\n";
					std::cout << "\tTo: \"" << toPath << "\"" << "\n";
				}
			}

			std::this_thread::sleep_for(std::chrono::seconds(1)); // TODO: change to OS listener for a change in the directory
			if (dryRun)
			{
#ifdef _WIN32
				system("cls");
#else // If MacOS/Linux
				system("clear");
#endif
			}
		}
	}

	static void printDirectoryRecursively(std::string path)
	{
		std::filesystem::path inputPath(path);

		// Error checking
		if (!std::filesystem::exists(inputPath))
		{
			std::cerr << "Path doesn't exist: " << inputPath << "\n";
			return;
		}
		// Prints the regular files recursively
		std::cout << "Files in " << inputPath.relative_path().generic_string() << ":" << "\n";
		for (auto const& dirEntry : std::filesystem::recursive_directory_iterator{ inputPath })
		{
			if (dirEntry.is_regular_file())
			{
				std::cout << " - " << dirEntry.path().generic_string() << "\n"; // generic_string() shows "/" instead of string() which shows " to stay consistent
				std::cout << "\tSize: " << dirEntry.file_size() << " bytes" << ", Last Modified: " << dirEntry.last_write_time() << "\n"; // Prints the associated metadata
			}
		}
	}
	void beginComparing()
	{
		while (true)
		{
			clearCollections();
			fillDirSets();
			fillDirCopyDeleteSets();
			fillFileSets();
			fillSourceMetadata();
			fillFilesToCopyDelete();

			printCollections();
			std::this_thread::sleep_for(std::chrono::seconds(1));
#ifdef _WIN32
			system("cls");
#else // If MacOS/Linux
			system("clear");
#endif
		}
	}
};