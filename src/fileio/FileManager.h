#ifndef CLS_FILEIO_FILEMANAGER_H
#define CLS_FILEIO_FILEMANAGER_H

#include <string>

/**
 * @class FileManager
 * @brief 文件 IO 抽象基类
 */
class FileManager {
public:
    explicit FileManager(const std::string& filePath);
    virtual ~FileManager() = default;

    /** @brief 读取文件内容 */
    virtual bool read() = 0;

    /** @brief 写入当前内容 */
    virtual bool write() = 0;

    /** @brief 获取文件路径 */
    const std::string& getFilePath() const;

protected:
    std::string filePath;
};

#endif // CLS_FILEIO_FILEMANAGER_H
