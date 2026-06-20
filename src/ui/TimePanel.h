#ifndef CLS_UI_TIMEPANEL_H
#define CLS_UI_TIMEPANEL_H

#include <SFML/Graphics.hpp>
#include "ui/UIComponent.h"

class TimeSystem;

/**
 * @class TimePanel
 * @brief 游戏内时钟面板 — 显示当前时间和天数
 */
class TimePanel : public UIComponent {
public:
    explicit TimePanel(sf::Font& font);
    void setTimeSystem(const TimeSystem* time);
    bool handleClick(sf::Vector2f point);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    sf::FloatRect collapsedBounds() const;
    sf::FloatRect expandedBounds() const;

    sf::Font& mFont;
    const TimeSystem* mTimeSystem = nullptr;
    bool mExpanded = false;
};

#endif
