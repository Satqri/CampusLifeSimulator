#ifndef CLS_UI_HELPPANEL_H
#define CLS_UI_HELPPANEL_H

#include "ui/UIComponent.h"

#include <SFML/Graphics.hpp>

/**
 * @class HelpPanel
 * @brief Help and settings page that explains controls and basic game rules.
 */
class HelpPanel : public UIComponent {
public:
    explicit HelpPanel(sf::Font& font);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    void drawLine(sf::RenderWindow& window, const std::string& text,
                  const sf::Vector2f& position, unsigned int size,
                  const sf::Color& color);

    sf::Font& font;
    sf::RectangleShape background;
    sf::RectangleShape panel;
};

#endif // CLS_UI_HELPPANEL_H
