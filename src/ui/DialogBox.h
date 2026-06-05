#ifndef CLS_UI_DIALOGBOX_H
#define CLS_UI_DIALOGBOX_H

#include "ui/UIComponent.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

/**
 * @class DialogBox
 * @brief Reusable text panel for quest, event, and combat dialogs.
 */
class DialogBox : public UIComponent {
public:
    explicit DialogBox(sf::Font& font);

    void setBounds(const sf::Vector2f& position, const sf::Vector2f& size);
    void setOverlayEnabled(bool enabled);
    void setOverlayColor(const sf::Color& color);
    void setBackgroundColor(const sf::Color& color);
    void setTitle(const std::string& text);
    void setBody(const std::string& text);
    void addLine(const std::string& text, const sf::Vector2f& offset,
                 unsigned int size, const sf::Color& color);
    void clearLines();

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    struct Line {
        std::string text;
        sf::Vector2f offset;
        unsigned int size;
        sf::Color color;
    };

    sf::Font& font;
    sf::RectangleShape overlay;
    sf::RectangleShape panel;
    bool overlayEnabled;
    std::string title;
    std::string body;
    std::vector<Line> lines;
};

#endif // CLS_UI_DIALOGBOX_H
