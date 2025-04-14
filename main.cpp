#include "OneWayFileSync.cpp"
#include <filesystem>
#include <iostream>
#include <set>

int main() {

	std::filesystem::path dir = "./source/";
	std::filesystem::path backupDir = "./backup/";

	OneWayFileSync oneWayFileSync(dir, backupDir);

	oneWayFileSync.printComparing();

	return 0;
}