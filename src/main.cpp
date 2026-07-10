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

#include "audio/AudioManager.h"
#include "core/ActivityRunner.h"
#include "utils/AssetPath.h"
#include "core/GameSettings.h"
#include "core/SleepSystem.h"
#include "core/Localization.h"
#include "config/LibraryConfig.h"
#include "config/MealConfig.h"
#include "config/SceneConfig.h"
#include "utils/TextUtils.h"
#include "core/TimeSystem.h"
#include "core/Types.h"
#include "utils/WindowScaler.h"
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
#include "quest/QuestManager.h"
#include "quest/MainQuest.h"
#include "quest/SimpleQuest.h"
#include "quest/ExamQuest.h"
#include "ui/TimePanel.h"
#include "entity/Player.h"

constexpr char kDormitoryDeskPromptPurpose[] = "dormitory_desk_choice";
constexpr char kMorningRushPromptPurpose[] = "morning_rush_choice";
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
#include "ui/TguiContext.h"
#include "ui/TguiTheme.h"
#include "minigame/MorningRushGame.h"
#include "minigame/MorningRushRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <functional>
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


// 数据 struct 已拆离至独立 header：
//   CombatResult / SceneTransition / TimeSkipFlash → src/core/Types.h
//   ActivityNotice → src/ui/ActivityNotice.h
//   ChoicePrompt   → src/ui/ChoicePrompt.h
//   CombatHelper   → src/entity/CombatHelper.h

