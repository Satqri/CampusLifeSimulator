#include "ui/HUD.h"

#include "entity/Player.h"

#include <algorithm>
#include <sstream>

HUD::HUD(sf::Font& font)
    : font(font)
    , player(nullptr)
    , pageName("")
    , primaryHelp("")
    , secondaryHelp("")
    , panel({960.0f, 42.0f})
{
    panel.setFillColor(sf::Color(20, 20, 30, 230));
}

void HUD::setPlayer(const Player* p) {
    player = p;
}

void HUD::setPageName(const std::string& name) {
    pageName = name;
}

void HUD::setHelpText(const std::string& primary, const std::string& secondary) {
    primaryHelp = primary;
    secondaryHelp = secondary;
}

void HUD::update(float deltaTime) {
    (void)deltaTime;
}

void HUD::render(sf::RenderWindow& window) {
    window.draw(panel);
    if (!player) return;

    const auto& a = player->getAttributes();
    const auto& buffs = player->getCombatBuffs();

    std::ostringstream ss;
    ss << "Gold:" << a.gold
       << "  |  SAN Level:" << player->getSanLevel()
       << "  |  Buff:"
       << (buffs.nextEventPositive ? "[Win]" : "[-]")
       << " d" << (buffs.nextRollModifier >= 0 ? "+" : "")
       << buffs.nextRollModifier;

    sf::Text stats(font, ss.str(), 12);
    stats.setFillColor(sf::Color(200, 220, 255));
    stats.setPosition({8.0f, 6.0f});

    sf::Text page(font, pageName, 12);
    page.setFillColor(sf::Color(255, 200, 100));
    page.setPosition({840.0f, 6.0f});

    window.draw(stats);
    window.draw(page);
    drawAttributeBar(window, "SAN", a.san, 100, {8.0f, 25.0f}, sf::Color(120, 210, 255));
    drawAttributeBar(window, "EN", a.energy, 100, {190.0f, 25.0f}, sf::Color(100, 230, 150));
    drawAttributeBar(window, "ACD", a.academic, 100, {372.0f, 25.0f}, sf::Color(245, 205, 95));
    drawAttributeBar(window, "SOC", a.social, 100, {554.0f, 25.0f}, sf::Color(205, 140, 255));
}

void HUD::drawAttributeBar(sf::RenderWindow& window, const std::string& label,
                           int value, int maxValue, const sf::Vector2f& position,
                           const sf::Color& fillColor) {
    const int clamped = std::max(0, std::min(value, maxValue));
    const float width = 96.0f;
    const float ratio = maxValue > 0 ? static_cast<float>(clamped) / static_cast<float>(maxValue) : 0.0f;

    sf::Text labelText(font, label, 10);
    labelText.setFillColor(sf::Color(210, 215, 225));
    labelText.setPosition(position);

    sf::RectangleShape back({width, 8.0f});
    back.setPosition({position.x + 34.0f, position.y + 3.0f});
    back.setFillColor(sf::Color(45, 48, 58));
    back.setOutlineColor(sf::Color(80, 85, 100));
    back.setOutlineThickness(1.0f);

    sf::RectangleShape fill({width * ratio, 8.0f});
    fill.setPosition(back.getPosition());
    fill.setFillColor(fillColor);

    sf::Text valueText(font, std::to_string(clamped), 10);
    valueText.setFillColor(sf::Color(230, 235, 245));
    valueText.setPosition({position.x + 136.0f, position.y});

    window.draw(labelText);
    window.draw(back);
    window.draw(fill);
    window.draw(valueText);
}
