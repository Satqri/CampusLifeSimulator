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

#include <cmath>
#include <filesystem>
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
        hidden["lateNightLevel"] = 0;
        hidden["exerciseCount"] = 0;
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
        {}, {}, {}, {}  // callbacks 稍后注入
    };

    // ── 核心回调（handler 通过 ctx 调用）───────────────────────
    auto showTimedResult = [&ctx](const std::string& title, const std::string& body) {
        std::ostringstream message;
        message << body << "\n" << cls::text("time.current") << ": " << ctx.timeSystem.clockText();
        ctx.activityNotice.show(title, message.str());
    };

    auto checkEventTriggers = [&eventRunner, &ctx](int prev) {
        eventRunner.checkTriggers(ctx, prev);
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

    auto sleepFromDormitory = [&ctx, &showTimedResult, &maybeFinalizeRun]() {
        if (!ctx.timeSystem.canSleep()) {
            ctx.activityNotice.show("Too Early",
                "You can choose sleep after 22:30.");
            return;
        }
        const int sleptMinutes = ctx.timeSystem.sleepToNextDay();
        const int sleptHours = sleptMinutes / 60;
        const int sanGain = std::min(45, sleptHours * 5);
        const int energyGain = std::min(70, sleptHours * 8);
        ctx.player.modifyAttributes(Attributes{.energy = energyGain, .san = sanGain});
        ctx.player.dailyAttributeCheck();
        ctx.player.setPosition(480.0f, 276.0f);
        ctx.player.stopMovement();
        ctx.currentPlace = CampusPlace::Dormitory;
        ctx.currentMap = ctx.dormitoryMap;
        ctx.gamePlayDay = ctx.timeSystem.getDay();
        ctx.gamesPlayedToday = 0;
        std::ostringstream body;
        if (ctx.timeSystem.isFinished()) {
            body << "The 14-day project period is complete. SAN +"
                 << sanGain << ", Energy +" << energyGain << ".";
        } else {
            body << "You slept " << sleptHours << " hours. SAN +"
                 << sanGain << ", Energy +" << energyGain << ".";
        }
        ctx.timeSkipFlash.start("Sleeping...");
        showTimedResult(ctx.timeSystem.isFinished()
            ? "Fourteen Days Complete" : "New Day", body.str());
        maybeFinalizeRun();
    };

    auto runTimedActivity = [&ctx, &checkEventTriggers, &showTimedResult, &maybeFinalizeRun](
            int minutes, const Attributes& delta,
            const std::string& title, const std::string& body) {
        const int prev = ctx.timeSystem.advanceMinutes(minutes);
        ctx.player.modifyAttributes(delta);
        ctx.timeSkipFlash.start("Time passes...");
        showTimedResult(title, body);
        checkEventTriggers(prev);
        maybeFinalizeRun();
    };

    ctx.runTimedActivity = runTimedActivity;
    ctx.showTimedResult = showTimedResult;
    ctx.checkEventTriggers = checkEventTriggers;
    ctx.sleepFromDormitory = sleepFromDormitory;

    // ── 薄封装 lambda ──────────────────────────────────────────
    auto trySpawnEnemy = [&ctx]() {
        CombatSystem::trySpawnEnemy(ctx);
    };

    auto fightNearestEnemy = [&ctx]() -> bool {
        return CombatSystem::fightNearestEnemy(ctx);
    };

    auto startMapTransition = [&ctx, &sceneTransition](const MapPortal& portal) {
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

    auto handleInteraction = [&ctx, &eventRunner](const InteractionPoint& ip) {
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
                    if (!difficultyApplied) {
                        applyDifficulty(player, selectedDifficulty);
                        difficultyApplied = true;
                    }
                    currentPlace = CampusPlace::Campus;
                    currentMap = campusMap.get();
                    player.setPosition(480.0f, 276.0f);
                    timeSystem = TimeSystem();
                    initializeHiddenState();
                    activityNotice.clear();
                    mealChoicePrompt.clear();
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
                        player.setMoveTarget(target);
                    }
                    continue;
                }
            }

            if (mealChoicePrompt.active) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Num1 || keyEv->code == sf::Keyboard::Key::Numpad1) {
                        resolveMealChoice(0);
                    } else if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Numpad2) {
                        resolveMealChoice(1);
                    } else if (keyEv->code == sf::Keyboard::Key::Num3 || keyEv->code == sf::Keyboard::Key::Numpad3) {
                        resolveMealChoice(2);
                    } else if (keyEv->code == sf::Keyboard::Key::Escape) {
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

            // 按键 C = 压力事件（降低 SAN，触发敌人出现）
            if (justPressed(sf::Keyboard::Key::C)) {
                player.modifyAttributes(Attributes{.san = -15});
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
                }
            }

            // 按键 V = 恢复 SAN（模拟休息/自我关怀）
            if (justPressed(sf::Keyboard::Key::V)) {
                player.modifyAttributes(Attributes{.san = 15});
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
            body << mealChoicePrompt.body << "\n\n"
                 << "[1] " << mealChoicePrompt.first
                 << "\n[2] " << mealChoicePrompt.second;
            if (!mealChoicePrompt.third.empty())
                body << "\n[3] " << mealChoicePrompt.third;
            modalBox.setContent(mealChoicePrompt.title, body.str(),
                                mealChoicePrompt.third.empty() ? cls::text("prompt.choice12") : cls::text("prompt.choice123"));
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

