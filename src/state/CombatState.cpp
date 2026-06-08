#include "state/CombatState.h"

#include "combat/CombatSystem.h"
#include "core/TextUtils.h"
#include "entity/Enemy.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "map/BuildingInterior.h"

#include <sstream>

CombatState::CombatState(Game* game)
    : GameState(game) {
}

void CombatState::onEnter() {
    Enemy* enemy = game->getCurrentCombatEnemy();
    if (!enemy) {
        game->requestStateChange(StateType::EXPLORATION);
        return;
    }

    std::ostringstream intro;
    intro << enemy->getName() << " approaches. Press Enter to resolve a d20 confrontation."
          << "\nAction: " << CombatSystem::actionNameForEmotion(enemy->getEmotionType());
    game->setStatusMessage(intro.str());
}

void CombatState::handleInput(const sf::Event& event) {
    const auto* keyEv = event.getIf<sf::Event::KeyPressed>();
    if (!keyEv || keyEv->code != sf::Keyboard::Key::Enter) return;

    Enemy* enemy = game->getCurrentCombatEnemy();
    if (!enemy) {
        game->requestStateChange(StateType::EXPLORATION);
        return;
    }

    Player& player = game->getPlayer();
    const CombatRollResult roll = CombatSystem::resolveRoll(player, *enemy);
    game->finalizeCombat(roll.victory, roll.d20Roll, roll.modifier, roll.total, roll.dc);
}

void CombatState::update(float deltaTime) {
    (void)deltaTime;
}

void CombatState::render(sf::RenderWindow& window) {
    Player& player = game->getPlayer();
    BuildingInterior* currentMap = game->getCurrentMap();
    if (currentMap) currentMap->render(window);

    player.render(window);
    for (const auto& enemy : game->getActiveEnemies()) {
        enemy->render(window);
    }

    Enemy* currentEnemy = game->getCurrentCombatEnemy();
    if (!currentEnemy) return;

    sf::RectangleShape plate({520.0f, 150.0f});
    plate.setPosition({220.0f, 180.0f});
    plate.setFillColor(sf::Color(18, 18, 28, 230));
    plate.setOutlineColor(sf::Color(220, 180, 120));
    plate.setOutlineThickness(2.0f);
    window.draw(plate);

    sf::Text title = cls::makeText(game->getFont(), cls::text("combat.encounter"), 24);
    title.setFillColor(sf::Color(250, 238, 200));
    title.setPosition({246.0f, 200.0f});
    window.draw(title);

    std::ostringstream body;
    body << currentEnemy->getName() << "\n"
         << cls::text("combat.action") << ": " << CombatSystem::actionNameForEmotion(currentEnemy->getEmotionType()) << "\n"
         << cls::text("combat.enemy_dc") << ": " << currentEnemy->getDC() << "\n"
         << cls::text("combat.roll_buff") << ": " << player.getCombatBuffs().nextRollModifier << "\n\n"
         << cls::text("combat.press_enter");
    sf::Text text = cls::makeText(game->getFont(), body.str(), 16);
    text.setFillColor(sf::Color(220, 228, 220));
    text.setPosition({246.0f, 238.0f});
    window.draw(text);
}
