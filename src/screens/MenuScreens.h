#pragma once

#include "screens/Screen.h"

class DifficultyPanel;
class HelpPanel;
class SettingsPanel;
class TitleScreen;

class TitleMenuScreen final : public Screen {
public:
    explicit TitleMenuScreen(TitleScreen& titleScreen);

    void show() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window, TguiContext& tguiCtx) override;

private:
    TitleScreen& titleScreen_;
};

class SettingsMenuScreen final : public Screen {
public:
    explicit SettingsMenuScreen(SettingsPanel& settingsPanel);

    void show() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window, TguiContext& tguiCtx) override;

private:
    SettingsPanel& settingsPanel_;
};

class HelpMenuScreen final : public Screen {
public:
    explicit HelpMenuScreen(HelpPanel& helpPanel);

    void show() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window, TguiContext& tguiCtx) override;

private:
    HelpPanel& helpPanel_;
};

class DifficultyMenuScreen final : public Screen {
public:
    explicit DifficultyMenuScreen(DifficultyPanel& difficultyPanel);

    void show() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window, TguiContext& tguiCtx) override;

private:
    DifficultyPanel& difficultyPanel_;
};
