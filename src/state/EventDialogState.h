#ifndef CLS_STATE_EVENTDIALOGSTATE_H
#define CLS_STATE_EVENTDIALOGSTATE_H

#include "state/GameState.h"

/**
 * @class EventDialogState
 * @brief 事件对话状态，负责提示框与多选框交互
 */
class EventDialogState : public GameState {
public:
    explicit EventDialogState(Game* game);

    void handleInput(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
};

#endif // CLS_STATE_EVENTDIALOGSTATE_H
