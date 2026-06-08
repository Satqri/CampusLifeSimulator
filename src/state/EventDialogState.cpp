#include "state/EventDialogState.h"

#include "game/Game.h"
#include "map/BuildingInterior.h"

EventDialogState::EventDialogState(Game* game)
    : GameState(game) {
}

void EventDialogState::handleInput(const sf::Event& event) {
    if (!game->hasActiveDialog()) {
        game->requestStateChange(StateType::EXPLORATION);
        return;
    }

    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        if (!game->isChoiceDialog()) {
            if (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Escape) {
                game->confirmDialog(-1);
            }
            return;
        }

        const auto& options = game->getDialogOptions();
        if (keyEv->code == sf::Keyboard::Key::Escape) {
            game->confirmDialog(-1);
            return;
        }

        const sf::Keyboard::Key numericKeys[] = {
            sf::Keyboard::Key::Num1, sf::Keyboard::Key::Num2, sf::Keyboard::Key::Num3,
            sf::Keyboard::Key::Num4, sf::Keyboard::Key::Num5, sf::Keyboard::Key::Num6,
            sf::Keyboard::Key::Num7, sf::Keyboard::Key::Num8, sf::Keyboard::Key::Num9
        };
        const sf::Keyboard::Key keypadKeys[] = {
            sf::Keyboard::Key::Numpad1, sf::Keyboard::Key::Numpad2, sf::Keyboard::Key::Numpad3,
            sf::Keyboard::Key::Numpad4, sf::Keyboard::Key::Numpad5, sf::Keyboard::Key::Numpad6,
            sf::Keyboard::Key::Numpad7, sf::Keyboard::Key::Numpad8, sf::Keyboard::Key::Numpad9
        };

        for (std::size_t i = 0; i < options.size() && i < 9; ++i) {
            if (keyEv->code == numericKeys[i] || keyEv->code == keypadKeys[i]) {
                game->confirmDialog(static_cast<int>(i));
                return;
            }
        }

        if (keyEv->code == sf::Keyboard::Key::Enter && !options.empty()) {
            game->confirmDialog(0);
        }
    } else if (event.is<sf::Event::MouseButtonPressed>() && !game->isChoiceDialog()) {
        game->confirmDialog(-1);
    }
}

void EventDialogState::update(float deltaTime) {
    (void)deltaTime;
}

void EventDialogState::render(sf::RenderWindow& window) {
    BuildingInterior* currentMap = game->getCurrentMap();
    if (currentMap) currentMap->render(window);
    game->getPlayer().render(window);
}
