#include "state/ExplorationState.h"

#include "core/WindowScaler.h"
#include "entity/Enemy.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "map/BuildingInterior.h"
#include "core/TextUtils.h"

#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <sstream>

ExplorationState::ExplorationState(Game* game)
    : GameState(game) {
}

void ExplorationState::handleInput(const sf::Event& event) {
    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        const auto code = keyEv->code;
        if (code == sf::Keyboard::Key::C) {
            game->clearMoveTarget();
            game->getPlayer().modifyAttributes(Attributes(-15, 0, 0, 0, 0));
            game->trySpawnEnemy();
            game->setStatusMessage(cls::text("status.stress"));
            return;
        }
        if (code == sf::Keyboard::Key::V) {
            game->clearMoveTarget();
            game->getPlayer().modifyAttributes(Attributes(15, 0, 0, 0, 0));
            game->setStatusMessage(cls::text("status.rest"));
            return;
        }
        if (code == sf::Keyboard::Key::X) {
            game->clearMoveTarget();
            game->getPlayer().getCombatBuffs().nextEventPositive = true;
            game->getPlayer().getCombatBuffs().nextRollModifier = 2;
            game->setStatusMessage(cls::text("status.buff_ready"));
            return;
        }
        if (code == sf::Keyboard::Key::F) {
            game->clearMoveTarget();
            if (!game->startCombatWithNearestEnemy()) {
                game->showNotice(cls::text("notice.no_enemy.title"), cls::text("notice.no_enemy.body"));
            }
            return;
        }
        if (code == sf::Keyboard::Key::Enter) {
            game->clearMoveTarget();
            tryHandleInteractionAt(game->getPlayer().getPosition());
            return;
        }
    } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
        const sf::Vector2f target = cls::mapPixelToGameCoords(game->getWindow(), mouseEv->position);
        if (!tryHandleInteractionAt(target)) {
            game->setMoveTarget(target);
        }
    }
}

void ExplorationState::update(float deltaTime) {
    float dx = 0.0f;
    float dy = 0.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) dy = -1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) dy = 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) dx = -1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dx = 1.0f;

    const bool keyboardMoving = dx != 0.0f || dy != 0.0f;
    if (keyboardMoving) {
        game->clearMoveTarget();
    }

    if (dx != 0.0f && dy != 0.0f) {
        const float inv = 1.0f / std::sqrt(2.0f);
        dx *= inv;
        dy *= inv;
    }

    Player& player = game->getPlayer();
    if (!keyboardMoving && game->hasMoveDestination()) {
        const sf::Vector2f pos = player.getPosition();
        const sf::Vector2f target = game->getMoveTarget();
        const float targetDx = target.x - pos.x;
        const float targetDy = target.y - pos.y;
        const float distance = std::sqrt(targetDx * targetDx + targetDy * targetDy);
        if (distance <= 8.0f) {
            game->clearMoveTarget();
            player.stopMovement();
        } else {
            dx = targetDx / distance;
            dy = targetDy / distance;
        }
    }

    player.move(dx, dy, deltaTime);
    player.update(deltaTime);

    BuildingInterior* currentMap = game->getCurrentMap();
    if (currentMap) currentMap->clampPlayer(player);
}

void ExplorationState::render(sf::RenderWindow& window) {
    BuildingInterior* currentMap = game->getCurrentMap();
    if (currentMap) currentMap->render(window);

    game->getPlayer().render(window);

    for (const auto& enemy : game->getActiveEnemies()) {
        enemy->render(window);

        sf::Text label = cls::makeText(game->getFont(), enemy->getName(), 11);
        label.setFillColor(sf::Color(255, 180, 80));
        const auto pos = enemy->getPosition();
        label.setPosition({pos.x - 20.0f, pos.y - 18.0f});
        window.draw(label);

        std::ostringstream ss;
        ss << "DC:" << enemy->getDC() << " ATK:" << enemy->getAttackPower();
        sf::Text info = cls::makeText(game->getFont(), ss.str(), 9);
        info.setFillColor(sf::Color(200, 160, 100));
        info.setPosition({pos.x - 20.0f, pos.y + 10.0f});
        window.draw(info);
    }
}

bool ExplorationState::tryHandleInteractionAt(sf::Vector2f position) {
    BuildingInterior* currentMap = game->getCurrentMap();
    if (!currentMap) return false;

    for (const auto& portal : currentMap->getPortals()) {
        if (pointInRect(position, portal.area)) {
            game->startMapTransition(portal);
            return true;
        }
    }

    const InteractionPoint* point = currentMap->getInteractionAt(position);
    if (!point) return false;

    game->getPlayer().setPosition(position.x, position.y);
    game->getPlayer().stopMovement();
    game->handleInteraction(*point);
    return true;
}
