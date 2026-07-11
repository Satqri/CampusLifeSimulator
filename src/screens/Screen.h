#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

class TguiContext;

class Screen {
public:
    virtual ~Screen() = default;

    virtual void show() = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window, TguiContext& tguiCtx) = 0;
};
