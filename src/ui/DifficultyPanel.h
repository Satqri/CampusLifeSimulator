#ifndef CLS_UI_DIFFICULTY_PANEL_H
#define CLS_UI_DIFFICULTY_PANEL_H

#include "ui/UIComponent.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

enum class Difficulty {
    Easy,
    Normal,
    Hard
};

enum class DifficultyActionType {
    None,
    Back,
    Select
};

struct DifficultyAction {
    DifficultyActionType type = DifficultyActionType::None;
    Difficulty difficulty = Difficulty::Normal;
};

class DifficultyPanel : public UIComponent {
public:
    explicit DifficultyPanel(sf::Font& font);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    DifficultyAction handleClick(sf::Vector2f mousePosition) const;

private:
    struct Card {
        sf::FloatRect bounds;
        std::string title;
        std::string description;
        Difficulty difficulty;
        sf::Color accent;
    };

    sf::Font& font;
    sf::FloatRect backButton;
    std::array<Card, 3> cards;

    void drawCard(sf::RenderWindow& window, const Card& card) const;
    bool contains(const sf::FloatRect& bounds, sf::Vector2f point) const;
};

#endif // CLS_UI_DIFFICULTY_PANEL_H
