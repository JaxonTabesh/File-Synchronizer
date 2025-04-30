#include "DirectoryState.h"

bool FileInfo::operator==(const FileInfo& other) const
{
	return size == other.size && lastWriteTime == other.lastWriteTime;
}

DirectoryState::DirectoryState(const std::filesystem::path path) : absPath(path)
{
	scan();
}

void DirectoryState::scan()
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