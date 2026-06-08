#ifndef CLS_STATE_MINIGAMESTATE_H
#define CLS_STATE_MINIGAMESTATE_H

#include "state/GameState.h"

/**
 * @class MiniGameState
 * @brief 宿舍小游戏状态，负责短时节奏判定玩法与结果结算
 */
class MiniGameState : public GameState {
public:
    explicit MiniGameState(Game* game);

    void onEnter() override;
    void handleInput(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    float cursorPosition;
    float cursorVelocity;
    float targetCenter;
    float targetHalfWidth;
    float elapsedTime;
    bool resolved;

    void resolveHit();
    void resolveTimeout();
};

#endif // CLS_STATE_MINIGAMESTATE_H
