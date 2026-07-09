#include "core/GameApp.h"

#include "core/ActivityRunner.h"
#include "core/Localization.h"
#include "core/SleepSystem.h"
#include "ui/HUD.h"
#include "utils/AssetPath.h"
#include "utils/TextUtils.h"
#include "utils/WindowScaler.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

constexpr const char* kSettingsPath = "assets/config/settings.json";

void renderStatsPanel(sf::RenderWindow& window, sf::Font& font,
                      const Player& player) {
    HUD hud(font);
    hud.setPlayer(&player);
    hud.render(window);
}

void renderTimeSkipFlash(sf::RenderWindow& window, sf::Font& font, const TimeSkipFlash& flash) {
    if (!flash.active) return;
    sf::RectangleShape blackout({kRenderWidth, kRenderHeight});
    blackout.setFillColor(sf::Color(0, 0, 0, 245));
    window.draw(blackout);

    sf::Text text = cls::makeText(font, flash.text, 20);
    text.setFillColor(sf::Color(230, 230, 220));
    text.setPosition({360.0f, 252.0f});
    window.draw(text);
}

void renderSceneTransition(sf::RenderWindow& window, sf::Font& font,
                           SceneBackground& background, const SceneTransition& transition) {
    background.render(window, transition.background, sf::Color(0, 0, 0, 70));

    sf::RectangleShape plate({640.0f, 132.0f});
    plate.setPosition({160.0f, 332.0f});
    plate.setFillColor(sf::Color(9, 22, 28, 176));
    plate.setOutlineColor(sf::Color(230, 212, 148, 170));
    plate.setOutlineThickness(2.0f);
    window.draw(plate);

    sf::Text title = cls::makeText(font, transition.title, 34);
    title.setFillColor(sf::Color(250, 240, 205));
    title.setOutlineColor(sf::Color(18, 42, 45));
    title.setOutlineThickness(2.0f);
    title.setPosition({190.0f, 352.0f});
    window.draw(title);

    sf::Text subtitle = cls::makeText(font, transition.subtitle, 18);
    subtitle.setFillColor(sf::Color(224, 238, 220));
    subtitle.setPosition({190.0f, 407.0f});
    window.draw(subtitle);

    const bool ready = transition.canContinue();
    sf::Text hint = cls::makeText(font, ready ? cls::text("ui.enter_to_enter") : cls::text("ui.entering"), 13);
    hint.setFillColor(ready ? sf::Color(210, 210, 190, 180) : sf::Color(210, 210, 190, 110));
    hint.setPosition({190.0f, 444.0f});
    window.draw(hint);
}

bool containsPoint(const sf::Vector2f& point, const sf::FloatRect& rect) {
    return rect.contains(point);
}

} // namespace

GameApp::GameApp()
    : mGameView(sf::FloatRect({0.0f, 0.0f}, {kRenderWidth, kRenderHeight}))
    , mTitleScreen(mFont, "assets/ui/campus_title_bg.png")
    , mDifficultyPanel(mFont)
    , mSettingsPanel(mFont)
    , mHelpPanel(mFont)
    , mTimePanel(mFont)
    , mModalBox(mFont)
    , mDebugSandbox(mFont)
    , mSession(mFont) {
    initializeSettings();
    initializeHiddenVariables();
    initializeWindow();
    initializeFont();
    initializeUi();
}

bool GameApp::initializeSettings() {
    mGameSettings = cls::loadSettings(kSettingsPath);
#ifdef CLS_LANG_CHINESE
    if (!std::filesystem::exists(cls::resolveAssetPath(kSettingsPath))) {
        mGameSettings.language = cls::Language::Chinese;
    }
#endif
    cls::clampSettings(mGameSettings);
    cls::setLanguage(mGameSettings.language);
    return true;
}

bool GameApp::initializeHiddenVariables() {
    if (!mHiddenConfig.loadFromFile(cls::resolveAssetPath("assets/config/hidden_variables.json"))) {
        std::cerr << "[HiddenVars] Failed to load hidden_variables.json" << std::endl;
        return false;
    }
    initHiddenVariableConfig(mHiddenConfig);
    return true;
}

