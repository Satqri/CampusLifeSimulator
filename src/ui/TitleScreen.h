#ifndef CLS_UI_TITLE_SCREEN_H
#define CLS_UI_TITLE_SCREEN_H

#include "ui/UIComponent.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include <string>

/**
 * @enum TitleAction
 * @brief 标题界面按钮动作
 */
enum class TitleAction {
    None,
    Start,
    Settings,
    Help
};

/**
 * @class TitleScreen
 * @brief 游戏标题界面，支持键盘和鼠标导航
 */
class TitleScreen : public UIComponent {
public:
    TitleScreen(sf::Font& font, const std::string& backgroundPath);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    TitleAction handleClick(sf::Vector2f mousePosition);
    void moveSelection(int delta);
    void setSelection(std::size_t index);
    TitleAction confirmSelection() const;

private:
    struct Button {
        sf::FloatRect bounds;
        std::string labelKey;
        TitleAction action;
        sf::Color fill;
        sf::Color outline;
    };

    sf::Font& font;
    sf::Texture backgroundTexture;
    std::unique_ptr<sf::Sprite> backgroundSprite;
    std::array<Button, 3> buttons;
    std::size_t selectedIndex = 0;
    float elapsedTime = 0.0f;

    void drawButton(sf::RenderWindow& window, const Button& button, bool selected) const;
    void drawAmbientEffects(sf::RenderWindow& window) const;
    bool contains(const sf::FloatRect& bounds, sf::Vector2f point) const;
};

#endif // CLS_UI_TITLE_SCREEN_H
