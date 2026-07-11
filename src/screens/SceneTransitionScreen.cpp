#include "screens/SceneTransitionScreen.h"

#include <SFML/Graphics.hpp>

#include "core/Localization.h"
#include "core/Types.h"
#include "ui/SceneBackground.h"
#include "ui/TguiContext.h"
#include "utils/TextUtils.h"
#include "utils/WindowScaler.h"

SceneTransitionScreen::SceneTransitionScreen(
    sf::Font& font, SceneBackground& background, SceneTransition& transition)
    : font_(font), background_(background), transition_(transition) {}

void SceneTransitionScreen::show() {}

void SceneTransitionScreen::update(float dt) {
    background_.update(dt);
    transition_.update(dt);
}

void SceneTransitionScreen::render(sf::RenderWindow& window, TguiContext& tguiCtx) {
    window.clear(sf::Color(20, 20, 30));
    background_.render(window, transition_.background, sf::Color(0, 0, 0, 70));

    sf::RectangleShape plate({640.0f, 132.0f});
    plate.setPosition({160.0f, 332.0f});
    plate.setFillColor(sf::Color(9, 22, 28, 176));
    plate.setOutlineColor(sf::Color(230, 212, 148, 170));
    plate.setOutlineThickness(2.0f);
    window.draw(plate);

    sf::Text title = cls::makeText(font_, transition_.title, 34);
    title.setFillColor(sf::Color(250, 240, 205));
    title.setOutlineColor(sf::Color(18, 42, 45));
    title.setOutlineThickness(2.0f);
    title.setPosition({190.0f, 352.0f});
    window.draw(title);

    sf::Text subtitle = cls::makeText(font_, transition_.subtitle, 18);
    subtitle.setFillColor(sf::Color(224, 238, 220));
    subtitle.setPosition({190.0f, 407.0f});
    window.draw(subtitle);

    const bool ready = transition_.canContinue();
    sf::Text hint = cls::makeText(
        font_, ready ? cls::text("ui.enter_to_enter") : cls::text("ui.entering"), 13);
    hint.setFillColor(ready ? sf::Color(210, 210, 190, 180) : sf::Color(210, 210, 190, 110));
    hint.setPosition({190.0f, 444.0f});
    window.draw(hint);

    tguiCtx.draw();
    window.display();
}
