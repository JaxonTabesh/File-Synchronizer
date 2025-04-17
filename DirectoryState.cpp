#include <algorithm>
#include <filesystem>
#include <set>
#include <unordered_map>

struct FileInfo
{
	std::uintmax_t size;
	std::filesystem::file_time_type lastWriteTime;
	bool operator==(const FileInfo& other) const
	{
		return size == other.size && lastWriteTime == other.lastWriteTime;
	}
};

struct DirectoryState
{
	// State of a directory: its path then its metadata in an unordered map, nested directories in a vector, and files in a vector
	std::filesystem::path absPath;
	std::set<std::filesystem::path> files; // Containers store relative paths
	std::set<std::filesystem::path> dirs;
	std::unordered_map<std::filesystem::path, FileInfo> metadata;

	DirectoryState(const std::filesystem::path path) : absPath(path)
	{
		scan();
	}

	void scan()
	{
		files.clear();
		dirs.clear();
		metadata.clear();
		for (const auto& entry : std::filesystem::recursive_directory_iterator(absPath))
		{
			// Creates a relative path that begins after the outer directory
			std::filesystem::path relPath = entry.path().lexically_relative(absPath);

			if (entry.is_regular_file())
			{
				files.insert(relPath);
				metadata[relPath] = FileInfo(entry.file_size(), entry.last_write_time());
			}
			else if (entry.is_directory())
			{
				dirs.insert(relPath);
			}
			// else don't push back
		}
	}
};