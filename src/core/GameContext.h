#ifndef CLS_ENGINE_GAMECONTEXT_H
#define CLS_ENGINE_GAMECONTEXT_H

#include "core/CombatResult.h"
#include "core/TimeSkipFlash.h"
#include "core/TimeSystem.h"
#include "core/Types.h"
#include "ui/ActivityNotice.h"
#include "ui/ChoicePrompt.h"
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
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

struct MealOption;
struct LibraryBook;

/**
 * @struct GameContext
 * @brief 打包 main() 作用域中的所有可变游戏状态，供各 handler 共享
 */
struct GameContext {
    // ── 玩家 + 地图 ──────────────────────────────────────────
    Player& player;
    CampusPlace& currentPlace;
    BuildingInterior*& currentMap;
    CampusMap* campusMap;
    DormitoryInterior* dormitoryMap;
    GymInterior* gymMap;
    LibraryInterior* libraryMap;
    ClassroomInterior* classroomMap;
    CafeteriaInterior* cafeteriaMap;

    // ── UI 状态 ──────────────────────────────────────────────
    TimeSystem& timeSystem;
    CombatResult& combatResult;
    TimeSkipFlash& timeSkipFlash;
    ActivityNotice& activityNotice;
    ChoicePrompt& classChoicePrompt;
    ChoicePrompt& mealChoicePrompt;

    // ── 地图过渡 ─────────────────────────────────────────────
    CampusPlace& pendingPlace;
    sf::Vector2f& pendingSpawnPosition;
    bool& hasPendingMapTransition;

    // ── 游戏数据 ─────────────────────────────────────────────
    std::vector<std::unique_ptr<Enemy>>& activeEnemies;
    std::vector<MealOption>& mealOptions;
    std::vector<LibraryBook>& libraryBooks;
    std::array<int, 4>& libraryBookProgress;
    int& selectedLibraryBook;
    int& heldMealIndex;
    int& lastMealPickupSlot;
    int& gamePlayDay;
    int& gamesPlayedToday;
    int& spawnCounter;

    // ── 辅助回调（由 main() 设置，避免循环依赖）───────────────
    std::function<void(int minutes, const Attributes& delta,
                       const std::string& title, const std::string& body)> runTimedActivity;
    std::function<void(const std::string& title, const std::string& body)> showTimedResult;
    std::function<void(int previousMinute)> checkClassSchedule;
    std::function<void(int previousMinute)> checkEventTriggers;
    std::function<void()> sleepFromDormitory;
};

#endif
