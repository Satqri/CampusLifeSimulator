#include "fileio/JsonFileManager.h"

#include <filesystem>
#include <fstream>

JsonFileManager::JsonFileManager(const std::string& path)
    : FileManager(path)
    , jsonData() {
}

bool JsonFileManager::read() {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;
    file >> jsonData;
    return true;
}

bool JsonFileManager::write() {
    std::filesystem::path path(filePath);
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    std::ofstream file(filePath);
    if (!file.is_open()) return false;
    file << jsonData.dump(2);
    return true;
}
