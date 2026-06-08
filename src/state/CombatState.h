#ifndef CLS_STATE_COMBATSTATE_H
#define CLS_STATE_COMBATSTATE_H

#include "state/GameState.h"

/**
 * @class CombatState
 * @brief 战斗状态，负责情绪敌人的 d20 对抗与结算
 */
class CombatState : public GameState {
public:
    explicit CombatState(Game* game);

    void onEnter() override;
    void handleInput(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};

#endif // CLS_STATE_COMBATSTATE_H
