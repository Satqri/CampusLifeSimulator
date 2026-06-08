#ifndef CLS_COMBAT_COMBATSYSTEM_H
#define CLS_COMBAT_COMBATSYSTEM_H

#include "core/Types.h"
#include "entity/Enemy.h"
#include "entity/Player.h"

#include <memory>
#include <optional>
#include <vector>

/**
 * @struct CombatRollResult
 * @brief 战斗 d20 检定结果
 */
struct CombatRollResult {
    bool victory = false;
    int d20Roll = 0;
    int modifier = 0;
    int total = 0;
    int dc = 0;
};

/**
 * @class CombatSystem
 * @brief 情绪敌人生成、查找与 d20 战斗结算系统
 */
class CombatSystem {
public:
    /** @brief 根据 SAN 等级尝试生成情绪敌人 */
    static bool trySpawnEnemy(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                              int& spawnCounter);

    /** @brief 查找玩家附近最近的敌人 */
    static std::optional<std::size_t> findNearestEnemy(const Player& player,
                                                       const std::vector<std::unique_ptr<Enemy>>& enemies,
                                                       float maxDistance = 100.0f);

    /** @brief 执行一次 d20 对抗检定 */
    static CombatRollResult resolveRoll(Player& player, const Enemy& enemy);

    /** @brief 获取情绪对应的玩家属性值 */
    static int statForEmotion(const Player& player, EmotionType type);

    /** @brief 获取情绪对应的行动名称 */
    static const char* actionNameForEmotion(EmotionType type);
};

#endif // CLS_COMBAT_COMBATSYSTEM_H
