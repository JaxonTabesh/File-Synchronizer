#include "OneWayFileSync.h"
#include <iostream>

int main(int argc, char* argv[])
{
	try {
		if (argc > 1)
		{
			if (std::string(argv[1]) == "--help")
			{
				std::cout << "\n";
				std::cout << "Command:" << "\n";
				std::cout << "\t" << "one-way-dir-sync strSourcePath strTargetPath --dry-run" << "\n";
				std::cout << "\t" << "two-way-dir-sync strSourcePath strTargetPath --dry-run" << "\n";
				std::cout << "\t" << "rec-backup strSourcePath strTargetPath boolInterval intInterval stringUnits(\"s\" \"m\" \"h\" \"d\") --dry-run" << "\n";
				std::cout << "General Options:" << "\n";
				std::cout << "\t" << "--help" << "\n";
			}
			else if (std::string(argv[1]) == "one-way-dir-sync")
			{
				if (argc < 4)
				{
					std::cerr << "Missing argument(s)" << "\n";
					return 1;
				}
				// Implement one-way-dir-sync
				std::filesystem::path sourceDir = std::string(argv[2]);
				std::filesystem::path targetDir = std::string(argv[3]);
				//std::cout << sourceDir << "\n";
				//std::cout << targetDir << "\n";
				if (!std::filesystem::exists(sourceDir) || !std::filesystem::is_directory(sourceDir))
				{
					std::cout << "-" << "\n";
					std::cerr << "Error: Source directory either doesn't exist or is not a directory." << "\n";
					std::cout << "Please try again." << "\n";

					return 1;
				}
				if (!std::filesystem::exists(targetDir) || !std::filesystem::is_directory(targetDir))
				{
					std::cout << "\n";
					std::cerr << "Error: Target directory either doesn't exist or is not a directory." << "\n";
					std::cout << "Please try again." << "\n";

					return 1;
				}
				if (argc >= 5 && std::string(argv[4]) == "--dry-run")
				{
					OneWayFileSync oneWayFileSync(sourceDir, targetDir);
					oneWayFileSync.beginSync(true);
				}
				else
				{
					OneWayFileSync oneWayFileSync(sourceDir, targetDir);
					oneWayFileSync.beginSync();
				}
			}
			else if (std::string(argv[1]) == "two-way-dir-sync")
			{
				// Implement two-way-dir-sync
				std::cout << "\n";
				std::cout << "Coming soon" << "\n";
			}
			else if (std::string(argv[1]) == "rec-backup")
			{
				// Implement recurring backup with options for how often and if only one backup
				std::cout << "\n";
				std::cout << "Coming soon" << "\n";
			}
			else
			{
				std::cout << "\n";
				std::cout << "That is not a valid command" << "\n";
				std::cout << "--help for a list of commands" << "\n";
			}
		}
	}
	catch (std::filesystem::filesystem_error& e)
	{
		std::cerr << "Filesystem error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}