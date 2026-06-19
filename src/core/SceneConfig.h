#ifndef CLS_CORE_SCENECONFIG_H
#define CLS_CORE_SCENECONFIG_H

#include "ui/SceneBackground.h"
#include "core/Localization.h"
#include <string>
#include <unordered_map>
#include <fstream>
#include <nlohmann/json.hpp>

/**
 * @struct ScenePageConfig
 * @brief 单个 DemoPage 的过渡场景配置
 */
struct ScenePageConfig {
    SceneBackgroundType background = SceneBackgroundType::Dormitory;
    std::string title;
    std::string subtitle;
};

/**
 * @brief 从 JSON 加载场景过渡配置
 * @param path JSON 文件路径
 * @return page name → config 映射
 */
inline std::unordered_map<std::string, ScenePageConfig> loadSceneConfig(const std::string& path) {
    std::unordered_map<std::string, ScenePageConfig> configs;
    std::ifstream file(path);
    if (!file.is_open()) return configs;
    nlohmann::json data;
    file >> data;
    for (auto& [key, val] : data["pages"].items()) {
        ScenePageConfig cfg;
        std::string bgName = val.value("background", "Dormitory");
        if (bgName == "Dormitory")   cfg.background = SceneBackgroundType::Dormitory;
        else if (bgName == "Library")     cfg.background = SceneBackgroundType::Library;
        else if (bgName == "Classroom")   cfg.background = SceneBackgroundType::Classroom;
        else if (bgName == "Cafeteria")   cfg.background = SceneBackgroundType::Cafeteria;
        const std::string titleKey = val.value("title_key", "");
        const std::string subtitleKey = val.value("subtitle_key", "");
        cfg.title = titleKey.empty() ? val.value("title", "") : cls::text(titleKey);
        cfg.subtitle = subtitleKey.empty() ? val.value("subtitle", "") : cls::text(subtitleKey);
        configs[key] = cfg;
    }
    return configs;
}

#endif