void GameApp::initializeWindow() {
    const auto& initialWindowSize = cls::windowScalePresets()[mGameSettings.windowScaleIndex];
    mWindow.create(sf::VideoMode({initialWindowSize.width, initialWindowSize.height}),
        "CampusLifeSimulator - Class Demo");
    mWindow.setFramerateLimit(60);
    mWindow.setKeyRepeatEnabled(false);
    mWindow.setView(mGameView);
}

void GameApp::initializeFont() {
    const std::vector<std::string> fontCandidates = {
        "assets/fonts/NotoSansCJKsc-Regular.otf",
#if defined(_WIN32)
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/arial.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
#elif defined(__linux__)
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJKsc-Regular.otf",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
#endif
    };
    for (const auto& path : fontCandidates) {
        const std::string resolved = cls::resolveAssetPath(path);
        if (mFont.openFromFile(resolved)) {
            std::cout << "[Font] Loaded: " << resolved << std::endl;
            mFontOk = true;
            break;
        } else {
            std::cout << "[Font] Failed: " << resolved << std::endl;
        }
    }
    if (!mFontOk) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
    }
}

void GameApp::initializeUi() {
    mSettingsPanel.setSettings(&mGameSettings);
    mSettingsPanel.setOverlayMode(false);
    mHelpPanel.setOverlayMode(false);
}

void GameApp::applyRuntimeSettings() {
    cls::clampSettings(mGameSettings);
    cls::setLanguage(mGameSettings.language);
    const auto& windowSize = cls::windowScalePresets()[mGameSettings.windowScaleIndex];
    cls::applyWindowSize(mWindow, mGameView, windowSize.width, windowSize.height);
}

void GameApp::saveRuntimeSettings() {
    if (!cls::saveSettings(kSettingsPath, mGameSettings)) {
        std::cerr << "[Settings] Failed to save settings: " << kSettingsPath << std::endl;
    }
}

void GameApp::closeTitlePanel() {
    mSettingsPanel.setEditing(false);
    mScreen = GameScreen::TITLE;
}

void GameApp::handleTitleAction(TitleAction action) {
    switch (action) {
        case TitleAction::Start:
            mScreen = GameScreen::DIFFICULTY;
            break;
        case TitleAction::Settings:
            mSettingsPanel.setEditing(false);
            mScreen = GameScreen::SETTINGS;
            break;
        case TitleAction::Help:
            mScreen = GameScreen::HELP;
            break;
        case TitleAction::None:
            break;
    }
}

void GameApp::handleSettingsAction(SettingsAction action) {
    if (action == SettingsAction::Changed) {
        applyRuntimeSettings();
        saveRuntimeSettings();
    } else if (action == SettingsAction::Close) {
        saveRuntimeSettings();
        closeTitlePanel();
    }
}

int GameApp::run() {
    if (!initializeHiddenVariables()) {
        return 1;
    }

    while (mWindow.isOpen()) {
        const float dt = mClock.restart().asSeconds();
        update(dt);

        while (const auto eventOpt = mWindow.pollEvent()) {
            const auto& event = *eventOpt;
            bool shouldContinue = false;
            handleGlobalEvent(event, shouldContinue);
            if (shouldContinue) continue;

            switch (mScreen) {
                case GameScreen::TITLE:
                    handleTitleEvent(event);
                    break;
                case GameScreen::SETTINGS:
                    handleSettingsEvent(event);
                    break;
                case GameScreen::HELP:
                    handleHelpEvent(event);
                    break;
                case GameScreen::DIFFICULTY:
                    handleDifficultyEvent(event);
                    break;
                case GameScreen::GAME:
                    handleGameEvent(event);
                    break;
            }
        }

        if (mScreen == GameScreen::GAME) {
            updateGameplay(dt);
        }

        render();
    }

    return 0;
}

void GameApp::update(float dt) {
    mSession.combatResult().update(dt);
    mSceneBackground.update(dt);
    mSceneTransition.update(dt);
    mSession.timeSkipFlash().update(dt);

    if (mScreen == GameScreen::TITLE) mTitleScreen.update(dt);
    if (mScreen == GameScreen::SETTINGS) mSettingsPanel.update(dt);
    if (mScreen == GameScreen::HELP) mHelpPanel.update(dt);
}

