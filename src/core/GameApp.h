#ifndef CLS_CORE_GAMEAPP_H
#define CLS_CORE_GAMEAPP_H

#include <SFML/Graphics.hpp>

#include "config/HiddenVariableConfig.h"
#include "core/GameSettings.h"
#include "core/GameSession.h"
#include "ui/DebugSandboxPanel.h"
#include "ui/DifficultyPanel.h"
#include "ui/HelpPanel.h"
#include "ui/ModalBox.h"
#include "ui/SceneBackground.h"
#include "ui/SettingsPanel.h"
#include "ui/TimePanel.h"
#include "ui/TitleScreen.h"

#include <string>
#include <vector>

class GameApp {
public:
    GameApp();

    int run();

private:
    enum class GameScreen {
        TITLE,
        SETTINGS,
        HELP,
        DIFFICULTY,
        GAME
    };

    bool initializeSettings();
    bool initializeHiddenVariables();
    void initializeWindow();
    void initializeFont();
    void initializeUi();

    void applyRuntimeSettings();
    void saveRuntimeSettings();
    void closeTitlePanel();
    void handleTitleAction(TitleAction action);
    void handleSettingsAction(SettingsAction action);

    void update(float dt);
    void render();
    void renderTitle();
    void renderSettings();
    void renderHelp();
    void renderDifficulty();
    void renderGame();

    void handleGlobalEvent(const sf::Event& event, bool& shouldContinue);
    void handleTitleEvent(const sf::Event& event);
    void handleSettingsEvent(const sf::Event& event);
    void handleHelpEvent(const sf::Event& event);
    void handleDifficultyEvent(const sf::Event& event);
    void handleGameEvent(const sf::Event& event);

    bool justPressed(sf::Keyboard::Key key);
    void updateGameplay(float dt);
    void handleGameplayActions();
    void handlePromptInput(sf::Keyboard::Key key);
    void handleActivityNoticeInput();
    void tryInteractOrPortal();

    sf::RenderWindow mWindow;
    sf::View mGameView;
    sf::Font mFont;
    bool mFontOk = false;

    cls::GameSettings mGameSettings;
    HiddenVariableConfig mHiddenConfig;

    TitleScreen mTitleScreen;
    DifficultyPanel mDifficultyPanel;
    SettingsPanel mSettingsPanel;
    HelpPanel mHelpPanel;
    SceneBackground mSceneBackground;
    SceneTransition mSceneTransition;
    TimePanel mTimePanel;
    ModalBox mModalBox;
    DebugSandboxPanel mDebugSandbox;
    GameSession mSession;

    GameScreen mScreen = GameScreen::TITLE;
    Difficulty mSelectedDifficulty = Difficulty::Normal;

    sf::Clock mClock;
    bool mKeyWasPressed[static_cast<int>(sf::Keyboard::KeyCount)] = {};
};

#endif
