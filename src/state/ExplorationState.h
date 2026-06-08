#ifndef CLS_STATE_EXPLORATIONSTATE_H
#define CLS_STATE_EXPLORATIONSTATE_H

#include "state/GameState.h"

/**
 * @class ExplorationState
 * @brief 探索状态，负责地图移动、传送门、交互与随机事件触发
 */
class ExplorationState : public GameState {
public:
    explicit ExplorationState(Game* game);

    void handleInput(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    bool tryHandleInteractionAt(sf::Vector2f position);
};

#endif // CLS_STATE_EXPLORATIONSTATE_H