void GameApp::render() {
    if (mScreen == GameScreen::TITLE) {
        renderTitle();
        return;
    }
    if (mScreen == GameScreen::SETTINGS) {
        renderSettings();
        return;
    }
    if (mScreen == GameScreen::HELP) {
        renderHelp();
        return;
    }
    if (mScreen == GameScreen::DIFFICULTY) {
        renderDifficulty();
        return;
    }
    if (mSceneTransition.active) {
        mWindow.clear(sf::Color(20, 20, 30));
        renderSceneTransition(mWindow, mFont, mSceneBackground, mSceneTransition);
        mWindow.display();
        return;
    }
    if (mSession.timeSkipFlash().active) {
        mWindow.clear(sf::Color(0, 0, 0));
        renderTimeSkipFlash(mWindow, mFont, mSession.timeSkipFlash());
        mWindow.display();
        return;
    }
    renderGame();
}

void GameApp::renderTitle() {
    mWindow.clear(sf::Color(20, 20, 30));
    mTitleScreen.render(mWindow);
    mWindow.display();
}

void GameApp::renderSettings() {
    mWindow.clear(sf::Color(20, 20, 30));
    mSettingsPanel.render(mWindow);
    mWindow.display();
}

void GameApp::renderHelp() {
    mWindow.clear(sf::Color(20, 20, 30));
    mHelpPanel.render(mWindow);
    mWindow.display();
}

void GameApp::renderDifficulty() {
    mWindow.clear(sf::Color(20, 20, 30));
    mDifficultyPanel.render(mWindow);
    mWindow.display();
}

void GameApp::renderGame() {
    mWindow.clear(sf::Color(20, 20, 30));

    mSession.currentMap()->render(mWindow);
    mSession.player().render(mWindow);

    if (mFontOk) {
        renderStatsPanel(mWindow, mFont, mSession.player());
        mTimePanel.setTimeSystem(&mSession.timeSystem());
        mTimePanel.render(mWindow);
    }
    if (mSession.eventRunner().isActive()) {
        mSession.eventRunner().render(mWindow, mModalBox);
    }
    if (mSession.activityNotice().active) {
        if (mSession.settlementActive()) {
            const int page = mSession.settlementPage();
            const std::string title = page == 0
                ? cls::text("quest.final_result")
                : (page == 1 ? cls::text("quest.earned_titles") : cls::text("quest.semester_summary"));
            mModalBox.setContent(title, mSession.buildSettlementBody(mSession.settlementResult(), page),
                cls::text("quest.return_title"));
        } else {
            mModalBox.setContent(mSession.activityNotice().title, mSession.activityNotice().body,
                cls::text("ui.press_enter_continue"));
        }
        mModalBox.render(mWindow);
    }
    if (mSession.mealChoicePrompt().active) {
        std::ostringstream body;
        std::string footer;
        const auto& prompt = mSession.mealChoicePrompt();
        if (prompt.purpose == kDurationPromptPurpose) {
            body << prompt.body << "\n\n"
                 << cls::format("activity.duration.current",
                     {{"duration", durationLabel(prompt.selectedValue)}});
            footer = cls::text("activity.duration.footer");
        } else {
            body << prompt.body << "\n\n"
                 << "[1] " << prompt.first
                 << "\n[2] " << prompt.second;
            if (!prompt.third.empty()) body << "\n[3] " << prompt.third;
            if (!prompt.fourth.empty()) body << "\n[4] " << prompt.fourth;
            footer = !prompt.fourth.empty()
                ? cls::text("prompt.choice1234")
                : (prompt.third.empty() ? cls::text("prompt.choice12") : cls::text("prompt.choice123"));
        }
        mModalBox.setContent(prompt.title, body.str(), footer);
        mModalBox.render(mWindow);
    }
    if (mFontOk) {
        mDebugSandbox.refresh(mSession.context(), mSession.eventRunner());
        mDebugSandbox.render(mWindow);
    }

    mWindow.display();
}

