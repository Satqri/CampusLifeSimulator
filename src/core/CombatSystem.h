#ifndef CLS_COMBAT_COMBATSYSTEM_H
#define CLS_COMBAT_COMBATSYSTEM_H

#include "core/Types.h"
#include "entity/Enemy.h"
#include "entity/Player.h"

#include <memory>
#include <optional>
#include <vector>

struct GameContext;
struct InteractionPoint;

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

namespace CombatSystem {

// ── GameContext 版本（dev，当前 main.cpp 使用）────────────────

/** @brief 根据 SAN 等级尝试在玩家附近随机生成敌人 */
void trySpawnEnemy(GameContext& ctx);

/** @brief 与附近敌人战斗（d20 对抗检定） */
bool fightNearestEnemy(GameContext& ctx);

// ── 显式参数版本（moyoulingsen，Game 类 / 状态机使用）────────

/** @brief 根据 SAN 等级尝试生成情绪敌人（显式参数重载） */
bool trySpawnEnemy(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                   int& spawnCounter);

/** @brief 查找玩家附近最近的敌人 */
std::optional<std::size_t> findNearestEnemy(const Player& player,
                                            const std::vector<std::unique_ptr<Enemy>>& enemies,
                                            float maxDistance = 100.0f);

/** @brief 执行一次 d20 对抗检定 */
CombatRollResult resolveRoll(Player& player, const Enemy& enemy);

} // namespace CombatSystem

#endif