// ──────────────────────────────────────────────────────────────
// 渲染当前属性面板（所有模式下都在顶部显示）
// ──────────────────────────────────────────────────────────────
void renderStatsPanel(sf::RenderWindow&, HUD& hud, const Player& player) {
    hud.setPlayer(&player);
    hud.update(0.0f);  // 刷新 TGUI bars
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

// ── Quest 渲染 ─────────────────────────────────────────────
void renderQuestContent(sf::RenderWindow& window, ModalBox& modalBox, MainQuest* quest) {
    if (!quest || quest->isCompleted()) return;

    std::string title = quest->getQuestName();
    std::string body;
    std::string footer;

    // SimpleQuest / ExamQuest 各自渲染
    if (auto* exam = dynamic_cast<ExamQuest*>(quest)) {
        switch (exam->getCurrentPhase()) {
            case QuestPhase::ANNOUNCEMENT:
                body = exam->getDescription();
                footer = cls::text("ui.press_enter_continue");
                break;
            case QuestPhase::PREPARATION: {
                body = exam->getDescription() + "\n\n"
                     + cls::text("quest_chain.exam.review_prompt");
                body += "\n  ["
                     + std::string(exam->getHasReviewed() ? "x" : " ")
                     + "] " + cls::text("quest_chain.exam.review_yes")
                     + " (" + cls::text("quest_chain.exam.review_cost")
                     + ": " + std::to_string(exam->getReviewEnergyCost()) + " "
                     + cls::text("quest_chain.exam.review_bonus_label")
                     + ": +" + std::to_string(exam->getReviewBonus()) + ")";
                footer = cls::text("quest_chain.exam.review_footer");
                break;
            }
            case QuestPhase::EXAM_ROUND: {
                body = cls::format("quest_chain.exam.round_info",
                    {{"round", std::to_string(exam->getCurrentRound())},
                     {"total", std::to_string(exam->getTotalRounds())},
                     {"score", std::to_string(exam->getScore())},
                     {"required", std::to_string(exam->getRequiredPasses())},
                     {"dc", std::to_string(exam->getSubjectDC())}});
                footer = cls::text("quest_chain.exam.roll_prompt");
                break;
            }
            case QuestPhase::ROUND_RESULT: {
                const auto& roll = exam->getLastRoll();
                body = cls::format("quest_chain.exam.roll_result",
                    {{"d20", std::to_string(roll.d20Roll)},
                     {"bonus", std::to_string(roll.academicBonus)},
                     {"review", std::to_string(roll.reviewBonus)},
                     {"total", std::to_string(roll.total)},
                     {"dc", std::to_string(roll.dc)}});
                body += "\n" + std::string(roll.success
                    ? cls::text("quest_chain.exam.round_pass")
                    : cls::text("quest_chain.exam.round_fail"));
                footer = cls::text("ui.press_enter_continue");
                break;
            }
            case QuestPhase::FINAL_RESULT:
                body = exam->getPassed()
                    ? cls::text("quest_chain.exam.passed")
                    : cls::text("quest_chain.exam.failed");
                footer = cls::text("ui.press_enter_continue");
                break;
            default:
                footer = cls::text("ui.press_enter_continue");
                break;
        }
    } else {
        // SimpleQuest
        switch (quest->getCurrentPhase()) {
            case QuestPhase::ANNOUNCEMENT:
                body = quest->getDescription();
                footer = cls::text("ui.press_enter_continue");
                break;
            case QuestPhase::CHOICE: {
                body = quest->getDescription() + "\n\n";
                const auto& choices = quest->getChoiceTexts();
                for (size_t i = 0; i < choices.size(); ++i) {
                    body += "[" + std::to_string(i + 1) + "] " + choices[i];
                    if (i + 1 < choices.size()) body += "\n";
                }
                footer = cls::text("prompt.choice" + std::to_string(std::min(choices.size(), size_t(4))));
                break;
            }
            case QuestPhase::FINAL_RESULT: {
                int idx = quest->getSelectedChoiceIndex();
                if (idx >= 0 && idx < static_cast<int>(quest->getChoiceTexts().size())) {
                    body = quest->getChoiceTexts()[idx];
                }
                footer = cls::text("ui.press_enter_continue");
                break;
            }
            default:
                footer = cls::text("ui.press_enter_continue");
                break;
        }
    }

    modalBox.setContent(title, body, footer);
    modalBox.render(window);
}

namespace {

std::string settlementText(const std::string& key, const std::string& fallback) {
    return key.empty() ? fallback : cls::text(key);
}

void appendEndingLine(std::ostringstream& body, const EndingDefinition& ending) {
    body << settlementText(ending.nameKey, ending.name);
    const std::string tagline = settlementText(ending.taglineKey, ending.tagline);
    if (!tagline.empty()) body << " - " << tagline;
}

const EndingDefinition* primaryEndingFor(const SettlementResult& result) {
    if (!result.achievedEndings.empty()) return &result.achievedEndings.front();
    if (!result.ending.id.empty()) return &result.ending;
    return nullptr;
}

} // namespace


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

    // ── 隐藏变量配置 ──────────────────────────────────────────
    HiddenVariableConfig hiddenConfig;
    if (!hiddenConfig.loadFromFile(cls::resolveAssetPath("assets/config/hidden_variables.json"))) {
        std::cerr << "[HiddenVars] Failed to load hidden_variables.json" << std::endl;
        return 1;
    }
    initHiddenVariableConfig(hiddenConfig);

    // ── 窗口 ────────────────────────────────────────────────
    const auto& initialWindowSize = cls::windowScalePresets()[gameSettings.windowScaleIndex];
    sf::RenderWindow window(sf::VideoMode({initialWindowSize.width, initialWindowSize.height}),
                            "CampusLifeSimulator - Class Demo");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    // 将 960×540 渲染坐标系映射到当前窗口尺寸
    sf::View gameView(sf::FloatRect({0.0f, 0.0f}, {kRenderWidth, kRenderHeight}));
    window.setView(gameView);

    // TGUI 适配层
    TguiContext tguiCtx(window);

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
    std::string resolvedFontPath;
    for (const auto& path : fontCandidates) {
        const std::string resolved = cls::resolveAssetPath(path);
        if (font.openFromFile(resolved)) {
            std::cout << "[Font] Loaded: " << resolved << std::endl;
            fontOk = true;
            resolvedFontPath = resolved;
            break;
        } else {
            std::cout << "[Font] Failed: " << resolved << std::endl;
        }
    }
    if (!fontOk) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
    } else {
        cls::tgui_theme::loadGlobalFont(resolvedFontPath);
    }

    // ── 创建 Entity 对象 ─────────────────────────────────────
    TitleScreen titleScreen(font, "assets/image/ui/title/campus_title_bg.png");
    DifficultyPanel difficultyPanel(font);
    SettingsPanel settingsPanel(font);
    HelpPanel helpPanel(font);
    settingsPanel.setSettings(&gameSettings);
    settingsPanel.setOverlayMode(false);
    helpPanel.setOverlayMode(false);
    SceneBackground sceneBackground;
    SceneTransition sceneTransition;
    TimeSystem timeSystem;
    cls::AudioManager audioManager;
    ActivityNotice activityNotice;
    ChoicePrompt mealChoicePrompt;
    HUD hud(font);
    MorningRushGame morningRushGame;
    MorningRushRenderer morningRushRenderer(font, morningRushGame);
    EventRunner eventRunner;
    SettlementResolver settlementResolver;
    settlementResolver.load(cls::resolveAssetPath("assets/config/events/endings.json"),
                            cls::resolveAssetPath("assets/config/events/titles.json"));
    QuestManager questManager;
    questManager.loadQuestChain(cls::resolveAssetPath("assets/config/quests.json"));
    bool questActive = false;
    TimeSkipFlash timeSkipFlash;
    TimePanel timePanel(font);
    ModalBox modalBox(font);
    sf::Texture endingGoodTexture;
    sf::Texture endingBadTexture;
    const bool endingGoodLoaded = endingGoodTexture.loadFromFile(cls::resolveAssetPath("assets/image/ui/endings/good_ending.png"));
    const bool endingBadLoaded = endingBadTexture.loadFromFile(cls::resolveAssetPath("assets/image/ui/endings/bad_ending.png"));
    (void)endingGoodLoaded;
    (void)endingBadLoaded;
    DebugSandboxPanel debugSandbox(font);
    GameScreen screen = GameScreen::TITLE;
    GameScreen settingsReturnScreen = GameScreen::TITLE;
    Difficulty selectedDifficulty = Difficulty::Normal;
    bool difficultyApplied = false;
    bool settlementActive = false;
    int settlementPage = 0;
    SettlementResult settlementResult;

    PendingTimedActivity pendingTimedActivity;
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
    bool morningRushActive = false;
    bool morningRushTestMode = false;
    bool morningRushTriggeredToday = false;
    bool morningRushPromptedToday = false;
    int morningRushCooldownDays = 0;
    bool pendingMorningRushShortSleep = false;
    bool pendingMorningRushBreakfastRisk = false;
    int pendingMorningRushPenalty = 0;
    MorningRushRoute pendingMorningRushRoute = MorningRushRoute::Campus;
    int morningRushTeacherTrust = 0;

    auto hideModalUi = [&]() {
        modalBox.setFullscreenTexture(nullptr);
        modalBox.setVisible(false);
    };

    if (!audioManager.initialize())
        std::cerr << "[Audio] Failed to initialize audio system" << std::endl;
    audioManager.applyVolume(gameSettings);

    auto applyRuntimeSettings = [&]() {
        cls::clampSettings(gameSettings);
        cls::setLanguage(gameSettings.language);
        audioManager.applyVolume(gameSettings);
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
        settingsPanel.setVisible(false);
        helpPanel.setVisible(false);
        screen = GameScreen::TITLE;
    };

    auto openSettingsPanel = [&](GameScreen returnScreen) {
        settingsReturnScreen = returnScreen;
        settingsPanel.setEditing(false);
        settingsPanel.setOverlayMode(returnScreen == GameScreen::GAME);
        titleScreen.setVisible(false);
        helpPanel.setVisible(false);
        screen = GameScreen::SETTINGS;
    };

    auto closeSettingsPanel = [&]() {
        settingsPanel.setEditing(false);
        settingsPanel.setVisible(false);
        helpPanel.setVisible(false);
        settingsPanel.setOverlayMode(false);
        screen = settingsReturnScreen;
        settingsReturnScreen = GameScreen::TITLE;
    };

    auto isSettingsShortcut = [](const sf::Event::KeyPressed& keyEv) {
        return keyEv.control && keyEv.code == sf::Keyboard::Key::S;
    };

    auto isFinishShortcut = [](const sf::Event::KeyPressed& keyEv) {
        return keyEv.control && keyEv.code == sf::Keyboard::Key::H;
    };

    auto handleTitleAction = [&](TitleAction action) {
        switch (action) {
            case TitleAction::Start:
                titleScreen.setVisible(false);
                screen = GameScreen::DIFFICULTY;
                break;
            case TitleAction::Settings:
                openSettingsPanel(GameScreen::TITLE);
                break;
            case TitleAction::Help:
                titleScreen.setVisible(false);
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
            closeSettingsPanel();
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

    CampusPlace currentPlace = CampusPlace::Campus;
    Player player(480.0f, 280.0f);
    player.setName("Protagonist");
    initializeHiddenState(player.getHidden());

    auto startGameWithDifficulty = [&](Difficulty difficulty) {
        selectedDifficulty = difficulty;
        player.setAttributes(defaultPlayerAttributes());
        currentPlace = CampusPlace::Campus;
        currentMap = campusMap.get();
        player.setPosition(480.0f, 276.0f);
        timeSystem = TimeSystem();
        initializeHiddenState(player.getHidden());
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
        morningRushActive = false;
        morningRushTestMode = false;
        morningRushTriggeredToday = false;
        morningRushPromptedToday = false;
        morningRushCooldownDays = 0;
        pendingMorningRushShortSleep = false;
        pendingMorningRushBreakfastRisk = false;
        pendingMorningRushPenalty = 0;
        pendingMorningRushRoute = MorningRushRoute::Campus;
        morningRushTeacherTrust = 0;
        difficultyPanel.setVisible(false);
        screen = GameScreen::GAME;
    };

    difficultyPanel.attachToGui(tguiCtx);
    difficultyPanel.setOnAction([&](DifficultyAction action) {
        if (action.type == DifficultyActionType::Back) {
            difficultyPanel.setVisible(false);
            screen = GameScreen::TITLE;
        } else if (action.type == DifficultyActionType::Select) {
            startGameWithDifficulty(action.difficulty);
        }
    });

    // ── TGUI 初始化 ──────────────────────────────────────────
    helpPanel.attachToGui(tguiCtx);
    helpPanel.setOnClose([&]() { closeTitlePanel(); });
    titleScreen.attachToGui(tguiCtx);
    titleScreen.setOnAction([&](TitleAction action) { handleTitleAction(action); });
    settingsPanel.attachToGui(tguiCtx);
    settingsPanel.setOnAction([&](SettingsAction action) { handleSettingsAction(action); });
    hud.attachToGui(tguiCtx);
    hud.setVisible(true);
    timePanel.attachToGui(tguiCtx);
    timePanel.setVisible(true);
    timePanel.setAlwaysExpanded(true);
    modalBox.attachToGui(tguiCtx);

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

    std::function<bool()> maybeFinalizeRun;

    auto hiddenInt = [&](const std::string& key, int fallback = 0) {
        auto& hidden = player.getHidden();
        normalizeHidden(hidden);
        return hidden.value(key, fallback);
    };
    auto setHiddenInt = [&](const std::string& key, int value) {
        auto& hidden = player.getHidden();
        normalizeHidden(hidden);
        hidden[key] = value;
    };
    auto addHiddenInt = [&](const std::string& key, int delta) {
        setHiddenInt(key, hiddenInt(key) + delta);
    };
    auto syncMorningRushTeacherTrustFromHidden = [&]() {
        morningRushTeacherTrust = hiddenInt("teacherTrust");
    };
    auto syncMorningRushTeacherTrustToHidden = [&]() {
        setHiddenInt("teacherTrust", morningRushTeacherTrust);
    };

    auto movePlayerToClassroom = [&]() {
        currentPlace = CampusPlace::Classroom;
        currentMap = classroomMap.get();
        ctx.currentPlace = currentPlace;
        ctx.currentMap = currentMap;
        ctx.player.stopMovement();
        ctx.player.setPosition(480.0f, 276.0f);
    };

    auto routeForMorningRush = [](CampusPlace place) {
        switch (place) {
            case CampusPlace::Dormitory: return MorningRushRoute::Dormitory;
            case CampusPlace::Cafeteria: return MorningRushRoute::Cafeteria;
            case CampusPlace::Library: return MorningRushRoute::Library;
            case CampusPlace::Gym:
            case CampusPlace::Campus:
            case CampusPlace::Store:
            case CampusPlace::Classroom:
                return MorningRushRoute::Campus;
        }
        return MorningRushRoute::Campus;
    };

    auto settleOrdinaryMorningLate = [&](const std::string& title, const std::string& reason) {
        mealChoicePrompt.clear();
        morningRushActive = false;
        movePlayerToClassroom();
        timeSystem.setTimeAbsolute(std::max(normalizedMinute(timeSystem.getMinuteOfDay()),
                                            TimeSystem::kClassMinute + 5));
        timeSystem.markClassPrompted();
        timeSystem.markClassResolved();
        addHiddenInt("lateCount", 1);
        addHiddenInt("teacherTrust", -2);
        player.modifyAttributes(Attributes{.san = -3, .academic = -2});
        syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
        std::ostringstream body;
        body << reason << "\nSAN -3, Academic -2, Teacher Trust -2.";
        activityNotice.show(title, body.str());
        if (maybeFinalizeRun) maybeFinalizeRun();
    };

    auto startMorningRushRun = [&]() {
        mealChoicePrompt.clear();
        activityNotice.clear();
        hideModalUi();
        morningRushActive = true;
        morningRushTestMode = false;
        morningRushTriggeredToday = true;
        syncMorningRushTeacherTrustFromHidden();
        morningRushGame.start(timeSystem.getDay(),
                              pendingMorningRushShortSleep,
                              pendingMorningRushBreakfastRisk,
                              pendingMorningRushPenalty,
                              pendingMorningRushRoute);
    };

    auto resolveMorningRushChoice = [&](int choice) {
        if (choice == 1) {
            startMorningRushRun();
            return;
        }
        if (choice == 2) {
            settleOrdinaryMorningLate("Late Arrival",
                "You decide not to sprint and arrive after roll call.");
            return;
        }

        mealChoicePrompt.clear();
        const int roll = std::rand() % 20 + 1;
        const int socialBonus = (player.getAttributes().social - 50) / 10;
        const int total = roll + socialBonus - pendingMorningRushPenalty;
        constexpr int dc = 11;
        std::ostringstream body;
        body << "d20 " << roll << " + Social " << socialBonus
             << " - Penalty " << pendingMorningRushPenalty
             << " = " << total << " vs DC " << dc << ".";
        if (total >= dc) {
            movePlayerToClassroom();
            timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
            timeSystem.markClassPrompted();
            timeSystem.markClassResolved();
            mergeHidden(player.getHidden(), HiddenMap{{"classAttendCount", 1}, {"teacherTrust", 1}});
            syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
            body << "\nA classmate saves a seat and covers for you. SAN +1, Energy -2, Social +1.";
            player.modifyAttributes(Attributes{.energy = -2, .san = 1, .social = 1});
            activityNotice.show("Classmate Help", body.str());
            if (maybeFinalizeRun) maybeFinalizeRun();
        } else {
            body << "\nNo one manages to cover for you in time.";
            settleOrdinaryMorningLate("Help Failed", body.str());
        }
    };

    std::function<bool(int)> startMorningRushIfNeeded;

    auto checkEventTriggers = [&eventRunner, &ctx, &questManager, &questActive, &startMorningRushIfNeeded](int prev) {
        if (startMorningRushIfNeeded && startMorningRushIfNeeded(prev)) return true;
        bool triggered = eventRunner.checkTriggers(ctx, prev);
        if (triggered) {
            questManager.onEventCompleted();
            if (!questActive && questManager.shouldTriggerQuest()) {
                questActive = true;
                questManager.createNextQuest();
            }
        }
        return triggered;
    };

    auto buildSettlementBody = [&](const SettlementResult& result, int page) {
        std::ostringstream body;
        if (page == 0) {
            const auto* primaryEnding = primaryEndingFor(result);
            if (primaryEnding) {
                if (result.achievedEndings.size() > 1) {
                    body << cls::text("settlement.primary_ending") << "\n";
                }
                appendEndingLine(body, *primaryEnding);
                body << "\n\n" << settlementText(primaryEnding->descriptionKey, primaryEnding->description);

                if (result.achievedEndings.size() > 1) {
                    body << "\n\n" << cls::text("settlement.also_achieved") << ":\n";
                    for (std::size_t i = 1; i < result.achievedEndings.size(); ++i) {
                        body << "- ";
                        appendEndingLine(body, result.achievedEndings[i]);
                        body << "\n";
                    }
                }
            }
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

    auto triggerFinalSettlementNow = [&]() {
        if (settlementActive) return true;

        eventRunner.clear();
        questActive = false;
        mealChoicePrompt.clear();
        pendingTimedActivity.clear();
        pendingSleep.clear();
        morningRushActive = false;
        morningRushTestMode = false;
        timeSkipFlash.active = false;
        player.stopMovement();

        normalizeHidden(player.getHidden());
        player.syncDailyCountersFromHidden();
        syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());

        settlementResult = settlementResolver.resolveImmediate(player);
        if (!settlementResult.resolved) {
            settlementResult = settlementResolver.resolveFinal(player);
        }
        settlementActive = true;
        settlementPage = 0;
        activityNotice.show(cls::text("quest.final_result"),
            buildSettlementBody(settlementResult, settlementPage));
        return true;
    };

    startMorningRushIfNeeded = [&](int previousMinute) {
        if (morningRushActive || settlementActive) return false;
        if (mealChoicePrompt.active && mealChoicePrompt.purpose == kMorningRushPromptPurpose) return false;
        if (ctx.currentPlace == CampusPlace::Classroom) return false;
        if (ctx.timeSystem.isClassPrompted() || ctx.timeSystem.isClassResolved()) return false;

        const int now = normalizedMinute(ctx.timeSystem.getMinuteOfDay());
        auto commuteToClassMinutes = [](CampusPlace place) {
            switch (place) {
                case CampusPlace::Dormitory: return 15;
                case CampusPlace::Cafeteria: return 10;
                case CampusPlace::Gym: return 10;
                case CampusPlace::Library: return 5;
                case CampusPlace::Campus: return 5;
                case CampusPlace::Store: return 10;
                case CampusPlace::Classroom: return 0;
            }
            return 5;
        };

        const int commuteMinutes = commuteToClassMinutes(ctx.currentPlace);
        const bool stillBeforeClass = now < TimeSystem::kClassMinute;
        const bool normalWalkWouldBeLate = now + commuteMinutes > TimeSystem::kClassMinute;
        if (now >= TimeSystem::kClassMinute && now < TimeSystem::kClassEndMinute) {
            settleOrdinaryMorningLate("Late Arrival",
                "It is already past 08:50, so the sprint chance is gone.");
            return true;
        }
        if (!(previousMinute < TimeSystem::kClassMinute
              && stillBeforeClass
              && normalWalkWouldBeLate)) {
            return false;
        }

        if (morningRushTriggeredToday || morningRushPromptedToday) {
            settleOrdinaryMorningLate("Late Arrival",
                "You already had a rush chance today, so this late risk is resolved normally.");
            return true;
        }
        if (morningRushCooldownDays > 0) {
            settleOrdinaryMorningLate("Too Tired To Sprint",
                "You are still worn out from the last failed sprint and can only arrive late.");
            return true;
        }

        const int day = ctx.timeSystem.getDay();
        const bool firstLate = hiddenInt("lateCount") == 0;
        const bool beforeMajorExam = (day == 6 || day == 13);
        const bool shortSleepMorning = hiddenInt("lastSleepMinutes", kDefaultSleepMinutes) < kMinimumHealthySleepMinutes;
        const bool lowTrustEmergency = hiddenInt("teacherTrust") <= -10;
        const bool keyMorning = firstLate || beforeMajorExam || shortSleepMorning || lowTrustEmergency;
        if (!keyMorning) {
            settleOrdinaryMorningLate("Late Arrival",
                "This is not a key class moment, so the game uses the light late settlement.");
            return true;
        }

        morningRushPromptedToday = true;
        pendingMorningRushShortSleep = shortSleepMorning;
        pendingMorningRushBreakfastRisk = (ctx.currentPlace == CampusPlace::Cafeteria);
        pendingMorningRushPenalty = 0;
        pendingMorningRushRoute = routeForMorningRush(ctx.currentPlace);
        activityNotice.clear();

        std::ostringstream body;
        body << "Class starts at 08:50. Walking from here takes "
             << commuteMinutes << " minutes.\nChoose a response.";
        mealChoicePrompt.show(
            "Sprint To Class?",
            body.str(),
            std::vector<std::string>{"Run sprint", "Accept late penalty", "Ask classmate"},
            kMorningRushPromptPurpose,
            std::vector<int>{1, 2, 3});
        return true;
    };

    auto finishMorningRush = [&]() {
        morningRushActive = false;
        if (morningRushTestMode) {
            const int testedStage = morningRushGame.getStageIndex();
            morningRushTestMode = false;
            activityNotice.show("Morning Rush Test",
                "Stage " + std::to_string(testedStage)
                + " test finished. No player stats or schedule were changed.");
            return;
        }
        syncMorningRushTeacherTrustToHidden();
        movePlayerToClassroom();
        timeSystem.markClassPrompted();
        if (!morningRushGame.wasLate()) {
            timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
            mergeHidden(player.getHidden(), HiddenMap{{"classAttendCount", 1}, {"teacherTrust", 1}});
            syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
            activityNotice.show("Morning Class",
                "You arrive just in time and settle into the front row.");
        } else {
            addHiddenInt("lateCount", 1);
            morningRushCooldownDays = 2;
            timeSystem.setTimeAbsolute(morningRushGame.wasCriticalFailure()
                ? TimeSystem::kClassMinute + 15
                : TimeSystem::kClassMinute + 5);
            timeSystem.markClassResolved();
            activityNotice.show(
                morningRushGame.wasCriticalFailure() ? "Called Out" : "Late Arrival",
                morningRushGame.getResultText());
        }
        if (maybeFinalizeRun) maybeFinalizeRun();
    };

    ctx.runTimedActivity = [&ctx, &pendingTimedActivity](int minutes, const Attributes& delta,
            const std::string& title, const std::string& body,
            const std::string& activityId, bool customDuration) {
        runTimedActivity(ctx, pendingTimedActivity, minutes, delta, title, body, activityId, customDuration);
    };

    ctx.runTimedActivityWithHidden = [&ctx, &pendingTimedActivity](int minutes, const Attributes& delta,
            const HiddenMap& hiddenDelta,
            const std::string& title, const std::string& body,
            const std::string& activityId, bool customDuration) {
        runTimedActivityWithHidden(ctx, pendingTimedActivity, minutes, delta, hiddenDelta, title, body, activityId, customDuration);
    };

    ctx.showTimedResult = showTimedResult;
    ctx.checkEventTriggers = checkEventTriggers;
    ctx.sleepFromDormitory = [&ctx, &pendingSleep]() {
        sleepFromDormitory(ctx, pendingSleep);
    };
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
                               &maybeFinalizeRun](const MapPortal& portal) {
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
                markNoSleepForSkippedDays(ctx);
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

    auto resolveDormitoryDeskChoice = [&ctx, &eventRunner](int choiceIndex) {
        if (choiceIndex == 0) {
            RegularInteraction::handleDormitoryDeskStudy(ctx, "dormitory_desk");
            return;
        }
        if (choiceIndex == 1) {
            if (eventRunner.triggerByAction("dormitory_games", ctx)) return;
            RegularInteraction::handleDormitoryGames(ctx, "dormitory_games");
        }
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

    auto handleInteraction = [&ctx, &eventRunner, &canTriggerInteraction, &mealChoicePrompt, &questManager, &questActive](const InteractionPoint& ip) {
        if (!canTriggerInteraction(ip)) return;
        if (ip.actionId == "dormitory_desk") {
            mealChoicePrompt.show(
                cls::text("prompt.dormitory_desk.title"),
                cls::text("prompt.dormitory_desk.body"),
                std::vector<std::string>{
                    cls::text("prompt.dormitory_desk.study"),
                    cls::text("prompt.dormitory_desk.games")
                },
                kDormitoryDeskPromptPurpose,
                std::vector<int>{0, 1});
            return;
        }
        if (eventRunner.triggerByAction(ip.actionId, ctx)) {
            questManager.onEventCompleted();
            if (!questActive && questManager.shouldTriggerQuest()) {
                questActive = true;
                questManager.createNextQuest();
            }
            return;
        }
        if (CafeteriaInteraction::handleInteraction(ctx, ip.actionId, ip.displayLabel())) return;
        if (DormitoryInteraction::handle(ctx, ip)) return;
        if (RegularInteraction::handle(ctx, ip)) return;
        ctx.activityNotice.show(ip.displayLabel(), ip.displayDescription());
    };

    // ── 主循环 ───────────────────────────────────────────────
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        if (mealChoicePrompt.debounce > 0) --mealChoicePrompt.debounce;
        if (timeSystem.getDay() != gamePlayDay) {
            gamePlayDay = timeSystem.getDay();
            gamesPlayedToday = 0;
            morningRushTriggeredToday = false;
            morningRushPromptedToday = false;
            if (morningRushCooldownDays > 0) --morningRushCooldownDays;
        }

        // 更新战斗结果计时器
        combatResult.update(dt);
        sceneBackground.update(dt);
        sceneTransition.update(dt);
        timeSkipFlash.update(dt);
        audioManager.update(timeSystem, combatResult, gameSettings, currentPlace);

        // ── 事件处理 ────────────────────────────────────────
        while (const auto eventOpt = window.pollEvent()) {
            const auto& event = *eventOpt;

            // 转发给 TGUI，widget 回调在此触发
            tguiCtx.handleEvent(event);

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
            if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                if (screen == GameScreen::GAME && isSettingsShortcut(*keyEv)) {
                    openSettingsPanel(GameScreen::GAME);
                    continue;
                }
                if (screen == GameScreen::GAME && isFinishShortcut(*keyEv)) {
                    triggerFinalSettlementNow();
                    continue;
                }
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
                        openSettingsPanel(GameScreen::TITLE);
                    } else if (keyEv->code == sf::Keyboard::Key::H) {
                        handleTitleAction(TitleAction::Help);
                    } else if (keyEv->code == sf::Keyboard::Key::Enter
                        || keyEv->code == sf::Keyboard::Key::Space) {
                        handleTitleAction(titleScreen.confirmSelection());
                    }
                }
                // 鼠标点击由 TGUI 回调处理
                continue;
            }

            if (screen == GameScreen::SETTINGS) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Escape
                        || isSettingsShortcut(*keyEv)) {
                        saveRuntimeSettings();
                        closeSettingsPanel();
                    } else if (keyEv->code == sf::Keyboard::Key::Up
                        || keyEv->code == sf::Keyboard::Key::W) {
                        settingsPanel.moveSelection(-1);
                    } else if (keyEv->code == sf::Keyboard::Key::Down
                        || keyEv->code == sf::Keyboard::Key::S) {
                        settingsPanel.moveSelection(1);
                    } else if (keyEv->code == sf::Keyboard::Key::Left
                        || keyEv->code == sf::Keyboard::Key::A) {
                        handleSettingsAction(settingsPanel.adjustCurrent(-1));
                    } else if (keyEv->code == sf::Keyboard::Key::Right
                        || keyEv->code == sf::Keyboard::Key::D) {
                        handleSettingsAction(settingsPanel.adjustCurrent(1));
                    } else if (keyEv->code == sf::Keyboard::Key::Enter
                        || keyEv->code == sf::Keyboard::Key::Space) {
                        handleSettingsAction(settingsPanel.confirmCurrent());
                    }
                }
                // 鼠标和滑块交互由 TGUI 回调处理
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
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Escape) {
                        difficultyPanel.setVisible(false);
                        screen = GameScreen::TITLE;
                    } else if (keyEv->code == sf::Keyboard::Key::Num1) {
                        startGameWithDifficulty(Difficulty::Easy);
                    } else if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Enter) {
                        startGameWithDifficulty(Difficulty::Normal);
                    } else if (keyEv->code == sf::Keyboard::Key::Num3) {
                        startGameWithDifficulty(Difficulty::Hard);
                    }
                }
                // 鼠标点击由 TGUI 回调处理
                continue;
            }

            if (timeSkipFlash.active) {
                continue;
            }

            if (morningRushActive) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (morningRushTestMode && keyEv->code == sf::Keyboard::Key::Escape) {
                        morningRushActive = false;
                        morningRushTestMode = false;
                        activityNotice.show("Morning Rush Test",
                            "Practice exited. No player stats or schedule were changed.");
                        continue;
                    }
                }
                syncMorningRushTeacherTrustFromHidden();
                morningRushGame.handleInput(event, player, morningRushTeacherTrust);
                syncMorningRushTeacherTrustToHidden();
                continue;
            }

            if (eventRunner.isActive()) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>())
                    eventRunner.handleInput(keyEv->code, ctx);
                continue;
            }

            // ── 主线任务输入处理 ────────────────────────────
            if (questActive) {
                MainQuest* quest = questManager.getCurrentQuest();
                if (quest && !quest->isCompleted()) {
                    int choiceMade = -1;
                    if (event.is<sf::Event::KeyPressed>()) {
                        if (quest->handleInput(event, player, choiceMade)) {
                            if (quest->isCompleted()) {
                                quest->execute(player);
                                questManager.onQuestCompleted();
                                questActive = false;
                                if (maybeFinalizeRun()) continue;
                            }
                        }
                    }
                } else {
                    questActive = false;
                }
                continue;
            }

            if (screen == GameScreen::GAME && !debugSandbox.isExpanded()
                && !mealChoicePrompt.active && !activityNotice.active) {
                if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEv->button == sf::Mouse::Button::Left) {
                        const sf::Vector2f target = window.mapPixelToCoords(mouseEv->position, gameView);
                        // TGUI 处理 timePanel 点击
                        player.setMoveTarget(target);
                    }
                    continue;
                }
            }

            if (mealChoicePrompt.active) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (mealChoicePrompt.debounce > 0) continue;
                    if (mealChoicePrompt.purpose == kMorningRushPromptPurpose) {
                        auto choiceIndex = [&]() {
                            if (keyEv->code == sf::Keyboard::Key::Num1 || keyEv->code == sf::Keyboard::Key::Numpad1) return 0;
                            if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Numpad2) return 1;
                            if (keyEv->code == sf::Keyboard::Key::Num3 || keyEv->code == sf::Keyboard::Key::Numpad3) return 2;
                            if (keyEv->code == sf::Keyboard::Key::Enter) return 0;
                            return -1;
                        }();
                        if (choiceIndex >= 0
                            && choiceIndex < static_cast<int>(mealChoicePrompt.values.size())) {
                            resolveMorningRushChoice(mealChoicePrompt.values[choiceIndex]);
                        } else if (keyEv->code == sf::Keyboard::Key::Escape) {
                            settleOrdinaryMorningLate("Late Arrival",
                                "You miss the sprint chance and arrive late.");
                        }
                        continue;
                    }
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
                                executeTimedActivity(ctx,
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
                            executeSleep(ctx, settlementActive, selectedMinutes, explicitAlarm);
                        } else if (keyEv->code == sf::Keyboard::Key::Escape) {
                            pendingSleep.clear();
                            mealChoicePrompt.clear();
                        }
                        continue;
                    }

                    if (mealChoicePrompt.purpose == kDormitoryDeskPromptPurpose) {
                        auto choiceIndex = [&]() {
                            if (keyEv->code == sf::Keyboard::Key::Num1 || keyEv->code == sf::Keyboard::Key::Numpad1) return 0;
                            if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Numpad2) return 1;
                            return -1;
                        }();
                        if (choiceIndex >= 0) {
                            mealChoicePrompt.clear();
                            resolveDormitoryDeskChoice(choiceIndex);
                        } else if (keyEv->code == sf::Keyboard::Key::Escape) {
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
                        settlementActive = false;
                        settlementPage = 0;
                        screen = GameScreen::TITLE;
                        activityNotice.clear();
                        hideModalUi();
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
            titleScreen.setVisible(true);
            titleScreen.update(dt);
            window.clear(sf::Color(20, 20, 30));
            titleScreen.render(window);
            tguiCtx.draw();
            window.display();
            continue;
        }

        if (screen == GameScreen::SETTINGS) {
            settingsPanel.setVisible(true);
            window.clear(sf::Color(20, 20, 30));
            tguiCtx.draw();
            window.display();
            continue;
        }

        if (screen == GameScreen::HELP) {
            helpPanel.setVisible(true);
            window.clear(sf::Color(20, 20, 30));
            tguiCtx.draw();
            window.display();
            continue;
        }

        if (screen == GameScreen::DIFFICULTY) {
            difficultyPanel.setVisible(true);
            window.clear(sf::Color(20, 20, 30));
            difficultyPanel.render(window);
            tguiCtx.draw();
            window.display();
            continue;
        }

        if (sceneTransition.active) {
            window.clear(sf::Color(20, 20, 30));
            renderSceneTransition(window, font, sceneBackground, sceneTransition);
            tguiCtx.draw();
            window.display();
            continue;
        }

        if (timeSkipFlash.active) {
            window.clear(sf::Color(0, 0, 0));
            renderTimeSkipFlash(window, font, timeSkipFlash);
            tguiCtx.draw();
            window.display();
            continue;
        }

        if (morningRushActive) {
            hideModalUi();
            syncMorningRushTeacherTrustFromHidden();
            morningRushGame.update(dt, player, morningRushTeacherTrust);
            syncMorningRushTeacherTrustToHidden();
            if (morningRushGame.consumeCompleted()) {
                finishMorningRush();
            } else {
                morningRushRenderer.update(dt);
                window.clear(sf::Color(20, 20, 30));
                morningRushRenderer.render(window);
                tguiCtx.draw();
                window.display();
                continue;
            }
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

            if (!eventRunner.isActive() && justPressed(sf::Keyboard::Key::Enter) && !activityNotice.active) {
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
            renderStatsPanel(window, hud, player);
            timePanel.setTimeSystem(&timeSystem);
            timePanel.update(0.0f);
        }
        if (eventRunner.isActive()) {
            eventRunner.render(window, modalBox);
        }
        if (questActive) {
            MainQuest* quest = questManager.getCurrentQuest();
            if (quest && !quest->isCompleted()) {
                renderQuestContent(window, modalBox, quest);
            }
        }
        if (activityNotice.active) {
            if (settlementActive) {
                const std::string title = settlementPage == 0
                    ? cls::text("quest.final_result")
                    : (settlementPage == 1 ? cls::text("quest.earned_titles") : cls::text("quest.semester_summary"));
                modalBox.setContent(title, buildSettlementBody(settlementResult, settlementPage),
                                    cls::text("quest.return_title"));
                const bool isBadEnding = settlementResult.gameOver;
                modalBox.setFullscreenTexture(isBadEnding ? &endingBadTexture : &endingGoodTexture);
            } else {
                modalBox.setContent(activityNotice.title, activityNotice.body,
                                    cls::text("ui.press_enter_continue"));
                modalBox.setFullscreenTexture(nullptr);
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
        if (!eventRunner.isActive() && !questActive
            && !activityNotice.active && !mealChoicePrompt.active) {
            modalBox.setVisible(false);
        }
        if (fontOk) {
            debugSandbox.refresh(ctx, eventRunner);
            debugSandbox.render(window);
        }

        tguiCtx.draw();
        window.display();
    }

    return 0;
}

