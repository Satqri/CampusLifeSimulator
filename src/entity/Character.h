#ifndef CLS_ENTITY_CHARACTER_H
#define CLS_ENTITY_CHARACTER_H

#include "entity/Entity.h"
#include "core/CharacterState.h"
#include "core/Types.h"
#include <string>

/**
 * @class Character
 * @brief 角色抽象基类，继承自 Entity
 *
 * 继承关系: Entity → Character（第 2 层），被 Player/Enemy 继承（第 3 层）
 * 属性: attributes (Attributes 结构体), moveSpeed
 * 行为: 提供 move() 纯虚接口、takeDamage() 虚函数、属性访问接口
 * 派生关系: 派生 Player 和 Enemy 具体类
 */
class Character : public Entity {
public:
    Character();
    Character(float x, float y, const Attributes& attrs, float speed);
    virtual ~Character() = default;

    /**
     * @brief 角色移动
     * @param directionX X 方向分量
     * @param directionY Y 方向分量
     * @param deltaTime 帧间隔时间（秒）
     *
     * 纯虚函数，玩家和敌人有不同的移动逻辑
     */
    virtual void move(float directionX, float directionY, float deltaTime) = 0;

    /**
     * @brief 受到伤害
     * @param damage 伤害值
     *
     * 虚函数，子类可覆盖以实现不同的受伤逻辑
     */
    virtual void takeDamage(int damage);

    /**
     * @brief 获取角色属性引用
     * @return Attributes& 当前属性
     */
    /** @brief 获取隐藏变量 */
    HiddenMap& getHidden() { return mHidden; }
    const HiddenMap& getHidden() const { return mHidden; }

    Attributes& getAttributes();
    const Attributes& getAttributes() const;

    /**
     * @brief 设置角色属性
     * @param attrs 新属性值
     */
    void setAttributes(const Attributes& attrs);

    /** @brief 获取移动速度 */
    float getMoveSpeed() const;
    void setMoveSpeed(float speed);

    /** @brief 获取角色名称 */
    const std::string& getName() const;
    void setName(const std::string& name);

    /**
     * @brief 限制属性在合法范围内
     *
     * 将 energy/health/san/academic/social clamp 到 [0, 100]，gold clamp 到 [0, 9999]
     */
    void clampAttributes();

protected:
    Attributes attributes; ///< 可见属性
    HiddenMap mHidden = HiddenMap::object(); ///< 隐藏变量 — 叙事累积状态
    float moveSpeed;       ///< 移动速度（像素/秒）
    std::string name;      ///< 角色名称
};

#endif // CLS_ENTITY_CHARACTER_H
