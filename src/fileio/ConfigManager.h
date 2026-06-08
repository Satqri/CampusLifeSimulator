#ifndef CLS_FILEIO_CONFIGMANAGER_H
#define CLS_FILEIO_CONFIGMANAGER_H

#include "core/GameSettings.h"
#include "fileio/JsonFileManager.h"

/**
 * @class ConfigManager
 * @brief 游戏设置 JSON 管理器
 */
class ConfigManager : public JsonFileManager {
public:
    explicit ConfigManager(const std::string& filePath);

    /** @brief 读取设置 */
    cls::GameSettings loadSettings();

    /** @brief 保存设置 */
    bool saveSettings(const cls::GameSettings& settings);
};

#endif // CLS_FILEIO_CONFIGMANAGER_H