void GameApp::handleGlobalEvent(const sf::Event& event, bool& shouldContinue) {
    if (event.is<sf::Event::Closed>()) {
        mWindow.close();
        shouldContinue = true;
        return;
    }

    if (mSceneTransition.active) {
        if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
            if (mSceneTransition.canContinue()
                && (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Escape)) {
                mSession.finishSceneTransition(mSceneTransition);
            }
        } else if (event.is<sf::Event::MouseButtonPressed>()) {
            if (mSceneTransition.canContinue()) {
                mSession.finishSceneTransition(mSceneTransition);
            }
        }
        shouldContinue = true;
        return;
    }

    if (mScreen == GameScreen::GAME) {
        if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
            const bool sandboxWasExpanded = mDebugSandbox.isExpanded();
            const bool sandboxConsumesKey = keyEv->code == sf::Keyboard::Key::F1
                || (mDebugSandbox.isEnabled() && keyEv->code == sf::Keyboard::Key::F2)
                || (sandboxWasExpanded
                    && (keyEv->code == sf::Keyboard::Key::Tab
                    || keyEv->code == sf::Keyboard::Key::Escape
                    || keyEv->code == sf::Keyboard::Key::Up
                    || keyEv->code == sf::Keyboard::Key::Down
                    || keyEv->code == sf::Keyboard::Key::Left
                    || keyEv->code == sf::Keyboard::Key::Right
                    || keyEv->code == sf::Keyboard::Key::W
                    || keyEv->code == sf::Keyboard::Key::A
                    || keyEv->code == sf::Keyboard::Key::S
                    || keyEv->code == sf::Keyboard::Key::D
                    || keyEv->code == sf::Keyboard::Key::Enter
                    || keyEv->code == sf::Keyboard::Key::Space
                    || keyEv->code == sf::Keyboard::Key::R
                    || keyEv->code == sf::Keyboard::Key::E
                    || keyEv->code == sf::Keyboard::Key::Delete));
            const DebugSandboxResult debugResult = mDebugSandbox.handleKey(
                keyEv->code, mSession.context(), mSession.eventRunner());
            if (debugResult.action != DebugSandboxAction::None) {
                std::cout << "[DebugSandbox] " << debugResult.message << std::endl;
            }
            if (sandboxConsumesKey) {
                shouldContinue = true;
                return;
            }
        }
    }
}

void GameApp::handleTitleEvent(const sf::Event& event) {
    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEv->code == sf::Keyboard::Key::A || keyEv->code == sf::Keyboard::Key::Left) {
            mTitleScreen.moveSelection(-1);
        } else if (keyEv->code == sf::Keyboard::Key::D || keyEv->code == sf::Keyboard::Key::Right) {
            mTitleScreen.moveSelection(1);
        } else if (keyEv->code == sf::Keyboard::Key::S) {
            handleTitleAction(TitleAction::Settings);
        } else if (keyEv->code == sf::Keyboard::Key::H) {
            handleTitleAction(TitleAction::Help);
        } else if (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Space) {
            handleTitleAction(mTitleScreen.confirmSelection());
        }
    } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
        const auto action = mTitleScreen.handleClick(mWindow.mapPixelToCoords(mouseEv->position, mGameView));
        handleTitleAction(action);
    }
}

void GameApp::handleSettingsEvent(const sf::Event& event) {
    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEv->code == sf::Keyboard::Key::Escape || keyEv->code == sf::Keyboard::Key::S) {
            saveRuntimeSettings();
            closeTitlePanel();
        } else if (keyEv->code == sf::Keyboard::Key::Up) {
            mSettingsPanel.moveSelection(-1);
        } else if (keyEv->code == sf::Keyboard::Key::Down) {
            mSettingsPanel.moveSelection(1);
        } else if (keyEv->code == sf::Keyboard::Key::Left || keyEv->code == sf::Keyboard::Key::A) {
            handleSettingsAction(mSettingsPanel.adjustCurrent(mGameSettings, -1));
        } else if (keyEv->code == sf::Keyboard::Key::Right || keyEv->code == sf::Keyboard::Key::D) {
            handleSettingsAction(mSettingsPanel.adjustCurrent(mGameSettings, 1));
        } else if (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Space) {
            handleSettingsAction(mSettingsPanel.confirmCurrent(mGameSettings));
        }
    } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
        handleSettingsAction(mSettingsPanel.handleClick(
            mWindow.mapPixelToCoords(mouseEv->position, mGameView),
            mGameSettings));
    }
}

