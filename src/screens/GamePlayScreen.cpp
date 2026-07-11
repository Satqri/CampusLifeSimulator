#include "screens/GamePlayScreen.h"

#include <utility>

GamePlayScreen::GamePlayScreen(RenderCallback renderCallback)
    : renderCallback_(std::move(renderCallback)) {}

void GamePlayScreen::show() {}

void GamePlayScreen::update(float) {}

void GamePlayScreen::render(sf::RenderWindow& window, TguiContext& tguiCtx) {
    renderCallback_(window, tguiCtx);
}
