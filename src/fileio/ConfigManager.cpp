#include "fileio/ConfigManager.h"

ConfigManager::ConfigManager(const std::string& filePath)
    : JsonFileManager(filePath) {
}

cls::GameSettings ConfigManager::loadSettings() {
    cls::GameSettings settings = cls::defaultSettings();
    if (!read()) {
        cls::clampSettings(settings);
        return settings;
    }

    settings.bgmVolume = jsonData.value("bgmVolume", settings.bgmVolume);
    settings.sfxVolume = jsonData.value("sfxVolume", settings.sfxVolume);
    settings.windowScaleIndex = jsonData.value("windowScaleIndex", settings.windowScaleIndex);
    settings.language = jsonData.value("language", std::string("en")) == "zh"
        ? cls::Language::Chinese
        : cls::Language::English;
    cls::clampSettings(settings);
    return settings;
}

bool ConfigManager::saveSettings(const cls::GameSettings& settings) {
    jsonData["bgmVolume"] = settings.bgmVolume;
    jsonData["sfxVolume"] = settings.sfxVolume;
    jsonData["windowScaleIndex"] = settings.windowScaleIndex;
    jsonData["language"] = settings.language == cls::Language::Chinese ? "zh" : "en";
    return write();
}