void GameApp::handleHelpEvent(const sf::Event& event) {
    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEv->code == sf::Keyboard::Key::Escape
            || keyEv->code == sf::Keyboard::Key::H
            || keyEv->code == sf::Keyboard::Key::Enter
            || keyEv->code == sf::Keyboard::Key::Space) {
            closeTitlePanel();
        }
    } else if (event.is<sf::Event::MouseButtonPressed>()) {
        closeTitlePanel();
    }
}

void GameApp::handleDifficultyEvent(const sf::Event& event) {
    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEv->code == sf::Keyboard::Key::Escape) {
            mScreen = GameScreen::TITLE;
        } else if (keyEv->code == sf::Keyboard::Key::Num1) {
            mSelectedDifficulty = Difficulty::Easy;
            mSession.resetForNewGame(mSelectedDifficulty);
            mScreen = GameScreen::GAME;
        } else if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Enter) {
            mSelectedDifficulty = Difficulty::Normal;
            mSession.resetForNewGame(mSelectedDifficulty);
            mScreen = GameScreen::GAME;
        } else if (keyEv->code == sf::Keyboard::Key::Num3) {
            mSelectedDifficulty = Difficulty::Hard;
            mSession.resetForNewGame(mSelectedDifficulty);
            mScreen = GameScreen::GAME;
        }
    } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
        const auto action = mDifficultyPanel.handleClick(
            mWindow.mapPixelToCoords(mouseEv->position, mGameView));
        if (action.type == DifficultyActionType::Back) {
            mScreen = GameScreen::TITLE;
        } else if (action.type == DifficultyActionType::Select) {
            mSelectedDifficulty = action.difficulty;
            mSession.resetForNewGame(mSelectedDifficulty);
            mScreen = GameScreen::GAME;
        }
    }
}

void GameApp::handleGameEvent(const sf::Event& event) {
    if (mSession.timeSkipFlash().active) {
        return;
    }

    if (mSession.eventRunner().isActive()) {
        if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
            mSession.eventRunner().handleInput(keyEv->code, mSession.context());
        }
        return;
    }

    if (!mDebugSandbox.isExpanded() && !mSession.mealChoicePrompt().active && !mSession.activityNotice().active) {
        if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseEv->button == sf::Mouse::Button::Left) {
                const sf::Vector2f target = mWindow.mapPixelToCoords(mouseEv->position, mGameView);
                if (mTimePanel.handleClick(target)) {
                    return;
                }
                mSession.player().setMoveTarget(target);
            }
            return;
        }
    }

    if (mSession.mealChoicePrompt().active) {
        if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
            handlePromptInput(keyEv->code);
        }
        return;
    }

    if (mSession.activityNotice().active) {
        handleActivityNoticeInput();
        return;
    }
}

bool GameApp::justPressed(sf::Keyboard::Key key) {
    const bool pressed = sf::Keyboard::isKeyPressed(key);
    const bool previous = mKeyWasPressed[static_cast<int>(key)];
    mKeyWasPressed[static_cast<int>(key)] = pressed;
    return pressed && !previous;
}

void GameApp::updateGameplay(float dt) {
    if (mDebugSandbox.isExpanded() || mSession.mealChoicePrompt().active || mSession.activityNotice().active) {
        return;
    }

    float dx = 0.0f;
    float dy = 0.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) dy = -1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) dy = 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) dx = -1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dx = 1.0f;

    const bool hasKeyboardMove = dx != 0.0f || dy != 0.0f;
    if (hasKeyboardMove) {
        mSession.player().clearMoveTarget();
        if (dx != 0.0f && dy != 0.0f) {
            const float inv = 1.0f / std::sqrt(2.0f);
            dx *= inv;
            dy *= inv;
        }
        mSession.player().move(dx, dy, dt);
    } else {
        mSession.player().moveToTarget(dt);
    }

    handleGameplayActions();

    mSession.player().update(dt);
    mSession.currentMap()->clampPlayer(mSession.player());
    mSession.currentMap()->resolveCollisions(mSession.player());
}

