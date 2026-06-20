/**
 * @file main.cpp
 * @brief 校园生活模拟器 — 主入口
 *
 * 操作说明:
 *   WASD/方向键 — 移动玩家
 *   Enter — 场景切换 / 交互点触发
 *   C — 降低 SAN / 清除 buff
 *   F — 与附近敌人战斗
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "core/AssetPath.h"
#include "core/CombatResult.h"
#include "core/GameSettings.h"
#include "core/Localization.h"
#include "core/LibraryConfig.h"
#include "core/MealConfig.h"
#include "core/SceneConfig.h"
#include "core/SceneTransition.h"
#include "core/TextUtils.h"
#include "core/TimeSkipFlash.h"
#include "core/TimeSystem.h"
#include "core/Types.h"
#include "core/WindowScaler.h"
#include "core/CombatSystem.h"
#include "core/GameContext.h"
#include "core/SettlementResolver.h"
#include "event/EventRunner.h"
#include "entity/CombatHelper.h"
#include "interaction/CafeteriaInteraction.h"
#include "interaction/DormitoryInteraction.h"
#include "interaction/RegularInteraction.h"
#include "ui/ActivityNotice.h"
#include "ui/ChoicePrompt.h"
#include "ui/DebugSandboxPanel.h"
#include "ui/ModalBox.h"
#include "ui/TimePanel.h"
#include "entity/Player.h"
#include "entity/Enemy.h"
#include "map/MapPortal.h"
#include "map/BuildingInterior.h"
#include "map/CampusMap.h"
#include "map/DormitoryInterior.h"
#include "map/GymInterior.h"
#include "map/LibraryInterior.h"
#include "map/ClassroomInterior.h"
#include "map/CafeteriaInterior.h"
#include "map/ConvenienceStoreInterior.h"
#include "ui/HUD.h"
#include "ui/TitleScreen.h"
#include "ui/DifficultyPanel.h"
#include "ui/HelpPanel.h"
#include "ui/SceneBackground.h"
#include "ui/SettingsPanel.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <memory>

enum class GameScreen {
    TITLE,
    SETTINGS,
    HELP,
    DIFFICULTY,
    GAME
};

namespace {

constexpr char kLastActivityIdKey[] = "lastActivityId";
constexpr char kActivityStreakKey[] = "activityStreak";
constexpr char kDurationPromptPurpose[] = "activity_duration";
constexpr char kSleepPromptPurpose[] = "sleep_alarm";
constexpr int kMinActivityMinutes = 10;
constexpr int kMaxActivityMinutes = 120;
constexpr int kMinutesPerDay = 24 * 60;
constexpr int kDefaultSleepMinutes = 7 * 60 + 30;
constexpr int kMinimumHealthySleepMinutes = 6 * 60 + 30;
constexpr int kOversleepFloorMinutes = 7 * 60;

int normalizedMinute(int minute) {
    int result = minute % kMinutesPerDay;
    if (result < 0) result += kMinutesPerDay;
    return result;
}

bool isWithinClockWindow(int minute, int start, int end) {
    minute = normalizedMinute(minute);
    start = normalizedMinute(start);
    end = normalizedMinute(end);
    if (start <= end) return minute >= start && minute < end;
    return minute >= start || minute < end;
}

std::string campusPlaceKey(CampusPlace place) {
    switch (place) {
        case CampusPlace::Dormitory: return "dormitory";
        case CampusPlace::Cafeteria: return "cafeteria";
        case CampusPlace::Classroom: return "classroom";
        case CampusPlace::Library: return "library";
        case CampusPlace::Gym: return "gym";
        case CampusPlace::Store: return "store";
        case CampusPlace::Campus: return "campus";
    }
    return "campus";
}

int commuteMinutes(CampusPlace from, CampusPlace to) {
    if (from == to || from == CampusPlace::Campus || to == CampusPlace::Campus)
        return 0;

    const std::string a = campusPlaceKey(from);
    const std::string b = campusPlaceKey(to);
    const auto matches = [&](const std::string& lhs, const std::string& rhs) {
        return (a == lhs && b == rhs) || (a == rhs && b == lhs);
    };

    if (matches("dormitory", "cafeteria")) return 5;
    if (matches("dormitory", "classroom")) return 15;
    if (matches("dormitory", "library")) return 10;
    if (matches("dormitory", "gym")) return 10;
    if (matches("dormitory", "store")) return 5;
    if (matches("cafeteria", "classroom")) return 10;
    if (matches("cafeteria", "library")) return 5;
    if (matches("cafeteria", "gym")) return 5;
    if (matches("cafeteria", "store")) return 5;
    if (matches("classroom", "library")) return 5;
    if (matches("classroom", "gym")) return 10;
    if (matches("classroom", "store")) return 10;
    if (matches("library", "gym")) return 10;
    if (matches("library", "store")) return 5;
    if (matches("gym", "store")) return 5;
    return 10;
}

int repeatedBenefitPercent(int streak) {
    return std::max(40, 100 - (streak - 1) * 20);
}

int repeatedPenaltyPercent(int streak) {
    return std::min(200, 100 + (streak - 1) * 25);
}

int scalePositiveBenefit(int value, int streak) {
    if (value <= 0 || streak <= 1) return value;
    return (value * repeatedBenefitPercent(streak) + 50) / 100;
}

int scalePenaltyMagnitude(int value, int streak) {
    if (value <= 0 || streak <= 1) return value;
    return (value * repeatedPenaltyPercent(streak) + 99) / 100;
}

int scaleNormalDelta(int value, int streak) {
    if (value > 0) return scalePositiveBenefit(value, streak);
    if (value < 0) return -scalePenaltyMagnitude(-value, streak);
    return 0;
}

int scaleBadAccumulationDelta(int value, int streak) {
    if (value > 0) return scalePenaltyMagnitude(value, streak);
    if (value < 0) return -scalePositiveBenefit(-value, streak);
    return 0;
}

Attributes adjustAttributesForRepetition(const Attributes& delta, int streak) {
    if (streak <= 1) return delta;

    Attributes adjusted = delta;
    adjusted.energy = scaleNormalDelta(delta.energy, streak);
    adjusted.health = scaleNormalDelta(delta.health, streak);
    adjusted.gold = delta.gold > 0 ? scalePositiveBenefit(delta.gold, streak) : delta.gold;
    adjusted.san = scaleNormalDelta(delta.san, streak);
    adjusted.academic = scaleNormalDelta(delta.academic, streak);
    adjusted.social = scaleNormalDelta(delta.social, streak);
    return adjusted;
}

bool isScalableHiddenBenefit(const std::string& key) {
    return key == "healthIndex"
        || key == "teacherTrust"
        || key == "friendBond"
        || key == "storeTrust"
        || key == "clubContribution"
        || key == "clubRelation"
        || key == "clubShowcaseScore"
        || key == "innovationProgress"
        || key == "innovationTeamTrust"
        || key == "innovationDefenseScore";
}

bool isScalableHiddenBurden(const std::string& key) {
    return key == "lateNightLevel"
        || key == "gameAddiction";
}

HiddenMap adjustHiddenForRepetition(const HiddenMap& hiddenDelta, int streak) {
    if (streak <= 1 || !hiddenDelta.is_object()) return hiddenDelta;

    HiddenMap adjusted = hiddenDelta;
    for (auto it = adjusted.begin(); it != adjusted.end(); ++it) {
        if (!it.value().is_number_integer()) continue;

        const std::string key = it.key();
        const int value = it.value().get<int>();
        if (isScalableHiddenBenefit(key)) {
            it.value() = scaleNormalDelta(value, streak);
        } else if (isScalableHiddenBurden(key)) {
            it.value() = scaleBadAccumulationDelta(value, streak);
        }
    }
    return adjusted;
}

int scaledIntegerDelta(int value, int actualMinutes, int baseMinutes) {
    if (value == 0 || actualMinutes <= 0 || baseMinutes <= 0) return 0;
    const int sign = value < 0 ? -1 : 1;
    const int magnitude = std::abs(value);
    return sign * ((magnitude * actualMinutes + baseMinutes / 2) / baseMinutes);
}

Attributes scaleAttributesByDuration(const Attributes& delta, int actualMinutes, int baseMinutes) {
    Attributes scaled = delta;
    scaled.energy = scaledIntegerDelta(delta.energy, actualMinutes, baseMinutes);
    scaled.health = scaledIntegerDelta(delta.health, actualMinutes, baseMinutes);
    scaled.gold = scaledIntegerDelta(delta.gold, actualMinutes, baseMinutes);
    scaled.san = scaledIntegerDelta(delta.san, actualMinutes, baseMinutes);
    scaled.academic = scaledIntegerDelta(delta.academic, actualMinutes, baseMinutes);
    scaled.social = scaledIntegerDelta(delta.social, actualMinutes, baseMinutes);
    return scaled;
}

bool isDurationScaledHiddenKey(const std::string& key) {
    return isScalableHiddenBenefit(key) || isScalableHiddenBurden(key);
}

HiddenMap scaleHiddenByDuration(const HiddenMap& hiddenDelta, int actualMinutes, int baseMinutes) {
    if (!hiddenDelta.is_object() || actualMinutes <= 0 || baseMinutes <= 0) return hiddenDelta;

    HiddenMap scaled = hiddenDelta;
    for (auto it = scaled.begin(); it != scaled.end(); ++it) {
        if (it.value().is_number_integer() && isDurationScaledHiddenKey(it.key())) {
            it.value() = scaledIntegerDelta(it.value().get<int>(), actualMinutes, baseMinutes);
        }
    }
    return scaled;
}

std::string formatClockMinute(int minuteOfDay) {
    const int minute = ((minuteOfDay % (24 * 60)) + (24 * 60)) % (24 * 60);
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << (minute / 60)
       << ":" << std::setw(2) << (minute % 60);
    return ss.str();
}

std::string durationLabel(int minutes) {
    if (minutes < 60) return cls::format("activity.duration.minutes", {{"minutes", std::to_string(minutes)}});
    const int hours = minutes / 60;
    const int restMinutes = minutes % 60;
    if (restMinutes == 0) {
        return cls::format("activity.duration.hours", {{"hours", std::to_string(hours)}});
    }
    return cls::format("activity.duration.hours_minutes",
        {{"hours", std::to_string(hours)}, {"minutes", std::to_string(restMinutes)}});
}

std::string activityKeyFor(const std::string& title, const std::string& body) {
    return title + "\n" + body;
}

int updateActivityStreak(HiddenMap& hidden, const std::string& activityKey) {
    if (!hidden.is_object()) hidden = HiddenMap::object();

    std::string lastActivity;
    if (hidden.contains(kLastActivityIdKey) && hidden[kLastActivityIdKey].is_string()) {
        lastActivity = hidden[kLastActivityIdKey].get<std::string>();
    }

    int previousStreak = 0;
    if (hidden.contains(kActivityStreakKey) && hidden[kActivityStreakKey].is_number_integer()) {
        previousStreak = hidden[kActivityStreakKey].get<int>();
    }

    const int streak = lastActivity == activityKey
        ? std::min(previousStreak + 1, 99)
        : 1;
    hidden[kLastActivityIdKey] = activityKey;
    hidden[kActivityStreakKey] = streak;
    return streak;
}

void resetActivityStreak(HiddenMap& hidden) {
    if (!hidden.is_object()) hidden = HiddenMap::object();
    hidden[kLastActivityIdKey] = "none";
    hidden[kActivityStreakKey] = 0;
}

std::string appendRepeatNotice(const std::string& body, int streak) {
    if (streak <= 1) return body;
    return body + "\n\n" + cls::format("activity.repeat_penalty",
        {{"streak", std::to_string(streak)}});
}

} // namespace

// 数据 struct 已拆离至独立 header：
//   CombatResult   → src/core/CombatResult.h
//   SceneTransition → src/core/SceneTransition.h
//   TimeSkipFlash  → src/core/TimeSkipFlash.h
//   ActivityNotice → src/ui/ActivityNotice.h
//   ChoicePrompt   → src/ui/ChoicePrompt.h

// ──────────────────────────────────────────────────────────────
// 根据情绪类型获取对应玩家属性值（用于战斗检定）
// ──────────────────────────────────────────────────────────────
// statForEmotion / actionNameForEmotion 已拆离至 src/entity/CombatHelper.h

// ──────────────────────────────────────────────────────────────
// 渲染当前属性面板（所有模式下都在顶部显示）
// ──────────────────────────────────────────────────────────────
void renderStatsPanel(sf::RenderWindow& window, sf::Font& font,
                      const Player& player) {
    HUD hud(font);
    hud.setPlayer(&player);
    hud.render(window);
}

// renderTimePanel 已拆离至 src/ui/TimePanel

// renderModalBox 已拆离至 src/ui/ModalBox

// renderNotice / renderChoicePrompt 已内联至 main loop，使用 ModalBox 组件

void renderTimeSkipFlash(sf::RenderWindow& window, sf::Font& font, const TimeSkipFlash& flash) {
    if (!flash.active) return;
    sf::RectangleShape blackout({kRenderWidth, kRenderHeight});
    blackout.setFillColor(sf::Color(0, 0, 0, 245));
    window.draw(blackout);

    sf::Text text = cls::makeText(font, flash.text, 20);
    text.setFillColor(sf::Color(230, 230, 220));
    text.setPosition({360.0f, 252.0f});
    window.draw(text);
}

void applyDifficulty(Player& player, Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:
            player.modifyAttributes(Attributes{.energy = 15, .health = 10, .san = 20, .academic = 5, .social = 5});
            break;
        case Difficulty::Normal:
            break;
        case Difficulty::Hard:
            player.modifyAttributes(Attributes{.energy = -10, .health = -5, .san = -20, .social = -5});
            break;
    }
}

void renderSceneTransition(sf::RenderWindow& window, sf::Font& font,
                           SceneBackground& background, const SceneTransition& transition) {
    background.render(window, transition.background, sf::Color(0, 0, 0, 70));

    sf::RectangleShape plate({640.0f, 132.0f});
    plate.setPosition({160.0f, 332.0f});
    plate.setFillColor(sf::Color(9, 22, 28, 176));
    plate.setOutlineColor(sf::Color(230, 212, 148, 170));
    plate.setOutlineThickness(2.0f);
    window.draw(plate);

    sf::Text title = cls::makeText(font, transition.title, 34);
    title.setFillColor(sf::Color(250, 240, 205));
    title.setOutlineColor(sf::Color(18, 42, 45));
    title.setOutlineThickness(2.0f);
    title.setPosition({190.0f, 352.0f});
    window.draw(title);

    sf::Text subtitle = cls::makeText(font, transition.subtitle, 18);
    subtitle.setFillColor(sf::Color(224, 238, 220));
    subtitle.setPosition({190.0f, 407.0f});
    window.draw(subtitle);

    const bool ready = transition.canContinue();
    sf::Text hint = cls::makeText(font, ready ? cls::text("ui.enter_to_enter") : cls::text("ui.entering"), 13);
    hint.setFillColor(ready ? sf::Color(210, 210, 190, 180) : sf::Color(210, 210, 190, 110));
    hint.setPosition({190.0f, 444.0f});
    window.draw(hint);
}

// scene_transition 配置已移至 assets/config/scene_transitions.json

// ──────────────────────────────────────────────────────────────
// main
// ──────────────────────────────────────────────────────────────
int main() {
    constexpr const char* kSettingsPath = "assets/config/settings.json";
    cls::loadDefaultLocales();
    cls::GameSettings gameSettings = cls::loadSettings(kSettingsPath);
#ifdef CLS_LANG_CHINESE
    if (!std::filesystem::exists(cls::resolveAssetPath(kSettingsPath))) {
        gameSettings.language = cls::Language::Chinese;
    }
#endif
    cls::clampSettings(gameSettings);
    cls::setLanguage(gameSettings.language);

    // ── 窗口 ────────────────────────────────────────────────
    const auto& initialWindowSize = cls::windowScalePresets()[gameSettings.windowScaleIndex];
    sf::RenderWindow window(sf::VideoMode({initialWindowSize.width, initialWindowSize.height}),
                            "CampusLifeSimulator - Class Demo");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    // 将 960×540 渲染坐标系映射到当前窗口尺寸
    sf::View gameView(sf::FloatRect({0.0f, 0.0f}, {kRenderWidth, kRenderHeight}));
    window.setView(gameView);

    // ── 字体：优先使用项目内开源 CJK 字体，避免不同电脑中文乱码 ─────
    sf::Font font;
    bool fontOk = false;
    const std::vector<std::string> fontCandidates = {
        "assets/fonts/NotoSansCJKsc-Regular.otf",
#if defined(_WIN32)
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/arial.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
#elif defined(__linux__)
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJKsc-Regular.otf",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
#endif
    };
    for (const auto& path : fontCandidates) {
        const std::string resolved = cls::resolveAssetPath(path);
        if (font.openFromFile(resolved)) {
            std::cout << "[Font] Loaded: " << resolved << std::endl;
            fontOk = true; break;
        } else {
            std::cout << "[Font] Failed: " << resolved << std::endl;
        }
    }
    if (!fontOk) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
    }

    // ── 创建 Entity 对象 ─────────────────────────────────────
    TitleScreen titleScreen(font, "assets/ui/campus_title_bg.png");
    DifficultyPanel difficultyPanel(font);
    SettingsPanel settingsPanel(font);
    HelpPanel helpPanel(font);
    settingsPanel.setSettings(&gameSettings);
    settingsPanel.setOverlayMode(false);
    helpPanel.setOverlayMode(false);
    SceneBackground sceneBackground;
    SceneTransition sceneTransition;
    TimeSystem timeSystem;
    ActivityNotice activityNotice;
    ChoicePrompt mealChoicePrompt;
    EventRunner eventRunner;
    SettlementResolver settlementResolver;
    settlementResolver.load(cls::resolveAssetPath("assets/config/events/endings.json"),
                            cls::resolveAssetPath("assets/config/events/titles.json"));
    TimeSkipFlash timeSkipFlash;
    TimePanel timePanel(font);
    ModalBox modalBox(font);
    DebugSandboxPanel debugSandbox(font);
    GameScreen screen = GameScreen::TITLE;
    Difficulty selectedDifficulty = Difficulty::Normal;
    bool difficultyApplied = false;
    bool settlementActive = false;
    int settlementPage = 0;
    SettlementResult settlementResult;

    struct PendingTimedActivity {
        bool active = false;
        int baseMinutes = 0;
        Attributes delta{};
        HiddenMap hiddenDelta = HiddenMap::object();
        std::string title;
        std::string body;
        std::string activityId;
        bool hasHidden = false;

        void clear() {
            active = false;
            baseMinutes = 0;
            delta = Attributes{};
            hiddenDelta = HiddenMap::object();
            title.clear();
            body.clear();
            activityId.clear();
            hasHidden = false;
        }
    };
    PendingTimedActivity pendingTimedActivity;
    struct PendingSleep {
        bool active = false;

        void clear() {
            active = false;
        }
    };
    PendingSleep pendingSleep;
    int selectedLibraryBook = 0;
    std::array<int, 4> libraryBookProgress = {0, 0, 0, 0};
    auto libraryBooks = loadLibraryConfig(cls::resolveAssetPath("assets/config/library.json"));
    auto mealOptions = loadMealConfig(cls::resolveAssetPath("assets/config/meals.json"));
    for (const auto& entry : std::filesystem::directory_iterator(
             cls::resolveAssetPath("assets/config/events"))) {
        if (entry.path().extension() == ".json")
            eventRunner.loadEvents(entry.path().string());
    }
    int lastMealPickupSlot = -1;
    int gamePlayDay = 1;
    int gamesPlayedToday = 0;

    auto applyRuntimeSettings = [&]() {
        cls::clampSettings(gameSettings);
        cls::setLanguage(gameSettings.language);
        const auto& windowSize = cls::windowScalePresets()[gameSettings.windowScaleIndex];
        cls::applyWindowSize(window, gameView, windowSize.width, windowSize.height);
    };

    auto saveRuntimeSettings = [&]() {
        if (!cls::saveSettings(kSettingsPath, gameSettings)) {
            std::cerr << "[Settings] Failed to save settings: " << kSettingsPath << std::endl;
        }
    };

    auto closeTitlePanel = [&]() {
        settingsPanel.setEditing(false);
        screen = GameScreen::TITLE;
    };

    auto handleTitleAction = [&](TitleAction action) {
        switch (action) {
            case TitleAction::Start:
                screen = GameScreen::DIFFICULTY;
                break;
            case TitleAction::Settings:
                settingsPanel.setEditing(false);
                screen = GameScreen::SETTINGS;
                break;
            case TitleAction::Help:
                screen = GameScreen::HELP;
                break;
            case TitleAction::None:
                break;
        }
    };

    auto handleSettingsAction = [&](SettingsAction action) {
        if (action == SettingsAction::Changed) {
            applyRuntimeSettings();
            saveRuntimeSettings();
        } else if (action == SettingsAction::Close) {
            saveRuntimeSettings();
            closeTitlePanel();
        }
    };

    // ── 地图对象 ───────────────────────────────────────────────
    auto campusMap     = std::make_unique<CampusMap>();
    auto dormitoryMap  = std::make_unique<DormitoryInterior>();
    auto gymMap        = std::make_unique<GymInterior>();
    auto libraryMap    = std::make_unique<LibraryInterior>();
    auto classroomMap  = std::make_unique<ClassroomInterior>();
    auto cafeteriaMap  = std::make_unique<CafeteriaInterior>();
    auto storeMap      = std::make_unique<ConvenienceStoreInterior>();

    // 设置字体
    campusMap->setFont(&font);
    campusMap->setTimeSystem(&timeSystem);
    dormitoryMap->setFont(&font);
    gymMap->setFont(&font);
    libraryMap->setFont(&font);
    classroomMap->setFont(&font);
    cafeteriaMap->setFont(&font);
    storeMap->setFont(&font);

    BuildingInterior* currentMap = campusMap.get();

    auto setCurrentMap = [&](CampusPlace place) -> BuildingInterior* {
        switch (place) {
            case CampusPlace::Campus:    return campusMap.get();
            case CampusPlace::Dormitory: return dormitoryMap.get();
            case CampusPlace::Gym:       return gymMap.get();
            case CampusPlace::Library:   return libraryMap.get();
            case CampusPlace::Classroom: return classroomMap.get();
            case CampusPlace::Cafeteria: return cafeteriaMap.get();
            case CampusPlace::Store:     return storeMap.get();
        }
        return campusMap.get();
    };

    Player player(480.0f, 280.0f);
    player.setName("Protagonist");
    auto initializeHiddenState = [&player]() {
        auto& hidden = player.getHidden();
        hidden = HiddenMap::object();
        hidden["friendStage"] = 0;
        hidden["innovationStage"] = 0;
        hidden["clubStage"] = 0;
        hidden["clubType"] = "none";
        hidden["innovationResult"] = "none";
        hidden["innovationTopic"] = "none";
        hidden["innovationDemoMode"] = "none";
        hidden["innovationSpeaker"] = "none";
        hidden["innovationJoined"] = false;
        hidden["innovationIntel"] = false;
        hidden["innovationLeader"] = false;
        hidden["innovationDemoReady"] = false;
        hidden["sharedNotes"] = false;
        hidden["owedFavor"] = false;
        hidden["teacherTrust"] = 0;
        hidden["friendBond"] = 0;
        hidden["friendHelpCount"] = 0;
        hidden["friendReviewCount"] = 0;
        hidden["friendRefuseCount"] = 0;
        hidden["friendRollCallHelpCount"] = 0;
        hidden["classAttendCount"] = 0;
        hidden["skipClassCount"] = 0;
        hidden["rollCallSavedCount"] = 0;
        hidden["absencePenaltyCount"] = 0;
        hidden["lateCount"] = 0;
        hidden["returnClassCount"] = 0;
        hidden["clubActivityCount"] = 0;
        hidden["clubContribution"] = 0;
        hidden["clubRelation"] = 0;
        hidden["clubShowcaseScore"] = 0;
        hidden["clubShowcaseSuccess"] = false;
        hidden["innovationProgress"] = 0;
        hidden["innovationTeamTrust"] = 0;
        hidden["innovationDefenseScore"] = 0;
        hidden["innovationCrisisCount"] = 0;
        hidden["healthIndex"] = 100;
        hidden["lowEnergyDays"] = 0;
        hidden["lowHealthDays"] = 0;
        hidden[kLastActivityIdKey] = "none";
        hidden[kActivityStreakKey] = 0;
        hidden["lateNightLevel"] = 0;
        hidden["lastSleepMinutes"] = kDefaultSleepMinutes;
        hidden["alarmSleepMinutes"] = kDefaultSleepMinutes;
        hidden["consecutiveNoSleepDays"] = 0;
        hidden["lastSleepDay"] = 0;
        hidden["exerciseCount"] = 0;
        hidden["dailyExerciseDay"] = 0;
        hidden["dailyExerciseCount"] = 0;
        hidden["lastExerciseAbsoluteMinute"] = -999999;
        hidden["partTimeCount"] = 0;
        hidden["storeNightShiftCount"] = 0;
        hidden["mealCount"] = 0;
        hidden["libraryVisitCount"] = 0;
        hidden["gameAddiction"] = 0;
        hidden["storeTrust"] = 0;
        hidden["socialAwkwardCount"] = 0;
        hidden["researchUnlocked"] = false;
        hidden["logicUnlocked"] = false;
        hidden["expressionUnlocked"] = false;
        hidden["campusIntelUnlocked"] = false;
    };
    initializeHiddenState();
    CampusPlace currentPlace = CampusPlace::Campus;
    CampusPlace pendingPlace = CampusPlace::Campus;
    sf::Vector2f pendingSpawnPosition(480.0f, 276.0f);
    bool hasPendingMapTransition = false;

    // 活跃敌人列表（SAN 低时动态生成，不在探索地图上预放置）
    std::vector<std::unique_ptr<Enemy>> activeEnemies;
    CombatResult combatResult;

    // ── 状态 ─────────────────────────────────────────────────
    sf::Clock clock;
    bool keyWasPressed[static_cast<int>(sf::Keyboard::KeyCount)] = {};

    // Lambda: 检测单次按键（防止连发）
    auto justPressed = [&](sf::Keyboard::Key k) -> bool {
        bool pressed = sf::Keyboard::isKeyPressed(k);
        bool prev = keyWasPressed[static_cast<int>(k)];
        keyWasPressed[static_cast<int>(k)] = pressed;
        return pressed && !prev;
    };

    // ── 战斗状态 ─────────────────────────────────────────────
    int spawnCounter = 0;

    // ── GameContext：打包共享状态给各 handler ──────────────────
    GameContext ctx{
        player, currentPlace, currentMap,
        campusMap.get(), dormitoryMap.get(), gymMap.get(),
        libraryMap.get(), classroomMap.get(), cafeteriaMap.get(), storeMap.get(),
        timeSystem, combatResult, timeSkipFlash,
        activityNotice, mealChoicePrompt,
        pendingPlace, pendingSpawnPosition, hasPendingMapTransition,
        activeEnemies, mealOptions, libraryBooks, libraryBookProgress,
        selectedLibraryBook, lastMealPickupSlot,
        gamePlayDay, gamesPlayedToday, spawnCounter,
        {}, {}, {}, {}, {}, {}  // callbacks 稍后注入
    };

    // ── 核心回调（handler 通过 ctx 调用）───────────────────────
    auto showTimedResult = [&ctx](const std::string& title, const std::string& body) {
        std::ostringstream message;
        message << body << "\n" << cls::text("time.current") << ": " << ctx.timeSystem.clockText();
        ctx.activityNotice.show(title, message.str());
    };

    auto checkEventTriggers = [&eventRunner, &ctx](int prev) {
        return eventRunner.checkTriggers(ctx, prev);
    };

    std::function<bool()> maybeFinalizeRun;

    auto buildSettlementBody = [&](const SettlementResult& result, int page) {
        std::ostringstream body;
        if (page == 0) {
            body << (result.ending.nameKey.empty() ? result.ending.name : cls::text(result.ending.nameKey));
            const std::string tagline = result.ending.taglineKey.empty() ? result.ending.tagline : cls::text(result.ending.taglineKey);
            if (!tagline.empty()) body << "\n" << tagline;
            body << "\n\n" << (result.ending.descriptionKey.empty() ? result.ending.description : cls::text(result.ending.descriptionKey));
            body << "\n\n" << cls::text("quest.return_title");
            return body.str();
        }
        if (page == 1) {
            if (result.titles.empty()) {
                body << cls::text("settlement.no_extra_titles");
            } else {
                for (const auto& title : result.titles) {
                    const std::string titleName = title.nameKey.empty() ? title.name : cls::text(title.nameKey);
                    const std::string subtitle = title.subtitleKey.empty() ? title.subtitle : cls::text(title.subtitleKey);
                    body << "- " << titleName;
                    if (!subtitle.empty()) body << "（" << subtitle << "）";
                    body << "\n";
                }
            }
            body << "\n" << cls::text("quest.return_title");
            return body.str();
        }
        body << result.summary << "\n\n" << cls::text("quest.return_title");
        return body.str();
    };

    maybeFinalizeRun = [&]() {
        if (settlementActive) return true;
        normalizeHidden(player.getHidden());
        player.syncDailyCountersFromHidden();
        syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
        settlementResult = settlementResolver.resolveImmediate(player);
        if (settlementResult.resolved) {
            settlementActive = true;
            settlementPage = 0;
            activityNotice.show(cls::text("quest.final_result"), buildSettlementBody(settlementResult, settlementPage));
            return true;
        }
        if (timeSystem.isFinished()) {
            settlementResult = settlementResolver.resolveFinal(player);
            settlementActive = true;
            settlementPage = 0;
            activityNotice.show(cls::text("quest.final_result"), buildSettlementBody(settlementResult, settlementPage));
            return true;
        }
        return false;
    };

    auto executeSleep = [&ctx, &showTimedResult, &maybeFinalizeRun, &settlementActive](int requestedMinutes, bool explicitAlarm) {
        auto& hidden = ctx.player.getHidden();
        normalizeHidden(hidden);

        int targetMinutes = requestedMinutes > 0
            ? requestedMinutes
            : hidden.value("lastSleepMinutes", kDefaultSleepMinutes);
        targetMinutes = std::clamp(targetMinutes, 60, 12 * 60);

        bool overslept = false;
        if (explicitAlarm && targetMinutes < 6 * 60) {
            const int seed = ctx.timeSystem.getDay() * 97 + normalizedMinute(ctx.timeSystem.getMinuteOfDay()) * 13
                + hidden.value("consecutiveNoSleepDays", 0) * 31;
            if (seed % 100 < 65) {
                targetMinutes = kOversleepFloorMinutes;
                overslept = true;
            }
        }

        const int startDay = ctx.timeSystem.getDay();
        const int sleptMinutes = ctx.timeSystem.sleepForMinutes(targetMinutes);
        const int sleptHours = sleptMinutes / 60;
        const int sleptRestMinutes = sleptMinutes % 60;

        int sanGain = std::min(45, sleptMinutes / 12);
        int energyGain = std::min(70, sleptMinutes / 8);
        int healthDelta = 0;
        int lateNightRelief = -5;
        if (sleptMinutes < 6 * 60) {
            sanGain = std::max(0, sanGain - 8);
            energyGain = std::max(5, energyGain - 12);
            healthDelta = -5;
            lateNightRelief = 1;
        } else if (sleptMinutes < kMinimumHealthySleepMinutes) {
            healthDelta = -2;
            lateNightRelief = -2;
        }

        if (startDay != ctx.timeSystem.getDay()) {
            ctx.player.dailyAttributeCheck();
        }

        HiddenMap sleepDelta = HiddenMap::object();
        sleepDelta["lastSleepMinutes"] = sleptMinutes;
        sleepDelta["alarmSleepMinutes"] = targetMinutes;
        sleepDelta["consecutiveNoSleepDays"] = 0;
        sleepDelta["lastSleepDay"] = ctx.timeSystem.getDay();
        sleepDelta["lateNightLevel"] = lateNightRelief;
        if (healthDelta != 0) sleepDelta["healthIndex"] = healthDelta;
        mergeHidden(hidden, sleepDelta);

        ctx.player.modifyAttributes(Attributes{.energy = energyGain, .san = sanGain});
        resetActivityStreak(hidden);
        syncVisibleHealthFromHidden(ctx.player.getAttributes(), hidden);
        ctx.player.setPosition(480.0f, 276.0f);
        ctx.player.stopMovement();
        ctx.currentPlace = CampusPlace::Dormitory;
        ctx.currentMap = ctx.dormitoryMap;
        ctx.gamePlayDay = ctx.timeSystem.getDay();
        ctx.gamesPlayedToday = 0;

        std::ostringstream body;
        if (ctx.timeSystem.isFinished()) {
            body << cls::text("sleep.semester_complete") << "\n";
        }
        body << cls::format("sleep.result",
            {{"hours", std::to_string(sleptHours)},
             {"minutes", std::to_string(sleptRestMinutes)},
             {"energy", std::to_string(energyGain)},
             {"san", std::to_string(sanGain)}});
        if (overslept) {
            body << "\n" << cls::text("sleep.overslept");
        }
        if (sleptMinutes < kMinimumHealthySleepMinutes) {
            body << "\n" << cls::text("sleep.short_penalty");
        }

        ctx.timeSkipFlash.start(cls::text("time.sleeping"));
        maybeFinalizeRun();
        if (!settlementActive) {
            showTimedResult(ctx.timeSystem.isFinished()
                ? cls::text("notice.days_complete")
                : (startDay == ctx.timeSystem.getDay() ? cls::text("sleep.result.title") : cls::text("notice.new_day")),
                body.str());
        }
    };

    auto sleepFromDormitory = [&ctx, &pendingSleep]() {
        if (!ctx.timeSystem.canSleep()) {
            ctx.activityNotice.show(cls::text("notice.too_early"),
                cls::text("sleep.too_early"));
            return;
        }
        auto& hidden = ctx.player.getHidden();
        normalizeHidden(hidden);
        const int previousSleep = hidden.value("lastSleepMinutes", kDefaultSleepMinutes);
        pendingSleep.active = true;
        ctx.mealChoicePrompt.show(
            cls::text("sleep.alarm.title"),
            cls::format("sleep.alarm.body", {{"last", durationLabel(previousSleep)}}),
            std::vector<std::string>{
                cls::text("sleep.alarm.option_default"),
                cls::text("sleep.alarm.option_7_5h"),
                cls::text("sleep.alarm.option_6h"),
                cls::text("sleep.alarm.option_5h")
            },
            kSleepPromptPurpose,
            std::vector<int>{0, kDefaultSleepMinutes, 6 * 60, 5 * 60});
    };

    auto markNoSleepForSkippedDays = [&ctx]() {
        auto& hidden = ctx.player.getHidden();
        normalizeHidden(hidden);
        const int currentDay = ctx.timeSystem.getDay();
        const int lastSleepDay = hidden.value("lastSleepDay", 0);
        if (currentDay > lastSleepDay + 1) {
            HiddenMap delta = HiddenMap::object();
            delta["consecutiveNoSleepDays"] = hidden.value("consecutiveNoSleepDays", 0)
                + (currentDay - lastSleepDay - 1);
            delta["lastSleepDay"] = currentDay - 1;
            mergeHidden(hidden, delta);
        }
    };

    auto executeTimedActivity = [&ctx, &checkEventTriggers, &showTimedResult, &maybeFinalizeRun, &markNoSleepForSkippedDays](
            int baseMinutes, int requestedMinutes, const Attributes& delta,
            const HiddenMap& hiddenDelta, bool hasHidden,
            const std::string& title, const std::string& body,
            const std::string& activityId) {
        const int startMinute = ctx.timeSystem.getMinuteOfDay();
        const int startDay = ctx.timeSystem.getDay();
        int actualMinutes = std::clamp(requestedMinutes, kMinActivityMinutes, kMaxActivityMinutes);
        const int rollCallMinute = ctx.timeSystem.getRollCallMinute();
        const bool interruptedByRollCall = !ctx.timeSystem.isClassPrompted()
            && ctx.currentPlace != CampusPlace::Classroom
            && startDay == ctx.timeSystem.getDay()
            && startMinute < rollCallMinute
            && startMinute + actualMinutes >= rollCallMinute;
        if (interruptedByRollCall) {
            actualMinutes = std::max(0, rollCallMinute - startMinute);
        }

        const int prev = ctx.timeSystem.advanceMinutes(actualMinutes);
        if (ctx.timeSystem.getDay() != startDay) {
            markNoSleepForSkippedDays();
            ctx.player.dailyAttributeCheck();
            ctx.gamePlayDay = ctx.timeSystem.getDay();
            ctx.gamesPlayedToday = 0;
        }
        if (actualMinutes <= 0) {
            if (interruptedByRollCall && checkEventTriggers(prev)) {
                maybeFinalizeRun();
                return;
            }
            showTimedResult(title, body);
            return;
        }

        const std::string activityKey = activityId.empty() ? activityKeyFor(title, body) : activityId;
        const int streak = updateActivityStreak(ctx.player.getHidden(), activityKey);
        const Attributes durationDelta = scaleAttributesByDuration(delta, actualMinutes, baseMinutes);
        const Attributes adjustedDelta = adjustAttributesForRepetition(durationDelta, streak);
        ctx.player.modifyAttributes(adjustedDelta);
        if (hasHidden) {
            const HiddenMap durationHiddenDelta = scaleHiddenByDuration(hiddenDelta, actualMinutes, baseMinutes);
            const HiddenMap adjustedHiddenDelta = adjustHiddenForRepetition(durationHiddenDelta, streak);
            if (!adjustedHiddenDelta.is_null()) {
                mergeHidden(ctx.player.getHidden(), adjustedHiddenDelta);
            }
        }
        syncVisibleHealthFromHidden(ctx.player.getAttributes(), ctx.player.getHidden());
        ctx.timeSkipFlash.start("Time passes...");
        if (maybeFinalizeRun()) return;

        if (interruptedByRollCall) {
            if (checkEventTriggers(prev)) {
                maybeFinalizeRun();
                return;
            }
        }

        std::string resultBody = body;
        resultBody += "\n\n" + cls::format("activity.duration_result",
            {{"minutes", std::to_string(actualMinutes)}, {"target", std::to_string(requestedMinutes)}});
        showTimedResult(title, appendRepeatNotice(resultBody, streak));
        checkEventTriggers(prev);
        maybeFinalizeRun();
    };

    auto promptTimedActivityDuration = [&ctx, &pendingTimedActivity](
            int minutes, const Attributes& delta, const HiddenMap& hiddenDelta, bool hasHidden,
            const std::string& title, const std::string& body, const std::string& activityId) {
        pendingTimedActivity.active = true;
        pendingTimedActivity.baseMinutes = std::max(1, minutes);
        pendingTimedActivity.delta = delta;
        pendingTimedActivity.hiddenDelta = hiddenDelta;
        pendingTimedActivity.title = title;
        pendingTimedActivity.body = body;
        pendingTimedActivity.activityId = activityId;
        pendingTimedActivity.hasHidden = hasHidden;

        const int initialMinutes = std::clamp(
            ((minutes + kMinActivityMinutes / 2) / kMinActivityMinutes) * kMinActivityMinutes,
            kMinActivityMinutes,
            kMaxActivityMinutes);
        ctx.mealChoicePrompt.showRange(
            cls::text("activity.duration.title"),
            cls::format("activity.duration.body",
                {{"activity", title}, {"base", durationLabel(minutes)}}),
            kDurationPromptPurpose,
            initialMinutes,
            kMinActivityMinutes,
            kMaxActivityMinutes,
            kMinActivityMinutes);
    };

    auto runTimedActivity = [&executeTimedActivity, &promptTimedActivityDuration](
            int minutes, const Attributes& delta,
            const std::string& title, const std::string& body,
            const std::string& activityId, bool customDuration) {
        if (customDuration) {
            promptTimedActivityDuration(minutes, delta, HiddenMap::object(), false, title, body, activityId);
            return;
        }
        executeTimedActivity(minutes, minutes, delta, HiddenMap::object(), false, title, body, activityId);
    };

    auto runTimedActivityWithHidden = [&executeTimedActivity, &promptTimedActivityDuration](
            int minutes, const Attributes& delta, const HiddenMap& hiddenDelta,
            const std::string& title, const std::string& body,
            const std::string& activityId, bool customDuration) {
        if (customDuration) {
            promptTimedActivityDuration(minutes, delta, hiddenDelta, true, title, body, activityId);
            return;
        }
        executeTimedActivity(minutes, minutes, delta, hiddenDelta, true, title, body, activityId);
    };

    ctx.runTimedActivity = runTimedActivity;
    ctx.runTimedActivityWithHidden = runTimedActivityWithHidden;
    ctx.showTimedResult = showTimedResult;
    ctx.checkEventTriggers = checkEventTriggers;
    ctx.sleepFromDormitory = sleepFromDormitory;
    ctx.finalizeStateChange = maybeFinalizeRun;

    // ── 薄封装 lambda ──────────────────────────────────────────
    auto trySpawnEnemy = [&ctx]() {
        CombatSystem::trySpawnEnemy(ctx);
    };

    auto fightNearestEnemy = [&ctx]() -> bool {
        return CombatSystem::fightNearestEnemy(ctx);
    };

    auto canEnterPlace = [&ctx](CampusPlace target) {
        const int minute = normalizedMinute(ctx.timeSystem.getMinuteOfDay());
        std::string body;
        switch (target) {
            case CampusPlace::Gym:
                if (!isWithinClockWindow(minute, 9 * 60, 22 * 60)) {
                    body = cls::text("place.closed.gym");
                }
                break;
            case CampusPlace::Library:
                if (!isWithinClockWindow(minute, 7 * 60, 23 * 60)) {
                    body = cls::text("place.closed.library");
                }
                break;
            case CampusPlace::Cafeteria:
                if (!isWithinClockWindow(minute, TimeSystem::kBreakfastStartMinute, 22 * 60)) {
                    body = cls::text("place.closed.cafeteria");
                }
                break;
            default:
                break;
        }
        if (body.empty()) return true;
        ctx.activityNotice.show(cls::text("place.closed.title"), body);
        return false;
    };

    auto startMapTransition = [&ctx, &sceneTransition, &canEnterPlace, &checkEventTriggers,
                               &maybeFinalizeRun, &markNoSleepForSkippedDays](const MapPortal& portal) {
        if (!canEnterPlace(portal.target)) return;

        auto& hidden = ctx.player.getHidden();
        normalizeHidden(hidden);
        CampusPlace commuteFrom = ctx.currentPlace;
        if (ctx.currentPlace == CampusPlace::Campus && hidden.contains("lastIndoorPlace")
            && hidden["lastIndoorPlace"].is_string()) {
            const std::string last = hidden["lastIndoorPlace"].get<std::string>();
            if (last == "dormitory") commuteFrom = CampusPlace::Dormitory;
            else if (last == "cafeteria") commuteFrom = CampusPlace::Cafeteria;
            else if (last == "classroom") commuteFrom = CampusPlace::Classroom;
            else if (last == "library") commuteFrom = CampusPlace::Library;
            else if (last == "gym") commuteFrom = CampusPlace::Gym;
            else if (last == "store") commuteFrom = CampusPlace::Store;
        }

        const bool enteringIndoor = ctx.currentPlace == CampusPlace::Campus && portal.target != CampusPlace::Campus;
        int travelMinutes = enteringIndoor ? commuteMinutes(commuteFrom, portal.target) : 0;
        if (enteringIndoor && portal.target == CampusPlace::Store) {
            travelMinutes = std::max(travelMinutes, 30);
        }
        if (travelMinutes > 0) {
            const int startDay = ctx.timeSystem.getDay();
            const int prev = ctx.timeSystem.advanceMinutes(travelMinutes);
            ctx.timeSkipFlash.start(cls::format("time.commuting", {{"minutes", std::to_string(travelMinutes)}}));
            if (ctx.timeSystem.getDay() != startDay) {
                markNoSleepForSkippedDays();
                ctx.player.dailyAttributeCheck();
                ctx.gamePlayDay = ctx.timeSystem.getDay();
                ctx.gamesPlayedToday = 0;
                if (maybeFinalizeRun()) return;
            }
            if (checkEventTriggers(prev)) {
                maybeFinalizeRun();
                return;
            }
            if (maybeFinalizeRun()) return;
        }

        if (enteringIndoor && portal.target == CampusPlace::Dormitory
            && isWithinClockWindow(normalizedMinute(ctx.timeSystem.getMinuteOfDay()), 0, 6 * 60)) {
            auto& afterTravelHidden = ctx.player.getHidden();
            const int seed = ctx.timeSystem.getDay() * 101 + normalizedMinute(ctx.timeSystem.getMinuteOfDay()) * 17
                + afterTravelHidden.value("lateNightLevel", 0) * 7;
            if (seed % 100 < 35) {
                ctx.player.modifyAttributes(Attributes{.san = -6});
                mergeHidden(afterTravelHidden, HiddenMap{{"lateNightLevel", 2}});
                syncVisibleHealthFromHidden(ctx.player.getAttributes(), afterTravelHidden);
                ctx.activityNotice.show(cls::text("notice.late_return.title"),
                    cls::text("notice.late_return.body"));
                if (maybeFinalizeRun()) return;
            }
        }

        if (portal.target == CampusPlace::Campus && ctx.currentPlace != CampusPlace::Campus) {
            hidden["lastIndoorPlace"] = campusPlaceKey(ctx.currentPlace);
        } else if (portal.target != CampusPlace::Campus) {
            hidden["lastIndoorPlace"] = campusPlaceKey(portal.target);
        }

        ctx.pendingPlace = portal.target;
        ctx.pendingSpawnPosition = portal.spawnPosition;
        ctx.hasPendingMapTransition = true;
        sceneTransition.start(portal.transitionBackground, portal.title, portal.subtitle);
    };

    auto finishSceneTransition = [&ctx, &setCurrentMap, &sceneTransition]() {
        if (ctx.hasPendingMapTransition) {
            ctx.currentPlace = ctx.pendingPlace;
            ctx.currentMap = setCurrentMap(ctx.pendingPlace);
            ctx.player.setPosition(ctx.pendingSpawnPosition.x, ctx.pendingSpawnPosition.y);
            ctx.player.stopMovement();
            ctx.hasPendingMapTransition = false;
        }
        sceneTransition.skip();
    };

    auto resolveMealChoice = [&ctx](int mealIndex) {
        CafeteriaInteraction::resolveMealChoice(ctx, mealIndex);
    };

    auto canTriggerInteraction = [&ctx](const InteractionPoint& ip) {
        const std::string& actionId = ip.actionId;
        const int minute = normalizedMinute(ctx.timeSystem.getMinuteOfDay());
        std::string body;

        if (actionId.rfind("classroom_", 0) == 0
            && isWithinClockWindow(minute, 23 * 60, 7 * 60)) {
            body = cls::text("interaction.closed.classroom_night");
        } else if ((actionId.rfind("gym_treadmill_", 0) == 0
                || actionId.rfind("gym_barbell_", 0) == 0
                || actionId == "gym_front_desk")
            && !isWithinClockWindow(minute, 9 * 60, 22 * 60)) {
            body = cls::text("interaction.closed.gym");
        } else if ((actionId.rfind("library_shelf_", 0) == 0
                || actionId == "library_table")
            && !isWithinClockWindow(minute, 7 * 60, 23 * 60)) {
            body = cls::text("interaction.closed.library");
        } else if ((actionId == "cafeteria_counter"
                || actionId.rfind("cafeteria_table_", 0) == 0)
            && !ctx.timeSystem.isMealTime()) {
            body = cls::text("activity.cafeteria.meal_time_closed");
        }

        if (body.empty()) return true;
        ctx.activityNotice.show(cls::text("interaction.closed.title"), body);
        return false;
    };

    auto handleInteraction = [&ctx, &eventRunner, &canTriggerInteraction](const InteractionPoint& ip) {
        if (!canTriggerInteraction(ip)) return;
        if (eventRunner.triggerByAction(ip.actionId, ctx)) return;
        if (CafeteriaInteraction::handleInteraction(ctx, ip.actionId, ip.displayLabel())) return;
        if (DormitoryInteraction::handle(ctx, ip)) return;
        if (RegularInteraction::handle(ctx, ip)) return;
        ctx.activityNotice.show(ip.displayLabel(), ip.displayDescription());
    };

    // ── 主循环 ───────────────────────────────────────────────
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // 更新战斗结果计时器
        combatResult.update(dt);
        sceneBackground.update(dt);
        sceneTransition.update(dt);
        timeSkipFlash.update(dt);

        // ── 事件处理 ────────────────────────────────────────
        while (const auto eventOpt = window.pollEvent()) {
            const auto& event = *eventOpt;

            if (event.is<sf::Event::Closed>()) {
                window.close();
                return 0;
            }
            if (sceneTransition.active) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (sceneTransition.canContinue()
                        && (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Escape)) {
                        finishSceneTransition();
                    }
                } else if (event.is<sf::Event::MouseButtonPressed>()) {
                    if (sceneTransition.canContinue()) {
                        finishSceneTransition();
                    }
                }
                continue;
            }
            if (screen == GameScreen::GAME) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    const bool sandboxWasExpanded = debugSandbox.isExpanded();
                    const bool sandboxConsumesKey = keyEv->code == sf::Keyboard::Key::F1
                        || (debugSandbox.isEnabled() && keyEv->code == sf::Keyboard::Key::F2)
                        || (sandboxWasExpanded
                            && (keyEv->code == sf::Keyboard::Key::Tab
                            || keyEv->code == sf::Keyboard::Key::Escape
                            || keyEv->code == sf::Keyboard::Key::Up
                            || keyEv->code == sf::Keyboard::Key::Down
                            || keyEv->code == sf::Keyboard::Key::Left
                            || keyEv->code == sf::Keyboard::Key::Right
                            || keyEv->code == sf::Keyboard::Key::W
                            || keyEv->code == sf::Keyboard::Key::A
                            || keyEv->code == sf::Keyboard::Key::S
                            || keyEv->code == sf::Keyboard::Key::D
                            || keyEv->code == sf::Keyboard::Key::Enter
                            || keyEv->code == sf::Keyboard::Key::Space
                            || keyEv->code == sf::Keyboard::Key::R
                            || keyEv->code == sf::Keyboard::Key::E
                            || keyEv->code == sf::Keyboard::Key::Delete));
                    const DebugSandboxResult debugResult = debugSandbox.handleKey(
                        keyEv->code, ctx, eventRunner);
                    if (debugResult.action != DebugSandboxAction::None) {
                        std::cout << "[DebugSandbox] " << debugResult.message << std::endl;
                    }
                    if (sandboxConsumesKey) {
                        continue;
                    }
                }
            }
            if (screen == GameScreen::TITLE) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::A
                        || keyEv->code == sf::Keyboard::Key::Left) {
                        titleScreen.moveSelection(-1);
                    } else if (keyEv->code == sf::Keyboard::Key::D
                        || keyEv->code == sf::Keyboard::Key::Right) {
                        titleScreen.moveSelection(1);
                    } else if (keyEv->code == sf::Keyboard::Key::S) {
                        handleTitleAction(TitleAction::Settings);
                    } else if (keyEv->code == sf::Keyboard::Key::H) {
                        handleTitleAction(TitleAction::Help);
                    } else if (keyEv->code == sf::Keyboard::Key::Enter
                        || keyEv->code == sf::Keyboard::Key::Space) {
                        handleTitleAction(titleScreen.confirmSelection());
                    }
                } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
                    const auto action = titleScreen.handleClick(
                        window.mapPixelToCoords(mouseEv->position, gameView));
                    handleTitleAction(action);
                }
                continue;
            }

            if (screen == GameScreen::SETTINGS) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Escape
                        || keyEv->code == sf::Keyboard::Key::S) {
                        saveRuntimeSettings();
                        closeTitlePanel();
                    } else if (keyEv->code == sf::Keyboard::Key::Up) {
                        settingsPanel.moveSelection(-1);
                    } else if (keyEv->code == sf::Keyboard::Key::Down) {
                        settingsPanel.moveSelection(1);
                    } else if (keyEv->code == sf::Keyboard::Key::Left
                        || keyEv->code == sf::Keyboard::Key::A) {
                        handleSettingsAction(settingsPanel.adjustCurrent(gameSettings, -1));
                    } else if (keyEv->code == sf::Keyboard::Key::Right
                        || keyEv->code == sf::Keyboard::Key::D) {
                        handleSettingsAction(settingsPanel.adjustCurrent(gameSettings, 1));
                    } else if (keyEv->code == sf::Keyboard::Key::Enter
                        || keyEv->code == sf::Keyboard::Key::Space) {
                        handleSettingsAction(settingsPanel.confirmCurrent(gameSettings));
                    }
                } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
                    handleSettingsAction(settingsPanel.handleClick(
                        window.mapPixelToCoords(mouseEv->position, gameView),
                        gameSettings));
                }
                continue;
            }

            if (screen == GameScreen::HELP) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Escape
                        || keyEv->code == sf::Keyboard::Key::H
                        || keyEv->code == sf::Keyboard::Key::Enter
                        || keyEv->code == sf::Keyboard::Key::Space) {
                        closeTitlePanel();
                    }
                } else if (event.is<sf::Event::MouseButtonPressed>()) {
                    closeTitlePanel();
                }
                continue;
            }

            if (screen == GameScreen::DIFFICULTY) {
                auto startGameWithDifficulty = [&](Difficulty difficulty) {
                    selectedDifficulty = difficulty;
                    player.setAttributes(defaultPlayerAttributes());
                    currentPlace = CampusPlace::Campus;
                    currentMap = campusMap.get();
                    player.setPosition(480.0f, 276.0f);
                    timeSystem = TimeSystem();
                    initializeHiddenState();
                    applyDifficulty(player, selectedDifficulty);
                    syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
                    difficultyApplied = true;
                    activityNotice.clear();
                    mealChoicePrompt.clear();
                    pendingTimedActivity.clear();
                    pendingSleep.clear();
                    lastMealPickupSlot = -1;
                    gamePlayDay = timeSystem.getDay();
                    gamesPlayedToday = 0;
                    screen = GameScreen::GAME;
                };

                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Escape) {
                        screen = GameScreen::TITLE;
                    } else if (keyEv->code == sf::Keyboard::Key::Num1) {
                        startGameWithDifficulty(Difficulty::Easy);
                    } else if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Enter) {
                        startGameWithDifficulty(Difficulty::Normal);
                    } else if (keyEv->code == sf::Keyboard::Key::Num3) {
                        startGameWithDifficulty(Difficulty::Hard);
                    }
                } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
                    const auto action = difficultyPanel.handleClick(
                        window.mapPixelToCoords(mouseEv->position, gameView));
                    if (action.type == DifficultyActionType::Back) {
                        screen = GameScreen::TITLE;
                    } else if (action.type == DifficultyActionType::Select) {
                        startGameWithDifficulty(action.difficulty);
                    }
                }
                continue;
            }

            if (timeSkipFlash.active) {
                continue;
            }

            if (eventRunner.isActive()) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>())
                    eventRunner.handleInput(keyEv->code, ctx);
                continue;
            }

            if (screen == GameScreen::GAME && !debugSandbox.isExpanded()
                && !mealChoicePrompt.active && !activityNotice.active) {
                if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEv->button == sf::Mouse::Button::Left) {
                        const sf::Vector2f target = window.mapPixelToCoords(mouseEv->position, gameView);
                        if (timePanel.handleClick(target)) {
                            continue;
                        }
                        player.setMoveTarget(target);
                    }
                    continue;
                }
            }

            if (mealChoicePrompt.active) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (mealChoicePrompt.purpose == kDurationPromptPurpose) {
                        if (keyEv->code == sf::Keyboard::Key::Left || keyEv->code == sf::Keyboard::Key::A) {
                            mealChoicePrompt.selectedValue = std::max(
                                mealChoicePrompt.minValue,
                                mealChoicePrompt.selectedValue - mealChoicePrompt.stepValue);
                        } else if (keyEv->code == sf::Keyboard::Key::Right || keyEv->code == sf::Keyboard::Key::D) {
                            mealChoicePrompt.selectedValue = std::min(
                                mealChoicePrompt.maxValue,
                                mealChoicePrompt.selectedValue + mealChoicePrompt.stepValue);
                        } else if (keyEv->code == sf::Keyboard::Key::Enter
                            || keyEv->code == sf::Keyboard::Key::Space) {
                            if (pendingTimedActivity.active) {
                                const int selectedMinutes = mealChoicePrompt.selectedValue;
                                const auto pending = pendingTimedActivity;
                                pendingTimedActivity.clear();
                                mealChoicePrompt.clear();
                                executeTimedActivity(
                                    pending.baseMinutes, selectedMinutes, pending.delta,
                                    pending.hiddenDelta, pending.hasHidden,
                                    pending.title, pending.body, pending.activityId);
                            }
                        } else if (keyEv->code == sf::Keyboard::Key::Escape) {
                            pendingTimedActivity.clear();
                            mealChoicePrompt.clear();
                        }
                        continue;
                    }

                    if (mealChoicePrompt.purpose == kSleepPromptPurpose) {
                        auto choiceIndex = [&]() {
                            if (keyEv->code == sf::Keyboard::Key::Num1 || keyEv->code == sf::Keyboard::Key::Numpad1) return 0;
                            if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Numpad2) return 1;
                            if (keyEv->code == sf::Keyboard::Key::Num3 || keyEv->code == sf::Keyboard::Key::Numpad3) return 2;
                            if (keyEv->code == sf::Keyboard::Key::Num4 || keyEv->code == sf::Keyboard::Key::Numpad4) return 3;
                            return -1;
                        }();
                        if (choiceIndex >= 0 && pendingSleep.active
                            && choiceIndex < static_cast<int>(mealChoicePrompt.values.size())) {
                            const int selectedMinutes = mealChoicePrompt.values[choiceIndex];
                            const bool explicitAlarm = selectedMinutes > 0;
                            pendingSleep.clear();
                            mealChoicePrompt.clear();
                            executeSleep(selectedMinutes, explicitAlarm);
                        } else if (keyEv->code == sf::Keyboard::Key::Escape) {
                            pendingSleep.clear();
                            mealChoicePrompt.clear();
                        }
                        continue;
                    }

                    auto choiceIndex = [&]() {
                        if (keyEv->code == sf::Keyboard::Key::Num1 || keyEv->code == sf::Keyboard::Key::Numpad1) return 0;
                        if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Numpad2) return 1;
                        if (keyEv->code == sf::Keyboard::Key::Num3 || keyEv->code == sf::Keyboard::Key::Numpad3) return 2;
                        if (keyEv->code == sf::Keyboard::Key::Num4 || keyEv->code == sf::Keyboard::Key::Numpad4) return 3;
                        return -1;
                    }();
                    if (choiceIndex >= 0) {
                        resolveMealChoice(choiceIndex);
                    } else if (keyEv->code == sf::Keyboard::Key::Escape) {
                        pendingTimedActivity.clear();
                        pendingSleep.clear();
                        mealChoicePrompt.clear();
                    }
                }
                continue;
            }

            if (activityNotice.active) {
                if (settlementActive) {
                    if (justPressed(sf::Keyboard::Key::Enter)
                        || justPressed(sf::Keyboard::Key::Escape)) {
                        if (settlementPage < 2) {
                            ++settlementPage;
                            activityNotice.show(
                                settlementPage == 1 ? cls::text("quest.earned_titles") : cls::text("quest.semester_summary"),
                                buildSettlementBody(settlementResult, settlementPage));
                        } else {
                            settlementActive = false;
                            screen = GameScreen::TITLE;
                            activityNotice.clear();
                        }
                    }
                } else if (justPressed(sf::Keyboard::Key::Enter)
                    || justPressed(sf::Keyboard::Key::Escape)) {
                    activityNotice.clear();
                }
                continue;
            }

        }

        // ── 持续性输入(移动) ─────────────────────────────────
        if (screen == GameScreen::TITLE) {
            titleScreen.update(dt);
            window.clear(sf::Color(20, 20, 30));
            titleScreen.render(window);
            window.display();
            continue;
        }

        if (screen == GameScreen::SETTINGS) {
            settingsPanel.update(dt);
            window.clear(sf::Color(20, 20, 30));
            settingsPanel.render(window);
            window.display();
            continue;
        }

        if (screen == GameScreen::HELP) {
            helpPanel.update(dt);
            window.clear(sf::Color(20, 20, 30));
            helpPanel.render(window);
            window.display();
            continue;
        }

        if (screen == GameScreen::DIFFICULTY) {
            window.clear(sf::Color(20, 20, 30));
            difficultyPanel.render(window);
            window.display();
            continue;
        }

        if (sceneTransition.active) {
            window.clear(sf::Color(20, 20, 30));
            renderSceneTransition(window, font, sceneBackground, sceneTransition);
            window.display();
            continue;
        }

        if (timeSkipFlash.active) {
            window.clear(sf::Color(0, 0, 0));
            renderTimeSkipFlash(window, font, timeSkipFlash);
            window.display();
            continue;
        }

        if (!debugSandbox.isExpanded() && !mealChoicePrompt.active && !activityNotice.active) {
            float dx = 0.0f, dy = 0.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    dy = -1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  dy = 1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dx = -1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dx = 1.0f;

            const bool hasKeyboardMove = dx != 0.0f || dy != 0.0f;
            if (hasKeyboardMove) {
                player.clearMoveTarget();
                if (dx != 0.0f && dy != 0.0f) {
                    float inv = 1.0f / std::sqrt(2.0f);
                    dx *= inv;
                    dy *= inv;
                }
                player.move(dx, dy, dt);
            } else {
                player.moveToTarget(dt);
            }

            // 按键 C = SAN 冲击（降低 SAN，触发敌人出现）
            if (justPressed(sf::Keyboard::Key::C)) {
                player.modifyAttributes(Attributes{.san = -15});
                maybeFinalizeRun();
                int lvl = player.getSanLevel();
                std::cout << "[Stress] SAN dropped to " << player.getAttributes().san
                          << " Level=" << lvl << std::endl;
                // 尝试生成敌人（SAN 越低概率越大）
                trySpawnEnemy();
            }

            // 按键 F = 与附近敌人战斗
            if (justPressed(sf::Keyboard::Key::F)) {
                if (!fightNearestEnemy()) {
                    std::cout << "[Combat] No enemy nearby! Get closer or spawn one first (press C)." << std::endl;
                } else {
                    maybeFinalizeRun();
                }
            }

            // 按键 V = 恢复 SAN（模拟休息/自我关怀）
            if (justPressed(sf::Keyboard::Key::V)) {
                player.modifyAttributes(Attributes{.san = 15});
                maybeFinalizeRun();
                int lvl = player.getSanLevel();
                // 恢复后重新缩放已生成的敌人
                for (auto& e : activeEnemies) {
                    e->scaleWithSanLevel(lvl);
                }
                std::cout << "[Rest] SAN restored to " << player.getAttributes().san
                          << " Level=" << lvl << std::endl;
                // SAN 恢复后部分敌人消失
                if (lvl == 0) {
                    activeEnemies.clear();
                    std::cout << "[Combat] All enemies retreated (SAN >= 30)" << std::endl;
                } else if (lvl == 1 && static_cast<int>(activeEnemies.size()) > 1) {
                    activeEnemies.resize(1);
                    std::cout << "[Combat] Enemies reduced to 1 (SAN 20-29)" << std::endl;
                }
            }

            // 按键 X = 设置战斗 buff
            if (justPressed(sf::Keyboard::Key::X)) {
                player.getCombatBuffs().nextEventPositive = true;
                player.getCombatBuffs().nextRollModifier = 2;
                std::cout << "[Buff] Victory buff set: +2 to rolls" << std::endl;
            }

            if (justPressed(sf::Keyboard::Key::Enter) && !activityNotice.active) {
                // 第一层：场景传送门（进出建筑）
                bool portalFound = false;
                for (const auto& portal : currentMap->getPortals()) {
                    static constexpr float kPortalMargin = 16.0f;
                    const sf::FloatRect expanded(
                        portal.area.position - sf::Vector2f(kPortalMargin, kPortalMargin),
                        portal.area.size + sf::Vector2f(kPortalMargin * 2, kPortalMargin * 2));
                    if (pointInRect(player.getPosition(), expanded)) {
                        startMapTransition(portal);
                        portalFound = true;
                        break;
                    }
                }
                // 第二层：家具交互点
                if (!portalFound) {
                    const InteractionPoint* ip = currentMap->getInteractionAt(player.getPosition());
                    if (ip) {
                        std::cout << "[Interact] " << ip->label
                                  << " (" << ip->actionId << ")" << std::endl;
                        handleInteraction(*ip);
                    }
                }
            }

            player.update(dt);

            // 边界限制，防止走出地图
            currentMap->clampPlayer(player);
            currentMap->resolveCollisions(player);
        }

        // ── 渲染 ──────────────────────────────────────────────
        window.clear(sf::Color(20, 20, 30));

        currentMap->render(window);
        player.render(window);

        // 顶部属性面板
        if (fontOk) {
            renderStatsPanel(window, font, player);
            timePanel.setTimeSystem(&timeSystem);
            timePanel.render(window);
        }
        if (eventRunner.isActive()) {
            eventRunner.render(window, modalBox);
        }
        if (activityNotice.active) {
            if (settlementActive) {
                const std::string title = settlementPage == 0
                    ? cls::text("quest.final_result")
                    : (settlementPage == 1 ? cls::text("quest.earned_titles") : cls::text("quest.semester_summary"));
                modalBox.setContent(title, buildSettlementBody(settlementResult, settlementPage),
                                    cls::text("quest.return_title"));
            } else {
                modalBox.setContent(activityNotice.title, activityNotice.body,
                                    cls::text("ui.press_enter_continue"));
            }
            modalBox.render(window);
        }
        if (mealChoicePrompt.active) {
            std::ostringstream body;
            std::string footer;
            if (mealChoicePrompt.purpose == kDurationPromptPurpose) {
                body << mealChoicePrompt.body << "\n\n"
                     << cls::format("activity.duration.current",
                         {{"duration", durationLabel(mealChoicePrompt.selectedValue)}});
                footer = cls::text("activity.duration.footer");
            } else {
                body << mealChoicePrompt.body << "\n\n"
                     << "[1] " << mealChoicePrompt.first
                     << "\n[2] " << mealChoicePrompt.second;
                if (!mealChoicePrompt.third.empty())
                    body << "\n[3] " << mealChoicePrompt.third;
                if (!mealChoicePrompt.fourth.empty())
                    body << "\n[4] " << mealChoicePrompt.fourth;
                footer = !mealChoicePrompt.fourth.empty()
                    ? cls::text("prompt.choice1234")
                    : (mealChoicePrompt.third.empty() ? cls::text("prompt.choice12") : cls::text("prompt.choice123"));
            }
            modalBox.setContent(mealChoicePrompt.title, body.str(),
                                footer);
            modalBox.render(window);
        }
        if (fontOk) {
            debugSandbox.refresh(ctx, eventRunner);
            debugSandbox.render(window);
        }

        window.display();
    }

    return 0;
}

