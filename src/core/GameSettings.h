#ifndef CLS_CORE_GAMESETTINGS_H
#define CLS_CORE_GAMESETTINGS_H

#include "core/Localization.h"

#include <string>
#include <vector>

namespace cls {

/**
 * @struct WindowScalePreset
 * @brief 窗口缩放预设
 */
struct WindowScalePreset {
    unsigned int width;
    unsigned int height;
    std::string labelKey;
};

/**
 * @struct GameSettings
 * @brief 运行时设置数据
 */
struct GameSettings {
    int bgmVolume = 70;
    int sfxVolume = 80;
    int windowScaleIndex = 1;
    Language language = Language::English;
};

/**
 * @brief 获取所有窗口缩放预设
 * @return 预设列表引用
 */
const std::vector<WindowScalePreset>& windowScalePresets();

/**
 * @brief 获取默认设置
 * @return 默认设置数据
 */
GameSettings defaultSettings();

/**
 * @brief 加载设置
 * @param relativePath 相对配置路径
 * @return 读取成功则返回文件中的设置，否则返回默认设置
 */
GameSettings loadSettings(const std::string& relativePath);

/**
 * @brief 保存设置
 * @param relativePath 相对配置路径
 * @param settings 待保存的设置
 * @return true 保存成功，false 保存失败
 */
bool saveSettings(const std::string& relativePath, const GameSettings& settings);

/**
 * @brief 限制设置值到合法范围
 * @param settings 待修正的设置
 */
void clampSettings(GameSettings& settings);

} // namespace cls

#endif // CLS_CORE_GAMESETTINGS_H
