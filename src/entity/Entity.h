#ifndef SLS_ENTITY_ENTITY_H
#define SLS_ENTITY_ENTITY_H

#include <SFML/Graphics.hpp>

/**
 * @class Entity
 * @brief 实体抽象基类，所有游戏实体的根类
 *
 * 继承关系: 根类，被 Character 继承（第 1 层）
 * 属性: position (posX, posY), visible, active
 * 行为: 提供 update() 和 render() 纯虚接口，子类多态实现
 * 派生关系: 派生 Character 抽象基类
 */
class Entity {
public:
    Entity() : posX(0), posY(0), visible(true), active(true) {}
    Entity(float x, float y) : posX(x), posY(y), visible(true), active(true) {}
    virtual ~Entity() = default;

    /**
     * @brief 每帧更新实体逻辑
     * @param deltaTime 帧间隔时间（秒）
     *
     * 纯虚函数，由具体子类实现各自的更新逻辑
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief 渲染实体到窗口
     * @param window SFML 渲染窗口引用
     *
     * 纯虚函数，由具体子类实现各自的渲染逻辑
     */
    virtual void render(sf::RenderWindow& window) = 0;

    /**
     * @brief 获取实体位置
     * @return sf::Vector2f 当前位置 (x, y)
     *
     * 公共接口访问私有成员 —— 封装性体现
     */
    sf::Vector2f getPosition() const { return sf::Vector2f(posX, posY); }

    /**
     * @brief 设置实体位置
     * @param x X 坐标
     * @param y Y 坐标
     */
    void setPosition(float x, float y) { posX = x; posY = y; }

    /** @brief 实体是否可见 */
    bool isVisible() const { return visible; }
    void setVisible(bool v) { visible = v; }

    /** @brief 实体是否活跃（参与更新） */
    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }

protected:
    float posX;   ///< X 坐标位置
    float posY;   ///< Y 坐标位置
    bool visible; ///< 可见性标志
    bool active;  ///< 活跃标志
};

#endif // SLS_ENTITY_ENTITY_H
