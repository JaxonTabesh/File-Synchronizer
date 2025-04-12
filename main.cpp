#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

void printDirectoryRecursively(std::filesystem::path directory, bool filesOnly);

int main() {

	std::filesystem::path directory = "./source/";

	// Create a new txt file via user input
	// 
	//std::string fileName;
	//std::cout << "Text File Name: ";
	//std::getline(std::cin, fileName);
	//fileName.append(".txt");
	//std::ofstream{ directory / fileName };

	// Prints what the current path is
	// 
	std::cout << std::filesystem::current_path().generic_string() << std::endl;

	// Check if the path exists
	if (!std::filesystem::exists(directory)) {
		std::cerr << "Path doesn't exist: " << directory << '\n';
		return 1;
	}

	// Run the functions to print the directory's contents
	// 
	printDirectoryRecursively(directory, true);

	return 0;
}

void printDirectoryRecursively(std::filesystem::path directory, bool filesOnly) {
	std::cout << "Files in " << directory.relative_path() << ":" << '\n';
	for (auto const& dirEntry : std::filesystem::recursive_directory_iterator{ directory }) {
		if (filesOnly && dirEntry.is_regular_file())
			std::cout << dirEntry.path().generic_string() << "\n"; // generic_string() shows "/" instead of string() which shows "\" to stay consistent
		else if (!filesOnly)
			std::cout << dirEntry.path().generic_string() << "\n";
	}
}