void GameApp::handleGameplayActions() {
    if (justPressed(sf::Keyboard::Key::C)) {
        mSession.player().modifyAttributes(Attributes{.san = -15});
        mSession.maybeFinalizeRun();
        const int level = mSession.player().getSanLevel();
        std::cout << "[Stress] SAN dropped to " << mSession.player().getAttributes().san
                  << " Level=" << level << std::endl;
        mSession.trySpawnEnemy();
    }

    if (justPressed(sf::Keyboard::Key::F)) {
        if (!mSession.fightNearestEnemy()) {
            std::cout << "[Combat] No enemy nearby! Get closer or spawn one first (press C)." << std::endl;
        } else {
            mSession.maybeFinalizeRun();
        }
    }

    if (justPressed(sf::Keyboard::Key::V)) {
        mSession.player().modifyAttributes(Attributes{.san = 15});
        mSession.maybeFinalizeRun();
        const int level = mSession.player().getSanLevel();
        for (auto& enemy : mSession.activeEnemies()) {
            enemy->scaleWithSanLevel(level);
        }
        std::cout << "[Rest] SAN restored to " << mSession.player().getAttributes().san
                  << " Level=" << level << std::endl;
        if (level == 0) {
            mSession.activeEnemies().clear();
            std::cout << "[Combat] All enemies retreated (SAN >= 30)" << std::endl;
        } else if (level == 1 && static_cast<int>(mSession.activeEnemies().size()) > 1) {
            mSession.activeEnemies().resize(1);
            std::cout << "[Combat] Enemies reduced to 1 (SAN 20-29)" << std::endl;
        }
    }

    if (justPressed(sf::Keyboard::Key::X)) {
        mSession.player().getCombatBuffs().nextEventPositive = true;
        mSession.player().getCombatBuffs().nextRollModifier = 2;
        std::cout << "[Buff] Victory buff set: +2 to rolls" << std::endl;
    }

    if (justPressed(sf::Keyboard::Key::Enter) && !mSession.activityNotice().active) {
        tryInteractOrPortal();
    }
}

