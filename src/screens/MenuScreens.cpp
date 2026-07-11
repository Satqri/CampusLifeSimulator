#include "screens/MenuScreens.h"

#include "ui/DifficultyPanel.h"
#include "ui/HelpPanel.h"
#include "ui/SettingsPanel.h"
#include "ui/TguiContext.h"
#include "ui/TitleScreen.h"

namespace {

void clearMenuBackground(sf::RenderWindow& window) {
    window.clear(sf::Color(20, 20, 30));
}

} // namespace

TitleMenuScreen::TitleMenuScreen(TitleScreen& titleScreen)
    : titleScreen_(titleScreen) {}

void TitleMenuScreen::show() {
    titleScreen_.setVisible(true);
}

void TitleMenuScreen::update(float dt) {
    titleScreen_.update(dt);
}

void TitleMenuScreen::render(sf::RenderWindow& window, TguiContext& tguiCtx) {
    clearMenuBackground(window);
    titleScreen_.render(window);
    tguiCtx.draw();
    window.display();
}

SettingsMenuScreen::SettingsMenuScreen(SettingsPanel& settingsPanel)
    : settingsPanel_(settingsPanel) {}

void SettingsMenuScreen::show() {
    settingsPanel_.setVisible(true);
}

void SettingsMenuScreen::update(float) {}

void SettingsMenuScreen::render(sf::RenderWindow& window, TguiContext& tguiCtx) {
    clearMenuBackground(window);
    tguiCtx.draw();
    window.display();
}

HelpMenuScreen::HelpMenuScreen(HelpPanel& helpPanel)
    : helpPanel_(helpPanel) {}

void HelpMenuScreen::show() {
    helpPanel_.setVisible(true);
}

void HelpMenuScreen::update(float) {}

void HelpMenuScreen::render(sf::RenderWindow& window, TguiContext& tguiCtx) {
    clearMenuBackground(window);
    tguiCtx.draw();
    window.display();
}

DifficultyMenuScreen::DifficultyMenuScreen(DifficultyPanel& difficultyPanel)
    : difficultyPanel_(difficultyPanel) {}

void DifficultyMenuScreen::show() {
    difficultyPanel_.setVisible(true);
}

void DifficultyMenuScreen::update(float) {}

void DifficultyMenuScreen::render(sf::RenderWindow& window, TguiContext& tguiCtx) {
    clearMenuBackground(window);
    difficultyPanel_.render(window);
    tguiCtx.draw();
    window.display();
}
