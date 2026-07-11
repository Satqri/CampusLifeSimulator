#pragma once

#include "screens/Screen.h"

class SceneBackground;
struct SceneTransition;

namespace sf {
class Font;
}

class SceneTransitionScreen final : public Screen {
public:
    SceneTransitionScreen(sf::Font& font, SceneBackground& background, SceneTransition& transition);

    void show() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window, TguiContext& tguiCtx) override;

private:
    sf::Font& font_;
    SceneBackground& background_;
    SceneTransition& transition_;
};
