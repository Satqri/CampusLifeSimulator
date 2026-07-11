#include "app/InputHandler.h"

#include <SFML/Window/Keyboard.hpp>

sf::Vector2f InputHandler::readMovementAxis() {
    sf::Vector2f axis{0.0f, 0.0f};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        axis.y = -1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        axis.y = 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
        axis.x = -1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
        axis.x = 1.0f;
    }

    return axis;
}
