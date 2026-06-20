#include "ui/TimePanel.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"
#include "core/TimeSystem.h"

TimePanel::TimePanel(sf::Font& font) : mFont(font) {}

void TimePanel::setTimeSystem(const TimeSystem* time) {
    mTimeSystem = time;
}

sf::FloatRect TimePanel::collapsedBounds() const {
    return sf::FloatRect({900.0f, 506.0f}, {52.0f, 24.0f});
}

sf::FloatRect TimePanel::expandedBounds() const {
    return sf::FloatRect({736.0f, 46.0f}, {216.0f, 52.0f});
}

bool TimePanel::handleClick(sf::Vector2f point) {
    const sf::FloatRect bounds = mExpanded ? expandedBounds() : collapsedBounds();
    if (!bounds.contains(point)) return false;
    mExpanded = !mExpanded;
    return true;
}

void TimePanel::update(float) {}

void TimePanel::render(sf::RenderWindow& window) {
    if (!mTimeSystem) return;

    if (!mExpanded) {
        const sf::FloatRect bounds = collapsedBounds();
        sf::RectangleShape tab(bounds.size);
        tab.setPosition(bounds.position);
        tab.setFillColor(sf::Color(10, 18, 26, 218));
        tab.setOutlineColor(mTimeSystem->isMidtermDay() ? sf::Color(255, 190, 90) : sf::Color(80, 96, 118));
        tab.setOutlineThickness(1.0f);
        window.draw(tab);

        sf::Text label = cls::makeText(mFont, cls::text("time.toggle"), 11);
        label.setFillColor(sf::Color(235, 238, 220));
        label.setPosition({bounds.position.x + 10.0f, bounds.position.y + 5.0f});
        window.draw(label);
        return;
    }

    const sf::FloatRect bounds = expandedBounds();
    sf::RectangleShape panel(bounds.size);
    panel.setPosition(bounds.position);
    panel.setFillColor(sf::Color(10, 18, 26, 230));
    panel.setOutlineColor(mTimeSystem->isMidtermDay() ? sf::Color(255, 190, 90) : sf::Color(80, 96, 118));
    panel.setOutlineThickness(1.0f);
    window.draw(panel);

    sf::Text clock = cls::makeText(mFont, mTimeSystem->clockText(), 12);
    clock.setFillColor(sf::Color(235, 238, 220));
    clock.setPosition({bounds.position.x + 12.0f, bounds.position.y + 9.0f});
    window.draw(clock);

    sf::Text label = cls::makeText(mFont, mTimeSystem->dayLabel(), 10);
    label.setFillColor(mTimeSystem->isMidtermDay() ? sf::Color(255, 210, 120) : sf::Color(155, 180, 205));
    label.setPosition({bounds.position.x + 12.0f, bounds.position.y + 30.0f});
    window.draw(label);
}
