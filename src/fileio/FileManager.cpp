#include "fileio/FileManager.h"

FileManager::FileManager(const std::string& path)
    : filePath(path) {
}

const std::string& FileManager::getFilePath() const {
    return filePath;
}
