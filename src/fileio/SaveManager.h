#ifndef CLS_FILEIO_SAVEMANAGER_H
#define CLS_FILEIO_SAVEMANAGER_H

#include "core/TimeSystem.h"
#include "core/Types.h"
#include "entity/Enemy.h"
#include "fileio/JsonFileManager.h"
#include "map/MapPortal.h"
#include "ui/DifficultyPanel.h"

#include <array>
#include <vector>

/**
 * @struct SavePlayerData
 * @brief 玩家存档数据
 */
struct SavePlayerData {
    std::string name;
    sf::Vector2f position;
    Attributes attributes;
    HiddenMap hidden = HiddenMap::object();
    bool nextEventPositive = false;
    int nextRollModifier = 0;
};

/**
 * @struct SaveTimeData
 * @brief 时间存档数据
 */
struct SaveTimeData {
    int day = 1;
    int minute = TimeSystem::kDayStartMinute;
    bool classPrompted = false;
    bool classResolved = false;
};

/**
 * @struct SaveWorldData
 * @brief 世界存档数据
 */
struct SaveWorldData {
    CampusPlace currentPlace = CampusPlace::Campus;
    int selectedLibraryBook = 0;
    std::array<int, 4> libraryBookProgress = {0, 0, 0, 0};
    int heldMealIndex = -1;
    int lastMealPickupSlot = -1;
    int gamePlayDay = 1;
    int gamesPlayedToday = 0;
};

/**
 * @struct SaveQuestData
 * @brief 主线任务存档数据
 */
struct SaveQuestData {
    int completedEventCount = 0;
    int currentQuestIndex = 0;
};

/**
 * @struct SaveEnemyData
 * @brief 敌人存档数据
 */
struct SaveEnemyData {
    sf::Vector2f position;
    EmotionType emotionType = EmotionType::ANXIETY;
    int baseDC = 12;
    int baseAttack = 5;
};

/**
 * @struct SaveGameData
 * @brief 整体存档数据
 */
struct SaveGameData {
    Difficulty difficulty = Difficulty::Normal;
    SavePlayerData player;
    SaveTimeData time;
    SaveWorldData world;
    SaveQuestData quest;
    std::vector<SaveEnemyData> enemies;
};

/**
 * @class SaveManager
 * @brief 游戏进度 JSON 存档管理器
 */
class SaveManager : public JsonFileManager {
public:
    explicit SaveManager(const std::string& filePath);

    /** @brief 保存完整游戏数据 */
    bool saveGame(const SaveGameData& data);

    /** @brief 读取完整游戏数据 */
    bool loadGame(SaveGameData& data);
};

#endif // CLS_FILEIO_SAVEMANAGER_H
