#include "OneWayFileSync.h"


std::filesystem::path OneWayFileSync::stripOuterDirectory(std::filesystem::path dirEntry, bool isSourceEntry)
{
	return dirEntry.lexically_relative(isSourceEntry ? sourcePath.absPath : targetPath.absPath);
}
void OneWayFileSync::updateCollections()
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
void OneWayFileSync::fillFilesToCopyDelete()
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
void OneWayFileSync::fillDirCopyDeleteSets()
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
void OneWayFileSync::printCollections()
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

// Constructor requires a source directory and a synced directory in either string or std::filesystem::path
OneWayFileSync::OneWayFileSync(std::filesystem::path source, std::filesystem::path synced) : sourcePath(source), targetPath(synced) {}
void OneWayFileSync::beginSync(bool dryRun)
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
		// create directories to cloud_synced
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
		// copy files to cloud_synced
		for (const auto& file : filesToCopy)
		{
			std::filesystem::path fromPath = sourcePath.absPath / file;
			std::filesystem::path toPath = targetPath.absPath / file;
			if (std::filesystem::directory_entry(fromPath).is_regular_file() && (/*If toPath exists then check if equivalent with fromPath*/ std::filesystem::is_regular_file(toPath) ? !std::filesystem::equivalent(fromPath, toPath) : true))
			{
				if (!dryRun)
				{
					std::filesystem::copy(fromPath, toPath, std::filesystem::copy_options::overwrite_existing);
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

void OneWayFileSync::printDirectoryRecursively(std::filesystem::path path)
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
void OneWayFileSync::beginComparing()
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