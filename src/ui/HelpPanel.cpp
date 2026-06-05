#include "ui/HelpPanel.h"

HelpPanel::HelpPanel(sf::Font& font)
    : font(font)
    , background({960.0f, 540.0f})
    , panel({820.0f, 410.0f})
{
    background.setFillColor(sf::Color(16, 18, 24));
    panel.setPosition({70.0f, 70.0f});
    panel.setFillColor(sf::Color(24, 27, 38, 235));
    panel.setOutlineColor(sf::Color(100, 110, 145));
    panel.setOutlineThickness(1.0f);
}

void HelpPanel::update(float deltaTime) {
    (void)deltaTime;
}

void HelpPanel::render(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(panel);

    drawLine(window, "Help / Settings", {100.0f, 95.0f}, 30, sf::Color::White);
    drawLine(window, "Page Switch", {105.0f, 150.0f}, 18, sf::Color(255, 210, 120));
    drawLine(window, "1 Entity Demo    2 Simple Quest    3 Midterm    4 Final    5 Quest Chain    0/6 Help", {125.0f, 180.0f}, 15, sf::Color(215, 220, 235));

    drawLine(window, "Exploration Controls", {105.0f, 230.0f}, 18, sf::Color(255, 210, 120));
    drawLine(window, "WASD / Arrow Keys: Move player", {125.0f, 260.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, "C: Stress event, lower SAN, may spawn emotion enemies", {125.0f, 285.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, "F: Fight nearby enemy with d20 roll    V: Restore SAN    X: Set test buff", {125.0f, 310.0f}, 15, sf::Color(215, 220, 235));

    drawLine(window, "Quest Controls", {105.0f, 360.0f}, 18, sf::Color(255, 210, 120));
    drawLine(window, "Enter: Continue / Confirm / Roll exam dice", {125.0f, 390.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, "Up / Down: Select quest choices or review option", {125.0f, 415.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, "Quest Chain page: E adds event count, S checks trigger, C resets chain", {125.0f, 440.0f}, 15, sf::Color(215, 220, 235));
}

void HelpPanel::drawLine(sf::RenderWindow& window, const std::string& text,
                         const sf::Vector2f& position, unsigned int size,
                         const sf::Color& color) {
    sf::Text line(font, text, size);
    line.setFillColor(color);
    line.setPosition(position);
    window.draw(line);
}
