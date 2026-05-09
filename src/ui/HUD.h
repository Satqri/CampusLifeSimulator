#ifndef CLS_UI_HUD_H
#define CLS_UI_HUD_H

#include "ui/UIComponent.h"

#include <SFML/Graphics.hpp>
#include <string>

class Player;

/**
 * @class HUD
 * @brief Top status panel that displays player attributes and control hints.
 */
class HUD : public UIComponent {
public:
    explicit HUD(sf::Font& font);

    void setPlayer(const Player* player);
    void setPageName(const std::string& pageName);
    void setHelpText(const std::string& primary, const std::string& secondary);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    void drawAttributeBar(sf::RenderWindow& window, const std::string& label,
                          int value, int maxValue, const sf::Vector2f& position,
                          const sf::Color& fillColor);

    sf::Font& font;
    const Player* player;
    std::string pageName;
    std::string primaryHelp;
    std::string secondaryHelp;
    sf::RectangleShape panel;
};

#endif // CLS_UI_HUD_H
