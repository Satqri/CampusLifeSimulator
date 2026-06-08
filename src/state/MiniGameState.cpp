#include "state/MiniGameState.h"

#include "core/Localization.h"
#include "core/TextUtils.h"
#include "game/Game.h"
#include "map/BuildingInterior.h"

#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cmath>
#include <sstream>

MiniGameState::MiniGameState(Game* game)
    : GameState(game)
    , cursorPosition(0.0f)
    , cursorVelocity(0.7f)
    , targetCenter(0.52f)
    , targetHalfWidth(0.08f)
    , elapsedTime(0.0f)
    , resolved(false) {
}

void MiniGameState::onEnter() {
    cursorPosition = 0.0f;
    cursorVelocity = 0.7f;
    targetCenter = 0.52f;
    targetHalfWidth = 0.08f;
    elapsedTime = 0.0f;
    resolved = false;
    game->setStatusMessage(cls::text("status.combat_intro"));
}

void MiniGameState::handleInput(const sf::Event& event) {
    const auto* keyEv = event.getIf<sf::Event::KeyPressed>();
    if (!keyEv || resolved) return;

    if (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Space) {
        resolveHit();
    }
}

void MiniGameState::update(float deltaTime) {
    if (resolved) return;

    elapsedTime += deltaTime;
    cursorPosition += cursorVelocity * deltaTime;
    if (cursorPosition >= 1.0f) {
        cursorPosition = 1.0f;
        cursorVelocity *= -1.0f;
    } else if (cursorPosition <= 0.0f) {
        cursorPosition = 0.0f;
        cursorVelocity *= -1.0f;
    }

    if (elapsedTime >= 4.5f) {
        resolveTimeout();
    }
}

void MiniGameState::render(sf::RenderWindow& window) {
    BuildingInterior* currentMap = game->getCurrentMap();
    if (currentMap) currentMap->render(window);
    game->getPlayer().render(window);

    sf::RectangleShape shade({960.0f, 540.0f});
    shade.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(shade);

    sf::RectangleShape panel({620.0f, 220.0f});
    panel.setPosition({170.0f, 150.0f});
    panel.setFillColor(sf::Color(16, 24, 30, 235));
    panel.setOutlineColor(sf::Color(230, 210, 148));
    panel.setOutlineThickness(2.0f);
    window.draw(panel);

    sf::Text title = cls::makeText(game->getFont(), cls::text("notice.minigame_title"), 24);
    title.setFillColor(sf::Color(245, 238, 208));
    title.setPosition({198.0f, 174.0f});
    window.draw(title);

    sf::Text prompt = cls::makeText(game->getFont(), cls::text("notice.minigame_prompt"), 16);
    prompt.setFillColor(sf::Color(220, 226, 220));
    prompt.setPosition({198.0f, 214.0f});
    window.draw(prompt);

    sf::RectangleShape lane({500.0f, 24.0f});
    lane.setPosition({230.0f, 270.0f});
    lane.setFillColor(sf::Color(52, 60, 66));
    lane.setOutlineColor(sf::Color(110, 126, 138));
    lane.setOutlineThickness(1.0f);
    window.draw(lane);

    sf::RectangleShape target({500.0f * targetHalfWidth * 2.0f, 24.0f});
    target.setPosition({230.0f + 500.0f * (targetCenter - targetHalfWidth), 270.0f});
    target.setFillColor(sf::Color(90, 188, 110));
    window.draw(target);

    sf::RectangleShape cursor({10.0f, 36.0f});
    cursor.setPosition({230.0f + 500.0f * cursorPosition - 5.0f, 264.0f});
    cursor.setFillColor(sf::Color(255, 220, 120));
    window.draw(cursor);

    const int remaining = std::max(0, static_cast<int>(std::ceil(4.5f - elapsedTime)));
    sf::Text timer = cls::makeText(game->getFont(),
        cls::format("notice.minigame_time", {{"count", std::to_string(remaining)}}), 14);
    timer.setFillColor(sf::Color(210, 220, 228));
    timer.setPosition({198.0f, 320.0f});
    window.draw(timer);
}

void MiniGameState::resolveHit() {
    resolved = true;
    const float distance = std::fabs(cursorPosition - targetCenter);

    Attributes delta;
    std::string title;
    std::string body;
    if (distance <= targetHalfWidth * 0.45f) {
        delta = Attributes(9, 6, 0, 0, 0);
        title = cls::text("notice.minigame_perfect");
        body = cls::text("notice.minigame_perfect_body");
    } else if (distance <= targetHalfWidth) {
        delta = Attributes(6, 4, 0, 0, 0);
        title = cls::text("notice.minigame_good");
        body = cls::text("notice.minigame_good_body");
    } else {
        delta = Attributes(2, 1, 0, 0, 0);
        title = cls::text("notice.minigame_messy");
        body = cls::text("notice.minigame_messy_body");
    }

    game->runTimedActivity(60, delta, title, body);
    game->requestStateChange(StateType::EXPLORATION);
}

void MiniGameState::resolveTimeout() {
    resolved = true;
    game->runTimedActivity(60, Attributes(1, -2, 0, 0, 0),
                           cls::text("notice.minigame_timeout"),
                           cls::text("notice.minigame_timeout_body"));
    game->requestStateChange(StateType::EXPLORATION);
}
