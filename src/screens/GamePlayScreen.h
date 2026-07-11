#pragma once

#include "screens/Screen.h"

#include <functional>

class GamePlayScreen final : public Screen {
public:
    using RenderCallback = std::function<void(sf::RenderWindow&, TguiContext&)>;

    explicit GamePlayScreen(RenderCallback renderCallback);

    void show() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window, TguiContext& tguiCtx) override;

private:
    RenderCallback renderCallback_;
};
