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
#include "core/LibraryConfig.h"
#include "core/MealConfig.h"
#include "core/SceneConfig.h"
#include "core/SceneTransition.h"
#include "core/TimeSkipFlash.h"
#include "core/TimeSystem.h"
#include "core/Types.h"
#include "entity/CombatHelper.h"
#include "ui/ActivityNotice.h"
#include "ui/ChoicePrompt.h"
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
#include "ui/HUD.h"
#include "ui/TitleScreen.h"
#include "ui/DifficultyPanel.h"
#include "ui/SceneBackground.h"

#include <cmath>
#include <sstream>
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <memory>

enum class GameScreen {
    TITLE,
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

    sf::Text text(font, flash.text, 20);
    text.setFillColor(sf::Color(230, 230, 220));
    text.setPosition({360.0f, 252.0f});
    window.draw(text);
}

void applyDifficulty(Player& player, Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:
            player.modifyAttributes(Attributes(20, 15, 5, 5, 0));
            break;
        case Difficulty::Normal:
            break;
        case Difficulty::Hard:
            player.modifyAttributes(Attributes(-20, -10, 0, -5, 0));
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

    sf::Text title(font, transition.title, 34);
    title.setFillColor(sf::Color(250, 240, 205));
    title.setOutlineColor(sf::Color(18, 42, 45));
    title.setOutlineThickness(2.0f);
    title.setPosition({190.0f, 352.0f});
    window.draw(title);

    sf::Text subtitle(font, transition.subtitle, 18);
    subtitle.setFillColor(sf::Color(224, 238, 220));
    subtitle.setPosition({190.0f, 407.0f});
    window.draw(subtitle);

    const bool ready = transition.canContinue();
    sf::Text hint(font, ready ? "Press Enter to enter" : "Entering...", 13);
    hint.setFillColor(ready ? sf::Color(210, 210, 190, 180) : sf::Color(210, 210, 190, 110));
    hint.setPosition({190.0f, 444.0f});
    window.draw(hint);
}

// scene_transition 配置已移至 assets/config/scene_transitions.json

// ──────────────────────────────────────────────────────────────
// main
// ──────────────────────────────────────────────────────────────
int main() {
    // ── 窗口 ────────────────────────────────────────────────
    sf::RenderWindow window(sf::VideoMode({kWindowWidth, kWindowHeight}), "CampusLifeSimulator - Class Demo");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    // 将 960×540 渲染坐标系映射到 1280×720 窗口
    sf::View gameView(sf::FloatRect({0.0f, 0.0f}, {kRenderWidth, kRenderHeight}));
    window.setView(gameView);

    // ── 字体（编译期检测平台，选择对应系统字体）─────────────
    sf::Font font;
    bool fontOk = false;
#if defined(_WIN32)
    const std::vector<std::string> fontCandidates = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/arial.ttf",
    };
    for (const auto& path : fontCandidates) {
        if (font.openFromFile(path)) { fontOk = true; break; }
    }
#elif defined(__APPLE__)
    fontOk = font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")
          || font.openFromFile("/System/Library/Fonts/PingFang.ttc");
#elif defined(__linux__)
    fontOk = font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
