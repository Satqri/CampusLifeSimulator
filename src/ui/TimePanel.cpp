#include "ui/TimePanel.h"
#include "core/TextUtils.h"
#include "core/TimeSystem.h"

TimePanel::TimePanel(sf::Font& font) : mFont(font) {}

void TimePanel::setTimeSystem(const TimeSystem* time) {
    mTimeSystem = time;
}

void TimePanel::update(float) {}

void TimePanel::render(sf::RenderWindow& window) {
    if (!mTimeSystem) return;

    sf::RectangleShape panel({182.0f, 34.0f});
    panel.setPosition({655.0f, 4.0f});
    panel.setFillColor(sf::Color(10, 18, 26, 230));
    panel.setOutlineColor(mTimeSystem->isMidtermDay() ? sf::Color(255, 190, 90) : sf::Color(80, 96, 118));
    panel.setOutlineThickness(1.0f);
    window.draw(panel);

    sf::Text clock = cls::makeText(mFont, mTimeSystem->clockText(), 12);
    clock.setFillColor(sf::Color(235, 238, 220));
    clock.setPosition({665.0f, 8.0f});
    window.draw(clock);

    sf::Text label = cls::makeText(mFont, mTimeSystem->dayLabel(), 10);
    label.setFillColor(mTimeSystem->isMidtermDay() ? sf::Color(255, 210, 120) : sf::Color(155, 180, 205));
    label.setPosition({665.0f, 23.0f});
    window.draw(label);
}