void GameApp::handlePromptInput(sf::Keyboard::Key key) {
    auto& prompt = mSession.mealChoicePrompt();
    if (prompt.purpose == kDurationPromptPurpose) {
        if (key == sf::Keyboard::Key::Left || key == sf::Keyboard::Key::A) {
            prompt.selectedValue = std::max(prompt.minValue, prompt.selectedValue - prompt.stepValue);
        } else if (key == sf::Keyboard::Key::Right || key == sf::Keyboard::Key::D) {
            prompt.selectedValue = std::min(prompt.maxValue, prompt.selectedValue + prompt.stepValue);
        } else if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space) {
            if (mSession.pendingTimedActivity().active) {
                const int selectedMinutes = prompt.selectedValue;
                const auto pending = mSession.pendingTimedActivity();
                mSession.pendingTimedActivity().clear();
                prompt.clear();
                executeTimedActivity(mSession.context(),
                    pending.baseMinutes, selectedMinutes, pending.delta,
                    pending.hiddenDelta, pending.hasHidden,
                    pending.title, pending.body, pending.activityId);
            }
        } else if (key == sf::Keyboard::Key::Escape) {
            mSession.pendingTimedActivity().clear();
            prompt.clear();
        }
        return;
    }

    if (prompt.purpose == kSleepPromptPurpose) {
        auto choiceIndex = [&]() {
            if (key == sf::Keyboard::Key::Num1 || key == sf::Keyboard::Key::Numpad1) return 0;
            if (key == sf::Keyboard::Key::Num2 || key == sf::Keyboard::Key::Numpad2) return 1;
            if (key == sf::Keyboard::Key::Num3 || key == sf::Keyboard::Key::Numpad3) return 2;
            if (key == sf::Keyboard::Key::Num4 || key == sf::Keyboard::Key::Numpad4) return 3;
            return -1;
        }();
        if (choiceIndex >= 0 && mSession.pendingSleep().active
            && choiceIndex < static_cast<int>(prompt.values.size())) {
            const int selectedMinutes = prompt.values[choiceIndex];
            const bool explicitAlarm = selectedMinutes > 0;
            mSession.pendingSleep().clear();
            prompt.clear();
            executeSleep(mSession.context(), mSession.settlementActive(), selectedMinutes, explicitAlarm);
        } else if (key == sf::Keyboard::Key::Escape) {
            mSession.pendingSleep().clear();
            prompt.clear();
        }
        return;
    }

    if (prompt.purpose == kDormitoryDeskPromptPurpose) {
        auto choiceIndex = [&]() {
            if (key == sf::Keyboard::Key::Num1 || key == sf::Keyboard::Key::Numpad1) return 0;
            if (key == sf::Keyboard::Key::Num2 || key == sf::Keyboard::Key::Numpad2) return 1;
            return -1;
        }();
        if (choiceIndex >= 0) {
            mSession.resolveDormitoryDeskChoice(choiceIndex);
        } else if (key == sf::Keyboard::Key::Escape) {
            prompt.clear();
        }
        return;
    }

    auto choiceIndex = [&]() {
        if (key == sf::Keyboard::Key::Num1 || key == sf::Keyboard::Key::Numpad1) return 0;
        if (key == sf::Keyboard::Key::Num2 || key == sf::Keyboard::Key::Numpad2) return 1;
        if (key == sf::Keyboard::Key::Num3 || key == sf::Keyboard::Key::Numpad3) return 2;
        if (key == sf::Keyboard::Key::Num4 || key == sf::Keyboard::Key::Numpad4) return 3;
        return -1;
    }();
    if (choiceIndex >= 0) {
        mSession.resolveMealChoice(choiceIndex);
    } else if (key == sf::Keyboard::Key::Escape) {
        mSession.pendingTimedActivity().clear();
        mSession.pendingSleep().clear();
        prompt.clear();
    }
}

void GameApp::handleActivityNoticeInput() {
    if (mSession.settlementActive()) {
        if (justPressed(sf::Keyboard::Key::Enter) || justPressed(sf::Keyboard::Key::Escape)) {
            if (mSession.settlementPage() < 2) {
                ++mSession.settlementPage();
                mSession.activityNotice().show(
                    mSession.settlementPage() == 1 ? cls::text("quest.earned_titles") : cls::text("quest.semester_summary"),
                    mSession.buildSettlementBody(mSession.settlementResult(), mSession.settlementPage()));
            } else {
                mSession.settlementActive() = false;
                mScreen = GameScreen::TITLE;
                mSession.activityNotice().clear();
            }
        }
    } else if (justPressed(sf::Keyboard::Key::Enter) || justPressed(sf::Keyboard::Key::Escape)) {
        mSession.activityNotice().clear();
    }
}

void GameApp::tryInteractOrPortal() {
    bool portalFound = false;
    for (const auto& portal : mSession.currentMap()->getPortals()) {
        static constexpr float kPortalMargin = 16.0f;
        const sf::FloatRect expanded(
            portal.area.position - sf::Vector2f(kPortalMargin, kPortalMargin),
            portal.area.size + sf::Vector2f(kPortalMargin * 2, kPortalMargin * 2));
        if (containsPoint(mSession.player().getPosition(), expanded)) {
            mSession.startMapTransition(portal, mSceneTransition);
            portalFound = true;
            break;
        }
    }
    if (!portalFound) {
        const InteractionPoint* ip = mSession.currentMap()->getInteractionAt(mSession.player().getPosition());
        if (ip) {
            std::cout << "[Interact] " << ip->label << " (" << ip->actionId << ")" << std::endl;
            mSession.handleInteraction(*ip);
        }
    }
}
