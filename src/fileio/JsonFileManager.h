#ifndef CLS_FILEIO_JSONFILEMANAGER_H
#define CLS_FILEIO_JSONFILEMANAGER_H

#include "fileio/FileManager.h"

#include <nlohmann/json.hpp>

/**
 * @class JsonFileManager
 * @brief JSON 文件管理抽象基类
 */
class JsonFileManager : public FileManager {
public:
    explicit JsonFileManager(const std::string& filePath);

    bool read() override;
    bool write() override;

protected:
    nlohmann::json jsonData;
};

#endif // CLS_FILEIO_JSONFILEMANAGER_H
