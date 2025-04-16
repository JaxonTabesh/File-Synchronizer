#include "OneWayFileSync.cpp"
#include <iostream>

int main(int argc, char* argv[])
{
	// Initial string (argv[0]) is the function name.
	// Second String (argv[1]) indicates which process you want to run—i.e. one-way directory syncing, single backup, recurring backup, two-way directory syncing.
	// or to indicate flags like --help
	// Third/Fourth strings (argv[2]/(argv[3])) to indicate source and target, respectively.

	try {
		if (argc > 1)
		{
			if (std::string(argv[1]) == "--help")
			{
				std::cout << "Command:" << "\n";
				std::cout << "\t" << "one-way-dir-sync strSourcePath strTargetPath" << "\n";
				std::cout << "\t" << "two-way-dir-sync strSourcePath strTargetPath" << "\n";
				std::cout << "\t" << "rec-backup strSourcePath strTargetPath boolInterval intInterval stringUnits (\"seconds\" \"minutes\" \"hours\" \"days\")" << "\n";
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
				if (!std::filesystem::exists(sourceDir) || !std::filesystem::is_directory(sourceDir))
				{
					std::cerr << "Error: Source directory either doesn't exist or is not a directory." << "\n";
					return 1;
				}
				if (!std::filesystem::exists(targetDir) || !std::filesystem::is_directory(targetDir))
				{
					std::cerr << "Error: Target directory either doesn't exist or is not a directory." << "\n";
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
				std::cout << "Coming soon" << "\n";
			}
			else if (std::string(argv[1]) == "rec-backup")
			{
				// Implement recurring backup with options for how often and if only one backup
				std::cout << "Coming soon" << "\n";
			}
			else
			{
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