#endif
    if (!fontOk) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
    }

    // ── 创建 Entity 对象 ─────────────────────────────────────
    TitleScreen titleScreen(font, "assets/ui/campus_title_bg.png");
    DifficultyPanel difficultyPanel(font);
    SceneBackground sceneBackground;
    SceneTransition sceneTransition;
    TimeSystem timeSystem;
    ActivityNotice activityNotice;
    ChoicePrompt classChoicePrompt;
    ChoicePrompt mealChoicePrompt;
    TimeSkipFlash timeSkipFlash;
    TimePanel timePanel(font);
    ModalBox modalBox(font);
    GameScreen screen = GameScreen::TITLE;
    Difficulty selectedDifficulty = Difficulty::Normal;
    bool difficultyApplied = false;
    int selectedLibraryBook = 0;
    std::array<int, 4> libraryBookProgress = {0, 0, 0, 0};
    auto libraryBooks = loadLibraryConfig(cls::resolveAssetPath("assets/config/library.json"));
    auto mealOptions = loadMealConfig(cls::resolveAssetPath("assets/config/meals.json"));
    int heldMealIndex = -1;
    int lastMealPickupSlot = -1;
    int gamePlayDay = 1;
    int gamesPlayedToday = 0;

    // ── 地图对象 ───────────────────────────────────────────────
    auto campusMap     = std::make_unique<CampusMap>();
    auto dormitoryMap  = std::make_unique<DormitoryInterior>();
    auto gymMap        = std::make_unique<GymInterior>();
    auto libraryMap    = std::make_unique<LibraryInterior>();
    auto classroomMap  = std::make_unique<ClassroomInterior>();
    auto cafeteriaMap  = std::make_unique<CafeteriaInterior>();

    // 设置字体
    campusMap->setFont(&font);
    campusMap->setTimeSystem(&timeSystem);
    dormitoryMap->setFont(&font);
    gymMap->setFont(&font);
    libraryMap->setFont(&font);
    classroomMap->setFont(&font);
    cafeteriaMap->setFont(&font);

    BuildingInterior* currentMap = campusMap.get();

    auto setCurrentMap = [&](CampusPlace place) -> BuildingInterior* {
        switch (place) {
            case CampusPlace::Campus:    return campusMap.get();
            case CampusPlace::Dormitory: return dormitoryMap.get();
            case CampusPlace::Gym:       return gymMap.get();
            case CampusPlace::Library:   return libraryMap.get();
            case CampusPlace::Classroom: return classroomMap.get();
            case CampusPlace::Cafeteria: return cafeteriaMap.get();
        }
        return campusMap.get();
    };

    Player player(480.0f, 280.0f);
    player.setName("Protagonist");
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

    // ── Entity Demo 状态 ─────────────────────────────────────
    int spawnCounter = 0;   // 每按 C 累计，用于决定生成哪个情绪类型

    // Lambda: 尝试根据 SAN 等级生成敌人
    // 规则: SAN<30 每按一次 C 概率生成（等级越高概率越大），最多 3 个活跃
    auto trySpawnEnemy = [&]() {
        int lvl = player.getSanLevel();
        if (lvl == 0) return;  // SAN >= 30, 无敌人

        // 最大活跃敌人数随 SAN 等级递增
        int maxEnemies = (lvl == 1) ? 1 : (lvl == 2) ? 2 : 3;
        if (static_cast<int>(activeEnemies.size()) >= maxEnemies) return;

        // 概率: lvl=1: 40%, lvl=2: 60%, lvl=3: 90%
        int chance = lvl == 1 ? 40 : (lvl == 2 ? 60 : 90);
        if ((std::rand() % 100) >= chance) return;

        // 随机情绪类型
        EmotionType types[] = {
            EmotionType::ANXIETY, EmotionType::DEPRESSION, EmotionType::ANGER,
            EmotionType::FEAR, EmotionType::LONELINESS
        };
        EmotionType type = types[spawnCounter % 5];
        spawnCounter++;

        // 在玩家附近随机位置生成
        float ox = player.getPosition().x + ((std::rand() % 160) - 80);
        float oy = player.getPosition().y + ((std::rand() % 160) - 80);
        ox = std::clamp(ox, 40.0f, 920.0f);
        oy = std::clamp(oy, 80.0f, 500.0f);

        auto enemy = std::make_unique<Enemy>(ox, oy, type, 12, 5);
        enemy->scaleWithSanLevel(lvl);
        activeEnemies.push_back(std::move(enemy));

        std::cout << "[Combat] " << activeEnemies.back()->getName()
                  << " appeared! SAN=" << player.getAttributes().san
                  << " Level=" << lvl
                  << " DC=" << activeEnemies.back()->getDC()
                  << " ATK=" << activeEnemies.back()->getAttackPower() << std::endl;
    };

    // Lambda: 与附近敌人战斗
    auto fightNearestEnemy = [&]() -> bool {
        if (activeEnemies.empty() || combatResult.active) return false;

        // 找最近的敌人（距离 < 100px 内）
        float minDist = 100.0f;
        int nearestIdx = -1;
        sf::Vector2f pp = player.getPosition();
        for (int i = 0; i < static_cast<int>(activeEnemies.size()); ++i) {
            sf::Vector2f ep = activeEnemies[i]->getPosition();
            float dx = pp.x - ep.x;
            float dy = pp.y - ep.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < minDist) {
                minDist = dist;
                nearestIdx = i;
            }
        }
        if (nearestIdx < 0) return false;

        Enemy& enemy = *activeEnemies[nearestIdx];
        EmotionType etype = enemy.getEmotionType();

        // 计算检定
        int statVal = statForEmotion(player, etype);
        int modifier = (statVal - 50) / 10;
        int buffMod = player.getCombatBuffs().nextRollModifier;
        int d20 = (std::rand() % 20) + 1;
        int total = d20 + modifier + buffMod;
        int dc = enemy.getDC();
        bool win = total >= dc;

        std::cout << "[Combat] " << actionNameForEmotion(etype)
                  << " vs " << enemy.getName()
                  << " | D20:" << d20 << " MOD:" << modifier
                  << " Buff:" << buffMod << " = " << total
                  << " vs DC:" << dc
                  << " -> " << (win ? "WIN" : "LOSE") << std::endl;

        if (win) {
            player.modifyAttributes(Attributes(20, 0, 0, 0, 0));
            player.getCombatBuffs().nextEventPositive = true;
            player.getCombatBuffs().nextRollModifier = 2;
        } else {
            player.modifyAttributes(Attributes(-15, 0, 0, 0, 0));
            player.getCombatBuffs().nextEventPositive = false;
            player.getCombatBuffs().nextRollModifier = -2;
        }

        combatResult.show(win, enemy.getName(), d20, modifier + buffMod, total, dc);

        // 战斗后敌人消失
        activeEnemies.erase(activeEnemies.begin() + nearestIdx);
        return true;
    };

    auto startMapTransition = [&](const MapPortal& portal) {
        pendingPlace = portal.target;
        pendingSpawnPosition = portal.spawnPosition;
        hasPendingMapTransition = true;
        sceneTransition.start(portal.transitionBackground, portal.title, portal.subtitle);
    };

    auto finishSceneTransition = [&]() {
        if (hasPendingMapTransition) {
            currentPlace = pendingPlace;
            currentMap = setCurrentMap(pendingPlace);
            player.setPosition(pendingSpawnPosition.x, pendingSpawnPosition.y);
            player.stopMovement();
            hasPendingMapTransition = false;
        }
        sceneTransition.skip();
    };

    auto forceMorningClass = [&]() {
        timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
        timeSystem.markClassPrompted();
        currentPlace = CampusPlace::Classroom;
        currentMap = classroomMap.get();

        std::vector<const InteractionPoint*> desks;
        for (const auto& ip : classroomMap->getInteractionPoints()) {
            if (ip.label == "Sit at Desk") desks.push_back(&ip);
        }
        if (!desks.empty()) {
            const InteractionPoint* desk = desks[std::rand() % desks.size()];
            player.setPosition(desk->area.position.x + desk->area.size.x * 0.5f,
                               desk->area.position.y + desk->area.size.y * 0.5f);
        } else {
            player.setPosition(480.0f, 276.0f);
        }
        player.stopMovement();

        classChoicePrompt.show(
            timeSystem.isMidtermDay() ? "Midterm Morning" : "Morning Class",
            timeSystem.isMidtermDay()
                ? "It is Day 7. The midterm starts from this classroom seat."
                : "The bell rings at 08:50. Choose how to handle this class.",
            timeSystem.isMidtermDay() ? "Take the midterm seriously" : "Attend class carefully",
            "Skip class"
        );
    };

    auto checkClassSchedule = [&](int previousMinute) {
        if (timeSystem.crossedClassTime(previousMinute) || timeSystem.shouldForceClass()) {
            activityNotice.clear();
            forceMorningClass();
        }
    };

    auto showTimedResult = [&](const std::string& title, const std::string& body) {
        std::ostringstream message;
        message << body << "\nCurrent time: " << timeSystem.clockText();
        activityNotice.show(title, message.str());
    };

    auto runTimedActivity = [&](int minutes, const Attributes& delta,
                                const std::string& title, const std::string& body) {
        const int previousMinute = timeSystem.advanceMinutes(minutes);
        player.modifyAttributes(delta);
        timeSkipFlash.start("Time passes...");
        showTimedResult(title, body);
        checkClassSchedule(previousMinute);
    };

    auto resolveClassChoice = [&](bool attend) {
        classChoicePrompt.clear();
        timeSystem.markClassResolved();

        if (attend) {
            timeSystem.setTimeAbsolute(TimeSystem::kClassEndMinute);
            std::ostringstream body;
            if (timeSystem.isMidtermDay()) {
                const int roll = (std::rand() % 20) + 1;
                const int academicBonus = (player.getAttributes().academic - 50) / 10;
                const int total = roll + academicBonus;
                const bool passed = total >= 12;
                player.modifyAttributes(passed
                    ? Attributes(-10, -16, 12, 0, 0)
                    : Attributes(-16, -18, 4, 0, 0));
                body << "Midterm finished. Roll " << roll
                     << " + Academic Bonus " << academicBonus
                     << " = " << total << (passed ? " (pass)." : " (struggle).");
            } else {
                player.modifyAttributes(Attributes(-8, -12, 8, 0, 0));
                body << "You focused through the morning lecture. Academic +8, SAN -8, Energy -12.";
            }
            timeSkipFlash.start("Class time passes...");
            showTimedResult(timeSystem.isMidtermDay() ? "Midterm Complete" : "Class Complete", body.str());
        } else {
            timeSystem.setTimeAbsolute(TimeSystem::kRollCallMinute);
            const bool called = (std::rand() % 100) < (timeSystem.isMidtermDay() ? 80 : 45);
            std::ostringstream body;
            if (called) {
                player.modifyAttributes(timeSystem.isMidtermDay()
                    ? Attributes(-18, -4, -18, -12, 0)
                    : Attributes(-10, -2, -10, -8, 0));
                body << "At 10:20 the teacher calls attendance. You are absent and take a penalty.";
            } else {
                player.modifyAttributes(Attributes(3, -2, -2, 0, 0));
                body << "At 10:20 there is no roll call. You avoid the immediate penalty, but lose study momentum.";
            }
            timeSkipFlash.start("Skipping class...");
            showTimedResult("Roll Call Notice", body.str());
        }
    };

    auto resolveMealChoice = [&](int mealIndex) {
        mealChoicePrompt.clear();
        if (mealIndex < 0 || mealIndex >= static_cast<int>(mealOptions.size())) return;
        if (!timeSystem.isMealTime()) {
            activityNotice.show("Meal Time Closed",
                "Food is available from 12:00-14:00 and 17:00-19:00.");
            return;
        }
        if (heldMealIndex >= 0) {
            activityNotice.show("Already Holding Food",
                "You already have a tray. Sit at a table and eat it before taking another meal.");
            return;
        }

        const int slot = timeSystem.mealSlotId();
        if (slot >= 0 && lastMealPickupSlot == slot) {
            activityNotice.show("Already Served",
                "You can only take food once during the current meal period.");
            return;
        }

        const MealOption& meal = mealOptions[mealIndex];
        if (player.getAttributes().gold < meal.cost) {
            activityNotice.show("Not Enough Gold",
                "You do not have enough Gold for " + meal.name + ".");
            return;
        }

        player.modifyAttributes(Attributes(0, 0, 0, 0, -meal.cost));
        heldMealIndex = mealIndex;
        lastMealPickupSlot = slot;
        activityNotice.show("Food Taken",
            meal.name + " is on your tray. Find a cafeteria table and press Enter to eat.");
    };

    auto sleepFromDormitory = [&]() {
        if (!timeSystem.canSleep()) {
            activityNotice.show("Too Early",
                "You can choose sleep after 22:30. Until then, the bed is only a short rest spot.");
            return;
        }

        const int sleptMinutes = timeSystem.sleepToNextDay();
        const int sleptHours = sleptMinutes / 60;
        const int sanGain = std::min(45, sleptHours * 5);
        const int energyGain = std::min(70, sleptHours * 8);
        player.modifyAttributes(Attributes(sanGain, energyGain, 0, 0, 0));
        player.setPosition(480.0f, 276.0f);
        player.stopMovement();
        currentPlace = CampusPlace::Dormitory;
        currentMap = dormitoryMap.get();
        heldMealIndex = -1;
        gamePlayDay = timeSystem.getDay();
        gamesPlayedToday = 0;

        std::ostringstream body;
        if (timeSystem.isFinished()) {
            body << "The 14-day project period is complete. Sleep recovered SAN +"
                 << sanGain << " and Energy +" << energyGain << ".";
        } else {
            body << "You slept " << sleptHours << " hours and woke at 08:00. SAN +"
                 << sanGain << ", Energy +" << energyGain << ".";
        }
        timeSkipFlash.start("Sleeping...");
        showTimedResult(timeSystem.isFinished() ? "Fourteen Days Complete" : "New Day", body.str());
    };

    auto handleInteraction = [&](const InteractionPoint& ip) {
        const std::string& id = ip.actionId;

        if (id.rfind("library_shelf_", 0) == 0) {
            selectedLibraryBook = std::clamp(id.back() - '0', 0, 3);
            std::ostringstream body;
            body << ip.label << " selected " << libraryBooks[selectedLibraryBook].name
                 << ". Reading progress: " << libraryBookProgress[selectedLibraryBook]
                 << "%. Browse Shelf does not consume time.";
            activityNotice.show("Shelf Browsed", body.str());
            return;
        }

        if (id == "library_table") {
            const int book = selectedLibraryBook;
            libraryBookProgress[book] = std::min(100, libraryBookProgress[book] + 25);
            const Attributes& delta = libraryBooks[book].delta;

            std::ostringstream body;
            body << "Read " << libraryBooks[book].name << " for 30 minutes. "
                 << libraryBooks[book].skill << " progress is now "
                 << libraryBookProgress[book] << "%.";
            runTimedActivity(30, delta, "Reading Complete", body.str());
            return;
        }

        if (id == "cafeteria_counter") {
            if (!timeSystem.isMealTime()) {
                activityNotice.show("Meal Time Closed",
                    "Food is available from 12:00-14:00 and 17:00-19:00.");
                return;
            }
            if (heldMealIndex >= 0) {
                activityNotice.show("Already Holding Food",
                    "You already have a tray. Sit at a table and eat it before taking another meal.");
                return;
            }
            const int slot = timeSystem.mealSlotId();
            if (slot >= 0 && lastMealPickupSlot == slot) {
                activityNotice.show("Already Served",
                    "You can only take food once during the current meal period.");
                return;
            }
            mealChoicePrompt.show("Choose Meal",
                "Take food from the counter, then sit at a table to eat.",
                mealOptions[0].description,
                mealOptions[1].description,
                mealOptions[2].description);
            return;
        }

        if (id.rfind("cafeteria_table_", 0) == 0) {
            if (heldMealIndex < 0) {
                activityNotice.show("No Food",
                    "You need to take Meal A, B, or C from the counter before eating at a table.");
                return;
            }
            const MealOption& meal = mealOptions[heldMealIndex];
            heldMealIndex = -1;
            runTimedActivity(20, meal.reward, "Meal Complete",
                             meal.name + " eaten. " + meal.description + ".");
            return;
        }

        if (id.rfind("gym_treadmill_", 0) == 0) {
            runTimedActivity(40, Attributes(-4, -14, 0, 2, 0),
                             "Training Complete",
                             "Treadmill run: SAN -4, Energy -14, Social +2.");
            return;
        }

        if (id.rfind("gym_barbell_", 0) == 0) {
            runTimedActivity(40, Attributes(-5, -16, 0, 1, 0),
                             "Training Complete",
                             "Barbell training: SAN -5, Energy -16, Social +1.");
            return;
        }

        if (id == "dormitory_bed") {
            sleepFromDormitory();
            return;
        }

        if (id == "dormitory_desk") {
            runTimedActivity(45, Attributes(-6, -10, 7, 0, 0),
                             "Study Complete",
                             "Desk study: Academic +7, SAN -6, Energy -10.");
            return;
        }

        if (id == "dormitory_games") {
            if (gamePlayDay != timeSystem.getDay()) {
                gamePlayDay = timeSystem.getDay();
                gamesPlayedToday = 0;
            }
            ++gamesPlayedToday;
            if (gamesPlayedToday <= 2) {
                runTimedActivity(60, Attributes(12, 8, 0, 0, 0),
                                 "Game Break Complete",
                                 "Healthy game break: SAN +12, Energy +8. Daily plays: "
                                     + std::to_string(gamesPlayedToday) + "/2.");
            } else {
                runTimedActivity(60, Attributes(4, -12, -2, 0, 0),
                                 "Overplayed",
                                 "Too much gaming today: SAN +4, Energy -12, Academic -2. Daily plays: "
                                     + std::to_string(gamesPlayedToday) + ".");
            }
            return;
        }

        if (id == "dormitory_rug") {
            activityNotice.show("Quiet Moment",
                "You sit down and collect your thoughts. This currently does not consume time.");
            return;
        }

        if (id == "classroom_board") {
            activityNotice.show("Board Reviewed",
                "The board shows today's notes. Review here is informational and does not consume time.");
            return;
        }

        if (id.rfind("classroom_desk_", 0) == 0) {
            activityNotice.show("Desk",
                "Morning class is handled by the 08:50 forced class event.");
            return;
        }

        activityNotice.show(ip.label, ip.description);
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
            if (screen == GameScreen::TITLE) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Enter) {
                        screen = GameScreen::DIFFICULTY;
                    }
                } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
                    const auto action = titleScreen.handleClick({
                        static_cast<float>(mouseEv->position.x),
                        static_cast<float>(mouseEv->position.y)
                    });
                    if (action == TitleAction::Start) {
                        screen = GameScreen::DIFFICULTY;
                    }
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
                    activityNotice.clear();
                    classChoicePrompt.clear();
                    mealChoicePrompt.clear();
                    heldMealIndex = -1;
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
                    const auto action = difficultyPanel.handleClick({
                        static_cast<float>(mouseEv->position.x),
                        static_cast<float>(mouseEv->position.y)
                    });
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

            if (classChoicePrompt.active) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Num1 || keyEv->code == sf::Keyboard::Key::Numpad1
                        || keyEv->code == sf::Keyboard::Key::Enter) {
                        resolveClassChoice(true);
                    } else if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Numpad2
                               || keyEv->code == sf::Keyboard::Key::Escape) {
                        resolveClassChoice(false);
                    }
                }
                continue;
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
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Escape) {
                        activityNotice.clear();
                    }
                } else if (event.is<sf::Event::MouseButtonPressed>()) {
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

        if (!classChoicePrompt.active && !mealChoicePrompt.active && !activityNotice.active) {
            float dx = 0.0f, dy = 0.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    dy = -1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  dy = 1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dx = -1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dx = 1.0f;

            if (dx != 0.0f && dy != 0.0f) {
                float inv = 1.0f / std::sqrt(2.0f);
                dx *= inv;
                dy *= inv;
            }

            player.move(dx, dy, dt);

            // 按键 C = 压力事件（降低 SAN，触发敌人出现）
            if (justPressed(sf::Keyboard::Key::C)) {
                player.modifyAttributes(Attributes(-15, 0, 0, 0, 0));
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
                player.modifyAttributes(Attributes(15, 0, 0, 0, 0));
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

            if (justPressed(sf::Keyboard::Key::Enter)) {
                // 第一层：场景传送门（进出建筑）
                bool portalFound = false;
                for (const auto& portal : currentMap->getPortals()) {
                    if (pointInRect(player.getPosition(), portal.area)) {
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
                                  << " (" << ip->actionId << ")"
                                  << " — " << ip->description << std::endl;
                        handleInteraction(*ip);
                    }
                }
            }

            player.update(dt);

            // 边界限制，防止走出地图
            currentMap->clampPlayer(player);
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
        if (activityNotice.active) {
            modalBox.setContent(activityNotice.title, activityNotice.body,
                                "Press Enter to continue");
            modalBox.render(window);
        }
        if (classChoicePrompt.active) {
            std::ostringstream body;
            body << classChoicePrompt.body << "\n\n"
                 << "[1] " << classChoicePrompt.first
                 << "\n[2] " << classChoicePrompt.second;
            if (!classChoicePrompt.third.empty())
                body << "\n[3] " << classChoicePrompt.third;
            modalBox.setContent(classChoicePrompt.title, body.str(),
                                classChoicePrompt.third.empty() ? "Press 1 or 2" : "Press 1, 2, or 3");
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
                                mealChoicePrompt.third.empty() ? "Press 1 or 2" : "Press 1, 2, or 3");
            modalBox.render(window);
        }

        window.display();
    }

    return 0;
}

