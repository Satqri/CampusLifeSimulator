#ifndef CLS_COMBAT_COMBATSYSTEM_H
#define CLS_COMBAT_COMBATSYSTEM_H

struct GameContext;
struct InteractionPoint;

namespace CombatSystem {

/**
 * @brief 根据 SAN 等级尝试在玩家附近随机生成敌人
 */
void trySpawnEnemy(GameContext& ctx);

/**
 * @brief 与附近敌人战斗（d20 对抗检定）
 * @return 是否找到了附近敌人并执行了战斗
 */
bool fightNearestEnemy(GameContext& ctx);

} // namespace CombatSystem

#endif
