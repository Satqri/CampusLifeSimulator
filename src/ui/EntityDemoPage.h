#ifndef CLS_UI_ENTITYDEMOPAGE_H
#define CLS_UI_ENTITYDEMOPAGE_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "ui/UIComponent.h"

class Player;
class Enemy;
struct CombatResult;

/**
 * @class EntityDemoPage
 * @brief 实体演示页面 — 探索地图、敌人渲染、战斗结果覆盖
 *
 * 通过 setter 注入每帧变化的数据指针，避免每帧分配。
 */
class EntityDemoPage : public UIComponent {
public:
    explicit EntityDemoPage(sf::Font& font);

    void setPlayer(Player* player);
    void setActiveEnemies(const std::vector<std::unique_ptr<Enemy>>* enemies);
    void setCombatResult(const CombatResult* result);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    sf::Font& mFont;
    Player* mPlayer = nullptr;
    const std::vector<std::unique_ptr<Enemy>>* mActiveEnemies = nullptr;
    const CombatResult* mCombatResult = nullptr;
};

#endif
