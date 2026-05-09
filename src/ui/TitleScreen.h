#ifndef CLS_UI_TITLE_SCREEN_H
#define CLS_UI_TITLE_SCREEN_H

#include "ui/UIComponent.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

enum class TitleAction {
    None,
    Start,
    Help
};

class TitleScreen : public UIComponent {
public:
    TitleScreen(sf::Font& font, const std::string& backgroundPath);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    TitleAction handleClick(sf::Vector2f mousePosition) const;

private:
    sf::Font& font;
    sf::Texture backgroundTexture;
    std::unique_ptr<sf::Sprite> backgroundSprite;
    sf::FloatRect startButton;
    sf::FloatRect helpButton;
    float elapsedTime = 0.0f;

    void drawButton(sf::RenderWindow& window, const sf::FloatRect& bounds,
                    const std::string& label, const sf::Color& fill,
                    const sf::Color& outline) const;
    void drawAmbientEffects(sf::RenderWindow& window) const;
    bool contains(const sf::FloatRect& bounds, sf::Vector2f point) const;
};

#endif // CLS_UI_TITLE_SCREEN_H
