#include "OneWayFileSync.cpp"
#include <filesystem>
#include <iostream>
#include <set>

int main()
{

	std::filesystem::path sourceDir = "./cloud_source/";
	std::filesystem::path syncedDir = "./cloud_synced/";

	OneWayFileSync oneWayFileSync(sourceDir, syncedDir);

	//std::filesystem::remove("./cloud_backup/MyText.txt");
	//std::cout << std::filesystem::directory_entry(sourceDir) + std::filesystem::directory_entry(syncedDir);

	oneWayFileSync.beginSync();
	//oneWayFileSync.beginComparing();

	return 0;
}