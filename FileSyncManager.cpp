#include "FileSyncManager.h"


std::filesystem::path FileSyncManager::stripOuterDirectory(std::filesystem::path dirEntry, bool isSourceEntry)
{
	return dirEntry.lexically_relative(isSourceEntry ? sourcePath.absPath : targetPath.absPath);
}
void FileSyncManager::updateCollections()
{
	filesToCopy.clear();
	filesToDelete.clear();
	dirsToCopy.clear();
	dirsToDelete.clear();
	sourcePath.scan();
	targetPath.scan();
	fillFilesToCopyDelete();
	fillDirCopyDeleteSets();
}
void FileSyncManager::fillFilesToCopyDelete()
{
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(targetPath.absPath))
	{
		if (dirEntry.is_regular_file())
		{
			if (sourcePath.files.contains(stripOuterDirectory(dirEntry, false)) && sourcePath.metadata[stripOuterDirectory(dirEntry, false)] != targetPath.metadata[stripOuterDirectory(dirEntry, false)])
			{
				filesToCopy.push_back(stripOuterDirectory(dirEntry, false));
			}
			else if (!sourcePath.files.contains(stripOuterDirectory(dirEntry, false)))
			{
				filesToDelete.push_back(stripOuterDirectory(dirEntry, false));
			}
			// else if it contains dirEntry and has equivalent metadata then no need to do anything
		}
	}
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(sourcePath.absPath))
	{
		if (dirEntry.is_regular_file())
		{
			if (!targetPath.files.contains(stripOuterDirectory(dirEntry, true)))
			{
				filesToCopy.push_back(stripOuterDirectory(dirEntry, true));
			}
		}
	}

}
void FileSyncManager::fillDirCopyDeleteSets()
{
	// Fill the dirsToDelete and dirsToCopy vectors
	for (const auto& entry : std::filesystem::recursive_directory_iterator(targetPath.absPath))
	{
		if (entry.is_directory() && !sourcePath.dirs.contains(stripOuterDirectory(entry, false)))
		{
			dirsToDelete.push_back(stripOuterDirectory(entry, false));
		}
	}
	for (const auto& dir : std::filesystem::recursive_directory_iterator(sourcePath.absPath))
	{
		if (dir.is_directory() && !targetPath.dirs.contains(stripOuterDirectory(dir, true)))
		{
			dirsToCopy.push_back(stripOuterDirectory(dir, true));
		}
	}
}
void FileSyncManager::printCollections()
{
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

// Constructor requires a source directory and a target directory in either string or std::filesystem::path
FileSyncManager::FileSyncManager(std::filesystem::path source, std::filesystem::path synced) : sourcePath(source), targetPath(synced) {}

void FileSyncManager::beginOneWaySync(bool dryRun)
{
#ifdef _WIN32
	system("cls");
#else // If MacOS/Linux
	system("clear");
#endif
	if (!dryRun)
	{
		std::cout << "One-Way-Sync Active" << "\n";
		std::cout << "\tSource Path: " << "\n\t\t" << sourcePath.absPath.generic_string() << "\n";
		std::cout << "\tTarget Path: " << "\n\t\t" << targetPath.absPath.generic_string() << "\n";
	}
	// TODO: use built-in tooling for copying recursively. Use more copyOptions instead of custom code.
	while (true)
	{
		updateCollections();
		if (dryRun)
		{
			std::cout << "One-Way-Sync Active (DryRun Mode)" << "\n";
			std::cout << "\tSource Path: " << "\n\t\t" << sourcePath.absPath.generic_string() << "\n";
			std::cout << "\tTarget Path: " << "\n\t\t" << targetPath.absPath.generic_string() << "\n";
		}

		for (const auto& dir : dirsToDelete)
		{
			std::filesystem::path path = targetPath.absPath / dir;
			if (std::filesystem::directory_entry(path).exists())
			{
				if (!dryRun)
				{
					std::filesystem::remove_all(path);
				}
				std::cout << "Deleted directory:" << "\n\t" << path.generic_string() << "\n";
			}
		}
		// delete files in cloud_synced
		for (const auto& file : filesToDelete)
		{
			std::filesystem::path path = targetPath.absPath / file;
			if (std::filesystem::directory_entry(path).is_regular_file())
			{
				if (!dryRun)
				{
					std::filesystem::remove(path);
				}
				std::cout << "Deleted file:" << "\n\t" << path.generic_string() << "\n";
			}
		}
		// create directories to target
		for (const auto& dir : dirsToCopy)
		{
			std::filesystem::path path = targetPath.absPath / dir;
			if (!std::filesystem::directory_entry(path).exists())
			{
				if (!dryRun)
				{
					std::filesystem::create_directory(path);
				}
				std::cout << "Created directory:" << "\n\t" << path.generic_string() << "\n";
			}
		}
		// copy files to target
		for (const auto& file : filesToCopy)
		{
			std::filesystem::path fromPath = sourcePath.absPath / file;
			std::filesystem::path toPath = targetPath.absPath / file;
			if (std::filesystem::directory_entry(fromPath).is_regular_file() && (/*If toPath exists then check if equivalent with fromPath*/ std::filesystem::is_regular_file(toPath) ? !std::filesystem::equivalent(fromPath, toPath) : true))
			{
				if (!dryRun)
				{
					std::filesystem::copy_file(fromPath, toPath, std::filesystem::copy_options::overwrite_existing);
				}
				std::cout << "Copied file from:" << "\n\t" << fromPath.generic_string() << "\n";
				std::cout << "\tTo:" << "\n\t" << toPath.generic_string() << "\n";
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

void FileSyncManager::beginOneWayComparing()
{
	while (true)
	{
		updateCollections();
		printCollections();
		std::this_thread::sleep_for(std::chrono::seconds(1));
#ifdef _WIN32
		system("cls");
#else // If MacOS/Linux
		system("clear");
#endif
	}
}

void FileSyncManager::oneTimeBackup(std::filesystem::path source, std::filesystem::path backup, bool dryRun)
{
	// Use options parameter and cout files that are copied to the backup
	// Implement error checking
	// Make the function static

#ifdef _WIN32
	system("cls");
#else // If MacOS/Linux
	system("clear");
#endif

	if (!source.is_absolute() || !backup.is_absolute())
	{
		std::cerr << "Paths for a one time backup must be absolute." << "\n";
	}

	std::string newBackupName = "Backup";
	int backupNumber = 1;

	// Will make the next available backup folder name
	while (std::filesystem::exists(backup / newBackupName) && std::filesystem::is_directory(backup / newBackupName))
	{
		backupNumber++;
		newBackupName = "Backup " + std::to_string(backupNumber);
	}
	const auto backupPath = backup / newBackupName;

	if (!dryRun)
	{
		std::filesystem::create_directory(backupPath);
	}

	std::cout << "One-Time-Backup" << (dryRun ? " (DryRun Mode)" : " (Active)") << "\n";
	std::cout << "\tSource Path: " << "\n\t\t" << source.generic_string() << "\n";
	std::cout << "\tBackup Path: " << "\n\t\t" << backupPath.generic_string() << "\n";

	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(source))
	{
		const auto relPath = std::filesystem::path(dirEntry).lexically_relative(source);
		const auto copyOptions = std::filesystem::copy_options::skip_existing;
		if (!dryRun)
		{
			std::filesystem::copy(dirEntry, backupPath / relPath, copyOptions);
		}
		std::cout << "Copied file from:\n\t" << std::filesystem::path(dirEntry).generic_string() << "\n";
		std::cout << "\tTo:\n\t" << std::filesystem::path(backupPath / relPath).generic_string() << "\n";
	}
}

void FileSyncManager::printDirectoryRecursively(std::filesystem::path path)
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
			std::filesystem::file_time_type lastWriteTime = dirEntry.last_write_time();
			std::chrono::time_point<std::chrono::system_clock> printableWriteTime = std::chrono::clock_cast<std::chrono::system_clock>(lastWriteTime);
			std::cout << "\tSize: " << dirEntry.file_size() << " bytes" << ", Last Modified: " << printableWriteTime << "\n"; // Prints the associated metadata
		}
	}
}