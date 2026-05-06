#ifndef SLS_ENTITY_ENEMY_H
#define SLS_ENTITY_ENEMY_H

#include "entity/Character.h"

/**
 * @class Enemy
 * @brief 敌人/负面情绪类，继承自 Character
 *
 * 继承关系: Entity → Character → Enemy（3 层继承）
 * 属性: emotionType (情绪类型), dc (检定难度), attackPower (攻击力)
 * 行为: 战斗中与玩家骰子拼点，DC 和攻击力随玩家 SAN 值变化
 * 派生关系: 无（具体类，不再派生）
 */
class Enemy : public Character {
public:
    Enemy();
    Enemy(float x, float y, EmotionType type, int baseDC, int baseAttack);

    /**
     * @brief 敌人不在地图上移动，此实现为空
     */
    void move(float directionX, float directionY, float deltaTime) override;

    /**
     * @brief 更新敌人状态（每帧调用）
     * @param deltaTime 帧间隔时间（秒）
     */
    void update(float deltaTime) override;

    /**
     * @brief 渲染敌人
     * @param window SFML 渲染窗口引用
     *
     * 使用不同颜色区分情绪类型
     */
    void render(sf::RenderWindow& window) override;

    /**
     * @brief 根据玩家 SAN 等级调整敌人难度
     * @param sanLevel 玩家 SAN 等级 (0=正常, 1=轻度, 2=中度, 3=危险)
     *
     * 算法步骤:
     * 1. 根据 sanLevel 计算 DC 加成和攻击力加成
     * 2. SAN 30-20: DC +2
     * 3. SAN 20-10: DC +4, 攻击力 +3
     * 4. SAN <10: DC +6, 攻击力 +5
     */
    void scaleWithSanLevel(int sanLevel);

    /** @brief 获取当前检定难度 (Difficulty Class) */
    int getDC() const;

    /** @brief 获取攻击力 */
    int getAttackPower() const;

    /** @brief 获取情绪类型 */
    EmotionType getEmotionType() const;

    /** @brief 设置情绪类型及对应名称 */
    void setEmotionType(EmotionType type);

    /** @brief 获取敌人基础 DC（未缩放前） */
    int getBaseDC() const;

    /** @brief 获取敌人基础攻击力（未缩放前） */
    int getBaseAttack() const;

private:
    EmotionType emotionType; ///< 情绪类型
    int baseDC;              ///< 基础检定难度
    int baseAttack;          ///< 基础攻击力
    int scaledDC;            ///< 缩放后检定难度
    int scaledAttack;        ///< 缩放后攻击力
    sf::RectangleShape sprite; ///< 简易像素精灵方块

    /**
     * @brief 根据情绪类型获取对应颜色
     */
    static sf::Color colorForEmotion(EmotionType type);

    /**
     * @brief 根据情绪类型获取中文名称
     */
    static const char* nameForEmotion(EmotionType type);
};

#endif // SLS_ENTITY_ENEMY_H
