#include "fileio/SaveManager.h"

#include <nlohmann/json.hpp>

SaveManager::SaveManager(const std::string& filePath)
    : JsonFileManager(filePath) {
}

bool SaveManager::saveGame(const SaveGameData& data) {
    jsonData.clear();
    jsonData["version"] = 1;
    jsonData["difficulty"] = static_cast<int>(data.difficulty);
    jsonData["player"] = {
        {"name", data.player.name},
        {"x", data.player.position.x},
        {"y", data.player.position.y},
        {"attributes", {
            {"energy", data.player.attributes.energy},
            {"health", data.player.attributes.health},
            {"gold", data.player.attributes.gold},
            {"san", data.player.attributes.san},
            {"academic", data.player.attributes.academic},
            {"social", data.player.attributes.social}
        }},
        {"hidden", data.player.hidden},
        {"combatBuffs", {
            {"nextEventPositive", data.player.nextEventPositive},
            {"nextRollModifier", data.player.nextRollModifier}
        }}
    };
    jsonData["time"] = {
        {"day", data.time.day},
        {"minute", data.time.minute},
        {"rollCallMinute", data.time.rollCallMinute},
        {"classPrompted", data.time.classPrompted},
        {"classResolved", data.time.classResolved}
    };
    jsonData["world"] = {
        {"currentPlace", static_cast<int>(data.world.currentPlace)},
        {"selectedLibraryBook", data.world.selectedLibraryBook},
        {"libraryBookProgress", data.world.libraryBookProgress},
        {"lastMealPickupSlot", data.world.lastMealPickupSlot},
        {"gamePlayDay", data.world.gamePlayDay},
        {"gamesPlayedToday", data.world.gamesPlayedToday}
    };
    jsonData["quest"] = {
        {"completedEventCount", data.quest.completedEventCount},
        {"currentQuestIndex", data.quest.currentQuestIndex}
    };

    nlohmann::json enemies = nlohmann::json::array();
    for (const auto& enemy : data.enemies) {
        enemies.push_back({
            {"x", enemy.position.x},
            {"y", enemy.position.y},
            {"emotionType", static_cast<int>(enemy.emotionType)},
            {"baseDC", enemy.baseDC},
            {"baseAttack", enemy.baseAttack}
        });
    }
    jsonData["enemies"] = enemies;
    return write();
}

bool SaveManager::loadGame(SaveGameData& data) {
    if (!read()) return false;

    const int saveVersion = jsonData.value("version", 0);
    data.difficulty = static_cast<Difficulty>(jsonData.value("difficulty", static_cast<int>(Difficulty::Normal)));

    const auto& playerJson = jsonData["player"];
    data.player.name = playerJson.value("name", std::string("Protagonist"));
    data.player.position = {playerJson.value("x", 480.0f), playerJson.value("y", 276.0f)};
    const auto& attrJson = playerJson["attributes"];
    const Attributes defaults = defaultPlayerAttributes();
    data.player.attributes.energy = attrJson.value("energy", defaults.energy);
    data.player.attributes.health = attrJson.value("health", defaults.health);
    data.player.attributes.gold = attrJson.value("gold", defaults.gold);
    data.player.attributes.san = attrJson.value("san", defaults.san);
    data.player.attributes.academic = attrJson.value("academic", defaults.academic);
    data.player.attributes.social = attrJson.value("social", defaults.social);
    if (playerJson.contains("hidden"))
        data.player.hidden = playerJson["hidden"];
    syncVisibleHealthFromHidden(data.player.attributes, data.player.hidden);
    const auto& buffJson = playerJson["combatBuffs"];
    data.player.nextEventPositive = buffJson.value("nextEventPositive", false);
    data.player.nextRollModifier = buffJson.value("nextRollModifier", 0);

    const auto& timeJson = jsonData["time"];
    data.time.day = timeJson.value("day", 1);
    data.time.minute = timeJson.value("minute", TimeSystem::kDayStartMinute);
    data.time.rollCallMinute = timeJson.value("rollCallMinute", 0);
    data.time.classPrompted = timeJson.value("classPrompted", false);
    data.time.classResolved = timeJson.value("classResolved", false);

    const auto& worldJson = jsonData["world"];
    data.world.currentPlace = static_cast<CampusPlace>(worldJson.value("currentPlace", static_cast<int>(CampusPlace::Campus)));
    data.world.selectedLibraryBook = worldJson.value("selectedLibraryBook", 0);
    const auto& progressJson = worldJson["libraryBookProgress"];
    for (std::size_t i = 0; i < data.world.libraryBookProgress.size() && i < progressJson.size(); ++i) {
        data.world.libraryBookProgress[i] = progressJson[i].get<int>();
    }
    data.world.lastMealPickupSlot = worldJson.value("lastMealPickupSlot", -1);
    data.world.gamePlayDay = worldJson.value("gamePlayDay", 1);
    data.world.gamesPlayedToday = worldJson.value("gamesPlayedToday", 0);

    const auto& questJson = jsonData["quest"];
    data.quest.completedEventCount = questJson.value("completedEventCount", 0);
    data.quest.currentQuestIndex = questJson.value("currentQuestIndex", 0);

    data.enemies.clear();
    for (const auto& enemyJson : jsonData["enemies"]) {
        SaveEnemyData enemy;
        enemy.position = {enemyJson.value("x", 480.0f), enemyJson.value("y", 276.0f)};
        enemy.emotionType = static_cast<EmotionType>(enemyJson.value("emotionType", 0));
        enemy.baseDC = enemyJson.value("baseDC", 12);
        enemy.baseAttack = enemyJson.value("baseAttack", 5);
        data.enemies.push_back(enemy);
    }
    return true;
}
