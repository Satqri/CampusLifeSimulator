#include "ui/HelpPanel.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"

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

void HelpPanel::setOverlayMode(bool value) {
    overlayMode = value;
}

void HelpPanel::render(sf::RenderWindow& window) {
    background.setFillColor(overlayMode ? sf::Color(8, 10, 16, 190) : sf::Color(16, 18, 24));
    window.draw(background);
    window.draw(panel);

    drawLine(window, cls::text("help.title"), {100.0f, 95.0f}, 30, sf::Color::White);
    drawLine(window, cls::text("help.section.pages"), {105.0f, 150.0f}, 18, sf::Color(255, 210, 120));
    drawLine(window, cls::text("help.pages"), {125.0f, 180.0f}, 15, sf::Color(215, 220, 235));

    drawLine(window, cls::text("help.section.explore"), {105.0f, 230.0f}, 18, sf::Color(255, 210, 120));
    drawLine(window, cls::text("help.move"), {125.0f, 260.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, cls::text("help.click_move"), {125.0f, 285.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, cls::text("help.stress"), {125.0f, 310.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, cls::text("help.combat"), {125.0f, 335.0f}, 15, sf::Color(215, 220, 235));

    drawLine(window, cls::text("help.section.quest"), {105.0f, 380.0f}, 18, sf::Color(255, 210, 120));
    drawLine(window, cls::text("help.quest_enter"), {125.0f, 410.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, cls::text("help.quest_ud"), {125.0f, 435.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, cls::text("help.quest_chain"), {125.0f, 460.0f}, 15, sf::Color(215, 220, 235));
    drawLine(window, cls::text("help.global"), {125.0f, 485.0f}, 15, sf::Color(255, 238, 188));
    drawLine(window, cls::text("notice.standard_route_title"), {520.0f, 150.0f}, 18, sf::Color(255, 210, 120));
    drawLine(window, cls::text("notice.standard_route_body"), {520.0f, 180.0f}, 14, sf::Color(215, 220, 235));
    drawLine(window, cls::text("help.close"), {125.0f, 508.0f}, 14, sf::Color(215, 220, 235));
}

void HelpPanel::drawLine(sf::RenderWindow& window, const std::string& text,
                         const sf::Vector2f& position, unsigned int size,
                         const sf::Color& color) {
    sf::Text line = cls::makeText(font, text, size);
    line.setFillColor(color);
    line.setPosition(position);
    window.draw(line);
}
