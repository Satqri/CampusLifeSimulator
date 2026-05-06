#ifndef SLS_ENTITY_PLAYER_H
#define SLS_ENTITY_PLAYER_H

#include "entity/Character.h"

/**
 * @brief 战斗增益/减益效果
 */
struct CombatBuffs {
    bool nextEventPositive; ///< 下次事件选项加成（胜利 buff）
    int nextRollModifier;   ///< 下次检定修正值（失败 debuff 为负）

    CombatBuffs() : nextEventPositive(false), nextRollModifier(0) {}
    void clear() { nextEventPositive = false; nextRollModifier = 0; }
};

/**
 * @class Player
 * @brief 玩家类，游戏中的玩家角色
 *
 * 继承关系: Entity → Character → Player（3 层继承）
 * 属性: position (继承自 Entity), attributes (继承自 Character), moveCooldown, combatBuffs
 * 行为: 键盘移动、属性修改、SAN 值检测
 * 派生关系: 无（具体类，不再派生）
 */
class Player : public Character {
public:
    Player();
    Player(float x, float y);

    /**
     * @brief 玩家在地图上按方向移动
     * @param directionX X 方向分量（-1/0/+1）
     * @param directionY Y 方向分量（-1/0/+1）
     * @param deltaTime 帧间隔时间（秒）
     *
     * 算法步骤:
     * 1. 检查移动冷却时间是否已过
     * 2. 计算目标坐标（移动速度 × deltaTime × 方向）
     * 3. 如可移动，更新位置并重置冷却；否则保持原位
     */
    void move(float directionX, float directionY, float deltaTime) override;

    /**
     * @brief 更新玩家状态（每帧调用）
     * @param deltaTime 帧间隔时间（秒）
     *
     * 更新冷却时间递减等
     */
    void update(float deltaTime) override;

    /**
     * @brief 渲染玩家
     * @param window SFML 渲染窗口引用
     *
     * 使用 sf::RectangleShape 绘制像素风格角色方块
     */
    void render(sf::RenderWindow& window) override;

    /**
     * @brief 修改玩家属性
     * @param delta 属性变化量（正值增加，负值减少）
     *
     * 修改后自动 clamp 到合法范围
     */
    void modifyAttributes(const Attributes& delta);

    /**
     * @brief 检测 SAN 值是否处于危险水平
     * @return true 当 SAN < 30
     */
    bool isSanCritical() const;

    /**
     * @brief 检测 SAN 值是否极低（触发更强敌人）
     * @return true 当 SAN < 10
     */
    bool isSanDangerous() const;

    /** @brief 获取 SAN 值区间等级 (0=正常 >=30, 1=轻度 20-29, 2=中度 10-19, 3=危险 <10) */
    int getSanLevel() const;

    /** @brief 获取当前战斗增益/减益 */
    CombatBuffs& getCombatBuffs();
    const CombatBuffs& getCombatBuffs() const;

    /** @brief 清除所有战斗增益/减益 */
    void clearBuffs();

private:
    float moveCooldown;         ///< 移动冷却时间（秒）
    float moveCooldownTimer;    ///< 当前冷却计时器
    CombatBuffs combatBuffs;    ///< 战斗增益/减益效果
    sf::RectangleShape sprite;  ///< 简易像素精灵方块
};

#endif // SLS_ENTITY_PLAYER_H
