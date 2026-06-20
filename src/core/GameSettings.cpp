#include "core/GameSettings.h"
#include "utils/AssetPath.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace cls {
namespace {

using json = nlohmann::json;

const std::vector<WindowScalePreset> kScalePresets = {
    {960, 540, "settings.scale.1"},
    {1280, 720, "settings.scale.2"},
    {1600, 900, "settings.scale.3"},
    {1920, 1080, "settings.scale.4"}
};

} // namespace

const std::vector<WindowScalePreset>& windowScalePresets() {
    return kScalePresets;
}

GameSettings defaultSettings() {
    return {};
}

void clampSettings(GameSettings& settings) {
    settings.bgmVolume = std::clamp(settings.bgmVolume, 0, 100);
    settings.sfxVolume = std::clamp(settings.sfxVolume, 0, 100);
    settings.windowScaleIndex = std::clamp(settings.windowScaleIndex, 0,
        static_cast<int>(kScalePresets.size()) - 1);
}

GameSettings loadSettings(const std::string& relativePath) {
    GameSettings settings = defaultSettings();
    const std::string resolved = resolveAssetPath(relativePath);
    std::ifstream file(resolved);
    if (!file.is_open()) {
        clampSettings(settings);
        return settings;
    }

    json data;
    file >> data;
    settings.bgmVolume = data.value("bgmVolume", settings.bgmVolume);
    settings.sfxVolume = data.value("sfxVolume", settings.sfxVolume);
    settings.windowScaleIndex = data.value("windowScaleIndex", settings.windowScaleIndex);
    settings.language = data.value("language", std::string("en")) == "zh"
        ? Language::Chinese
        : Language::English;
    clampSettings(settings);
    return settings;
}

bool saveSettings(const std::string& relativePath, const GameSettings& settings) {
    const std::string resolved = resolveAssetPath(relativePath);
    std::filesystem::path path(resolved);
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    json data;
    data["bgmVolume"] = settings.bgmVolume;
    data["sfxVolume"] = settings.sfxVolume;
    data["windowScaleIndex"] = settings.windowScaleIndex;
    data["language"] = settings.language == Language::Chinese ? "zh" : "en";

    std::ofstream file(resolved);
    if (!file.is_open()) return false;
    file << data.dump(2);
    return true;
}

} // namespace cls
