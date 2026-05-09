#include "ui/DialogBox.h"

DialogBox::DialogBox(sf::Font& font)
    : font(font)
    , overlay({960.0f, 540.0f})
    , panel({840.0f, 450.0f})
    , overlayEnabled(true)
    , title("")
    , body("")
{
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    panel.setPosition({60.0f, 45.0f});
    panel.setFillColor(sf::Color(18, 18, 28, 220));
    panel.setOutlineColor(sf::Color(90, 90, 120));
    panel.setOutlineThickness(1.0f);
}

void DialogBox::setBounds(const sf::Vector2f& position, const sf::Vector2f& size) {
    panel.setPosition(position);
    panel.setSize(size);
}

void DialogBox::setOverlayEnabled(bool enabled) {
    overlayEnabled = enabled;
}

void DialogBox::setOverlayColor(const sf::Color& color) {
    overlay.setFillColor(color);
}

void DialogBox::setBackgroundColor(const sf::Color& color) {
    panel.setFillColor(color);
}

void DialogBox::setTitle(const std::string& text) {
    title = text;
}

void DialogBox::setBody(const std::string& text) {
    body = text;
}

void DialogBox::addLine(const std::string& text, const sf::Vector2f& offset,
                        unsigned int size, const sf::Color& color) {
    lines.push_back({text, offset, size, color});
}

void DialogBox::clearLines() {
    lines.clear();
}

void DialogBox::update(float deltaTime) {
    (void)deltaTime;
}

void DialogBox::render(sf::RenderWindow& window) {
    if (overlayEnabled) {
        window.draw(overlay);
    }
    window.draw(panel);

    const sf::Vector2f origin = panel.getPosition();

    sf::Text titleText(font, title, 28);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition({origin.x + 20.0f, origin.y + 15.0f});
    window.draw(titleText);

    sf::Text bodyText(font, body, 17);
    bodyText.setFillColor(sf::Color(220, 220, 220));
    bodyText.setPosition({origin.x + 20.0f, origin.y + 65.0f});
    window.draw(bodyText);

    for (const auto& line : lines) {
        sf::Text text(font, line.text, line.size);
        text.setFillColor(line.color);
        text.setPosition({origin.x + line.offset.x, origin.y + line.offset.y});
        window.draw(text);
    }
}
