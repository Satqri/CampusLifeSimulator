#ifndef CLS_ENTITY_PLAYER_H
#define CLS_ENTITY_PLAYER_H

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
 * 属性: position (继承自 Entity), attributes (继承自 Character), velocity, combatBuffs
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
     * 1. 根据输入方向计算目标速度
     * 2. 按加速度/减速度平滑调整当前速度
     * 3. 限制最大速度后推进玩家位置
     */
    void move(float directionX, float directionY, float deltaTime) override;

    /**
     * @brief 更新玩家状态（每帧调用）
     * @param deltaTime 帧间隔时间（秒）
     *
     * 当前用于保留玩家帧更新接口
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
     * @brief 每日属性结算
     *
     * 每天结束时调用，检测长期低属性状态并施加跨属性惩罚：
     * - 体力 < 30 连续 >= 2 天 → 扣健康
     * - 健康 < 30 连续 >= 2 天 → 扣压力 / 知识 / 社交
     */
    void dailyAttributeCheck();

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

    /**
     * @brief 停止当前移动速度
     *
     * 用于场景切换或重置位置后，避免残余惯性继续移动。
     */
    void stopMovement();

    /** @brief 获取低体力持续天数（用于 UI 提示） */
    int getLowEnergyDays() const { return lowEnergyDays; }
    /** @brief 获取低健康持续天数（用于 UI 提示） */
    int getLowHealthDays() const { return lowHealthDays; }

    /** @brief 是否存在鼠标点击移动目标 */
    bool hasMoveTarget() const { return moveTargetActive; }

    /**
     * @brief 设置鼠标点击移动目标
     * @param target 目标坐标
     */
    void setMoveTarget(sf::Vector2f target);

    /** @brief 清除鼠标点击移动目标 */
    void clearMoveTarget();

    /**
     * @brief 朝鼠标点击目标自动移动
     * @param deltaTime 帧间隔时间（秒）
     */
    void moveToTarget(float deltaTime);

private:
    sf::Vector2f velocity;      ///< 当前移动速度
    float acceleration;         ///< 按住方向键时的加速度
    float deceleration;         ///< 松开方向键时的减速度
    float stopSpeedThreshold;   ///< 低于该速度时直接停止，避免抖动
    CombatBuffs combatBuffs;    ///< 战斗增益/减益效果
    sf::RectangleShape sprite;  ///< 简易像素精灵方块
    int lowEnergyDays = 0;      ///< 连续低体力天数（>=2 触发扣健康）
    int lowHealthDays = 0;      ///< 连续低健康天数（>=2 触发扣压力/知识/社交）
    bool moveTargetActive = false; ///< 鼠标点击移动目标是否有效
    sf::Vector2f moveTarget;    ///< 鼠标点击移动目标坐标
};

#endif // CLS_ENTITY_PLAYER_H
