#include "game/Game.h"

#include "combat/CombatSystem.h"
#include "core/AssetPath.h"
#include "core/Localization.h"
#include "core/TextUtils.h"
#include "core/WindowScaler.h"
#include "map/CafeteriaInterior.h"
#include "map/CampusMap.h"
#include "map/ClassroomInterior.h"
#include "map/DormitoryInterior.h"
#include "map/GymInterior.h"
#include "map/LibraryInterior.h"
#include "quest/MainQuest.h"
#include "state/CombatState.h"
#include "state/EventDialogState.h"
#include "state/ExplorationState.h"
#include "state/MainQuestState.h"
#include "state/MiniGameState.h"

#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

namespace {

std::string formatChoiceLines(const std::vector<std::string>& options) {
    std::ostringstream body;
    for (std::size_t i = 0; i < options.size(); ++i) {
        body << '[' << (i + 1) << "] " << options[i];
        if (i + 1 < options.size()) body << '\n';
    }
    return body.str();
}

} // namespace

void Game::SceneTransition::start(SceneBackgroundType bg, const std::string& heading, const std::string& line) {
    active = true;
    timer = 0.0f;
    background = bg;
    title = heading;
    subtitle = line;
}

void Game::SceneTransition::update(float deltaTime) {
    if (!active) return;
    timer += deltaTime;
}

bool Game::SceneTransition::canContinue() const {
    return timer >= 0.45f;
}

void Game::SceneTransition::clear() {
    active = false;
    timer = 0.0f;
    title.clear();
    subtitle.clear();
}

void Game::TimeSkipFlash::start(const std::string& message) {
    active = true;
    timer = 0.58f;
    text = message;
}

void Game::TimeSkipFlash::update(float deltaTime) {
    if (!active) return;
    timer -= deltaTime;
    if (timer <= 0.0f) clear();
}

void Game::TimeSkipFlash::clear() {
    active = false;
    timer = 0.0f;
    text.clear();
}

void Game::CombatResult::show(bool win, const std::string& name, int d20, int mod, int finalTotal, int difficultyClass) {
    active = true;
    victory = win;
    enemyName = name;
    d20Roll = d20;
    modifier = mod;
    total = finalTotal;
    dc = difficultyClass;
}

void Game::CombatResult::clear() {
    active = false;
    victory = false;
    enemyName.clear();
    d20Roll = 0;
    modifier = 0;
    total = 0;
    dc = 0;
}

Game::Game()
    : window(sf::VideoMode({kWindowWidth, kWindowHeight}), "CampusLifeSimulator")
    , gameView(sf::FloatRect({0.0f, 0.0f}, {kRenderWidth, kRenderHeight}))
    , font()
    , fontLoaded(false)
    , configManager(cls::resolveAssetPath("assets/config/settings.json"))
    , saveManager(cls::resolveAssetPath("assets/config/savegame.json"))
    , settings(configManager.loadSettings())
    , statusMessage("New game ready")
    , screen(Screen::Title)
    , previousScreen(Screen::Title)
    , helpOverlayOpen(false)
    , settingsModalOpen(false)
    , selectedDifficulty(Difficulty::Normal)
    , difficultyApplied(false)
    , settingsPanel(font)
    , titleScreen(font, cls::resolveAssetPath("assets/ui/campus_title_bg.png"))
    , difficultyPanel(font)
    , helpPanel(font)
    , hud(font)
    , sceneBackground()
    , timeSystem()
    , player(480.0f, 276.0f)
    , questManager()
    , campusMap()
    , dormitoryMap()
    , gymMap()
    , libraryMap()
    , classroomMap()
    , cafeteriaMap()
    , currentMap(nullptr)
    , currentPlace(CampusPlace::Campus)
    , pendingPlace(CampusPlace::Campus)
    , pendingSpawnPosition(480.0f, 276.0f)
    , hasPendingMapTransition(false)
    , mainQuestState()
    , explorationState()
    , eventDialogState()
    , combatState()
    , miniGameState()
    , currentState(nullptr)
    , currentStateType(std::nullopt)
    , pendingState(std::nullopt)
    , dialogReturnState(StateType::EXPLORATION)
    , sceneTransition()
    , timeSkipFlash()
    , combatResult()
    , dialogState()
    , activeEnemies()
    , activeCombatEnemyIndex(std::nullopt)
    , randomEventLibrary()
    , randomEventCounter(0)
    , spawnCounter(0)
    , randomEventPending(false)
    , hasMoveTarget(false)
    , moveTarget(480.0f, 276.0f)
    , selectedLibraryBook(0)
    , libraryBookProgress{0, 0, 0, 0}
    , libraryBookNames{"Reference Methods", "Literature Notes", "Science Primer", "Campus History"}
    , librarySkillNames{"Research", "Reflection", "Logic", "Context"}
    , mealOptions{{
        {"Meal A", 8,  Attributes(3, 12, 0, 1, 0), "Meal A: Gold -8, SAN +3, Energy +12, Social +1"},
        {"Meal B", 15, Attributes(6, 20, 0, 2, 0), "Meal B: Gold -15, SAN +6, Energy +20, Social +2"},
        {"Meal C", 28, Attributes(10, 30, 2, 4, 0), "Meal C: Gold -28, SAN +10, Energy +30, Academic +2, Social +4"}
      }}
    , heldMealIndex(-1)
    , lastMealPickupSlot(-1)
    , gamePlayDay(1)
    , gamesPlayedToday(0)
{
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    window.setView(gameView);

    cls::setLanguage(settings.language);
    const auto& scalePreset = cls::windowScalePresets()[settings.windowScaleIndex];
    cls::applyWindowSize(window, gameView, scalePreset.width, scalePreset.height);

    loadFont();
    settingsPanel.setSettings(&settings);
    player.setName("Protagonist");

    questManager.loadQuestChain(cls::resolveAssetPath("assets/config/quests.json"));
    initializeMaps();

    mainQuestState = std::make_unique<MainQuestState>(this, &questManager, &player);
    explorationState = std::make_unique<ExplorationState>(this);
    eventDialogState = std::make_unique<EventDialogState>(this);
    combatState = std::make_unique<CombatState>(this);
    miniGameState = std::make_unique<MiniGameState>(this);

    currentMap = campusMap.get();
}

int Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        const float deltaTime = clock.restart().asSeconds();

        while (const auto eventOpt = window.pollEvent()) {
            handleEvent(*eventOpt);
        }

        update(deltaTime);
        render();
    }
    return 0;
}

Game::~Game() = default;

sf::RenderWindow& Game::getWindow() {
    return window;
}

void Game::requestExitConfirmation() {
    showChoice(cls::text("notice.exit_confirm_title"),
               cls::text("notice.exit_confirm_body"),
               {cls::text("notice.exit_continue"), cls::text("notice.exit_now")},
               [this](int choice) {
                   if (choice == 1) {
                       window.close();
                   }
               });
}

bool Game::isExplorationState() const {
    return currentStateType.has_value() && *currentStateType == StateType::EXPLORATION;
}

void Game::setMoveTarget(sf::Vector2f target) {
    moveTarget = target;
    hasMoveTarget = true;
}

void Game::clearMoveTarget() {
    hasMoveTarget = false;
}

bool Game::hasMoveDestination() const {
    return hasMoveTarget;
}

sf::Vector2f Game::getMoveTarget() const {
    return moveTarget;
}

sf::Font& Game::getFont() {
    return font;
}

Player& Game::getPlayer() {
    return player;
}

TimeSystem& Game::getTimeSystem() {
    return timeSystem;
}

QuestManager& Game::getQuestManager() {
    return questManager;
}

BuildingInterior* Game::getCurrentMap() {
    return currentMap;
}

CampusPlace Game::getCurrentPlace() const {
    return currentPlace;
}

const std::vector<std::unique_ptr<Enemy>>& Game::getActiveEnemies() const {
    return activeEnemies;
}

Enemy* Game::getCurrentCombatEnemy() {
    if (!activeCombatEnemyIndex.has_value()) return nullptr;
    if (*activeCombatEnemyIndex >= activeEnemies.size()) return nullptr;
    return activeEnemies[*activeCombatEnemyIndex].get();
}

Difficulty Game::getDifficulty() const {
    return selectedDifficulty;
}

bool Game::isGameplayScreen() const {
    return screen == Screen::Gameplay;
}

bool Game::isHelpOpen() const {
    return helpOverlayOpen;
}

void Game::toggleHelpOverlay() {
    helpOverlayOpen = !helpOverlayOpen;
}

void Game::closeHelpOverlay() {
    helpOverlayOpen = false;
}

void Game::requestStateChange(StateType nextState) {
    pendingState = nextState;
}

void Game::changeState(StateType nextState) {
    if (currentState) currentState->onExit();

    currentState = nullptr;
    currentStateType = nextState;

    switch (nextState) {
        case StateType::EXPLORATION:
            currentState = explorationState.get();
            break;
        case StateType::EVENT_DIALOG:
            currentState = eventDialogState.get();
            break;
        case StateType::COMBAT:
            currentState = combatState.get();
            break;
        case StateType::MINI_GAME:
            currentState = miniGameState.get();
            break;
        case StateType::MAIN_QUEST:
            currentState = mainQuestState.get();
            break;
        default:
            currentState = explorationState.get();
            currentStateType = StateType::EXPLORATION;
            break;
    }

    if (currentState) currentState->onEnter();
}

void Game::resetNewGame(Difficulty difficulty) {
    selectedDifficulty = difficulty;
    difficultyApplied = false;
    player = Player(480.0f, 276.0f);
    player.setName("Protagonist");
    applyDifficulty(difficulty);

    timeSystem = TimeSystem();
    questManager = QuestManager();
    questManager.loadQuestChain(cls::resolveAssetPath("assets/config/quests.json"));

    activeEnemies.clear();
    activeCombatEnemyIndex.reset();
    combatResult.clear();
    clearDialog();
    timeSkipFlash.clear();
    sceneTransition.clear();
    randomEventPending = false;
    hasMoveTarget = false;
    selectedLibraryBook = 0;
    libraryBookProgress = {0, 0, 0, 0};
    heldMealIndex = -1;
    lastMealPickupSlot = -1;
    gamePlayDay = timeSystem.getDay();
    gamesPlayedToday = 0;
    spawnCounter = 0;
    randomEventCounter = 0;
    statusMessage = cls::text("status.new_game");

    setCurrentPlace(CampusPlace::Campus, {480.0f, 276.0f});
    screen = Screen::Gameplay;
    helpOverlayOpen = false;
    changeState(StateType::EXPLORATION);
}

bool Game::saveGame() {
    SaveGameData data;
    data.difficulty = selectedDifficulty;
    data.player.name = player.getName();
    data.player.position = player.getPosition();
    data.player.attributes = player.getAttributes();
    data.player.nextEventPositive = player.getCombatBuffs().nextEventPositive;
    data.player.nextRollModifier = player.getCombatBuffs().nextRollModifier;
    data.time.day = timeSystem.getDay();
    data.time.minute = timeSystem.getMinuteOfDay();
    data.time.classPrompted = timeSystem.isClassPrompted();
    data.time.classResolved = timeSystem.isClassResolved();
    data.world.currentPlace = currentPlace;
    data.world.selectedLibraryBook = selectedLibraryBook;
    data.world.libraryBookProgress = libraryBookProgress;
    data.world.heldMealIndex = heldMealIndex;
    data.world.lastMealPickupSlot = lastMealPickupSlot;
    data.world.gamePlayDay = gamePlayDay;
    data.world.gamesPlayedToday = gamesPlayedToday;
    data.quest.completedEventCount = questManager.getCompletedEventCount();
    data.quest.currentQuestIndex = questManager.getCurrentQuestIndex();

    for (const auto& enemy : activeEnemies) {
        SaveEnemyData enemyData;
        enemyData.position = enemy->getPosition();
        enemyData.emotionType = enemy->getEmotionType();
        enemyData.baseDC = enemy->getBaseDC();
        enemyData.baseAttack = enemy->getBaseAttack();
        data.enemies.push_back(enemyData);
    }

    if (!saveManager.saveGame(data)) {
        statusMessage = cls::text("status.save_failed");
        return false;
    }
    statusMessage = cls::text("status.saved");
    return true;
}

bool Game::loadGame() {
    SaveGameData data;
    if (!saveManager.loadGame(data)) {
        statusMessage = cls::text("status.no_save");
        return false;
    }

    selectedDifficulty = data.difficulty;
    difficultyApplied = true;
    player = Player(data.player.position.x, data.player.position.y);
    player.setName(data.player.name);
    player.setAttributes(data.player.attributes);
    player.getCombatBuffs().nextEventPositive = data.player.nextEventPositive;
    player.getCombatBuffs().nextRollModifier = data.player.nextRollModifier;

    timeSystem = TimeSystem();
    timeSystem.setTimeAbsolute(TimeSystem::kDayStartMinute);
    for (int i = 1; i < data.time.day; ++i) {
        timeSystem.sleepToNextDay();
    }
    timeSystem.setTimeAbsolute(data.time.minute);
    if (data.time.classPrompted) timeSystem.markClassPrompted();
    if (data.time.classResolved) timeSystem.markClassResolved();

    questManager = QuestManager();
    questManager.loadQuestChain(cls::resolveAssetPath("assets/config/quests.json"));
    for (int i = 0; i < data.quest.completedEventCount; ++i) questManager.onEventCompleted();
    for (int i = 0; i < data.quest.currentQuestIndex; ++i) questManager.onQuestCompleted();

    selectedLibraryBook = data.world.selectedLibraryBook;
    libraryBookProgress = data.world.libraryBookProgress;
    heldMealIndex = data.world.heldMealIndex;
    lastMealPickupSlot = data.world.lastMealPickupSlot;
    gamePlayDay = data.world.gamePlayDay;
    gamesPlayedToday = data.world.gamesPlayedToday;

    activeEnemies.clear();
    for (const auto& enemyData : data.enemies) {
        auto enemy = std::make_unique<Enemy>(
            enemyData.position.x,
            enemyData.position.y,
            enemyData.emotionType,
            enemyData.baseDC,
            enemyData.baseAttack);
        enemy->scaleWithSanLevel(player.getSanLevel());
        activeEnemies.push_back(std::move(enemy));
    }

    activeCombatEnemyIndex.reset();
    combatResult.clear();
    clearDialog();
    randomEventPending = false;
    hasMoveTarget = false;
    setCurrentPlace(data.world.currentPlace, player.getPosition(), false);
    player.stopMovement();

    screen = Screen::Gameplay;
    helpOverlayOpen = false;
    changeState(StateType::EXPLORATION);
    statusMessage = cls::text("status.loaded");
    return true;
}

void Game::showNotice(const std::string& title, const std::string& body, std::function<void()> onClose) {
    dialogState.mode = DialogMode::Notice;
    dialogState.title = title;
    dialogState.body = body;
    dialogState.options.clear();
    dialogState.onClose = std::move(onClose);
    dialogState.onChoose = {};
    dialogReturnState = currentStateType.value_or(StateType::EXPLORATION);
    requestStateChange(StateType::EVENT_DIALOG);
}

void Game::showChoice(const std::string& title, const std::string& body,
                      const std::vector<std::string>& options,
                      std::function<void(int)> onChoose) {
    dialogState.mode = DialogMode::Choice;
    dialogState.title = title;
    dialogState.body = body;
    dialogState.options = options;
    dialogState.onClose = {};
    dialogState.onChoose = std::move(onChoose);
    dialogReturnState = currentStateType.value_or(StateType::EXPLORATION);
    requestStateChange(StateType::EVENT_DIALOG);
}

void Game::clearDialog() {
    dialogState.mode = DialogMode::None;
    dialogState.title.clear();
    dialogState.body.clear();
    dialogState.options.clear();
    dialogState.onClose = {};
    dialogState.onChoose = {};
}

void Game::confirmDialog(int choiceIndex) {
    if (dialogState.mode == DialogMode::Notice) {
        auto callback = dialogState.onClose;
        clearDialog();
        if (callback) callback();
        requestStateChange(dialogReturnState);
        return;
    }

    if (dialogState.mode == DialogMode::Choice) {
        auto callback = dialogState.onChoose;
        clearDialog();
        if (callback) callback(choiceIndex);
        if (!hasActiveDialog()) {
            requestStateChange(dialogReturnState);
        }
    }
}

bool Game::hasActiveDialog() const {
    return dialogState.mode != DialogMode::None;
}

const std::string& Game::getDialogTitle() const {
    return dialogState.title;
}

const std::string& Game::getDialogBody() const {
    return dialogState.body;
}

const std::vector<std::string>& Game::getDialogOptions() const {
    return dialogState.options;
}

bool Game::isChoiceDialog() const {
    return dialogState.mode == DialogMode::Choice;
}

void Game::startTimeSkipFlash(const std::string& text) {
    timeSkipFlash.start(text);
}

bool Game::isTimeSkipFlashActive() const {
    return timeSkipFlash.active;
}

bool Game::isSceneTransitionActive() const {
    return sceneTransition.active;
}

void Game::finishSceneTransition() {
    if (hasPendingMapTransition) {
        setCurrentPlace(pendingPlace, pendingSpawnPosition);
        hasPendingMapTransition = false;
    }
    sceneTransition.clear();
}

void Game::setCurrentPlace(CampusPlace place, sf::Vector2f spawnPosition, bool stopMovement) {
    currentPlace = place;
    currentMap = mapForPlace(place);
    player.setPosition(spawnPosition.x, spawnPosition.y);
    if (stopMovement) player.stopMovement();
}

void Game::handleInteraction(const InteractionPoint& point) {
    const std::string& id = point.actionId;

    if (id.rfind("library_shelf_", 0) == 0) {
        selectedLibraryBook = std::clamp(id.back() - '0', 0, 3);
        std::ostringstream body;
        body << point.label << " selected " << libraryBookNames[selectedLibraryBook]
             << ". Reading progress: " << libraryBookProgress[selectedLibraryBook]
             << "%. Browsing does not consume time.";
        showNotice(cls::text("notice.shelf_browsed"), body.str());
        return;
    }

    if (id == "library_table") {
        const int book = selectedLibraryBook;
        libraryBookProgress[book] = std::min(100, libraryBookProgress[book] + 25);
        Attributes delta(-3, -6, 4, 0, 0);
        if (book == 1) delta = Attributes(2, -5, 0, 3, 0);
        if (book == 2) delta = Attributes(-4, -7, 6, 0, 0);
        if (book == 3) delta = Attributes(-2, -5, 2, 4, 0);

        std::ostringstream body;
        body << "Read " << libraryBookNames[book] << " for 30 minutes. "
             << librarySkillNames[book] << " progress is now "
             << libraryBookProgress[book] << "%.";
        runTimedActivity(30, delta, cls::text("notice.reading_complete"), body.str());
        return;
    }

    if (id == "cafeteria_counter") {
        if (!timeSystem.isMealTime()) {
            showNotice(cls::text("notice.meal_closed"), "Food is available from 12:00-14:00 and 17:00-19:00.");
            return;
        }
        if (heldMealIndex >= 0) {
            showNotice(cls::text("notice.meal_holding"), "You already have a tray. Sit at a table to finish it first.");
            return;
        }
        const int slot = timeSystem.mealSlotId();
        if (slot >= 0 && lastMealPickupSlot == slot) {
            showNotice(cls::text("notice.already_served"), "You can only take food once during the current meal period.");
            return;
        }
        showChoice(cls::text("notice.choose_meal"),
                   "Take food from the counter, then sit at a table to eat.",
                   {mealOptions[0].description, mealOptions[1].description, mealOptions[2].description},
                   [this](int choice) { resolveMealChoice(choice); });
        return;
    }

    if (id.rfind("cafeteria_table_", 0) == 0) {
        if (heldMealIndex < 0) {
            showNotice(cls::text("notice.no_food"), "Take Meal A, B, or C from the counter before eating at a table.");
            return;
        }
        const MealOption meal = mealOptions[heldMealIndex];
        heldMealIndex = -1;
        runTimedActivity(20, meal.reward, cls::text("notice.meal_complete"), meal.name + " eaten. " + meal.description + '.');
        return;
    }

    if (id.rfind("gym_treadmill_", 0) == 0) {
        runTimedActivity(40, Attributes(-4, -14, 0, 2, 0), cls::text("notice.training_complete"),
                         "Treadmill run: SAN -4, Energy -14, Social +2.");
        return;
    }

    if (id.rfind("gym_barbell_", 0) == 0) {
        runTimedActivity(40, Attributes(-5, -16, 0, 1, 0), cls::text("notice.training_complete"),
                         "Barbell training: SAN -5, Energy -16, Social +1.");
        return;
    }

    if (id == "dormitory_bed") {
        sleepFromDormitory();
        return;
    }

    if (id == "dormitory_desk") {
        runTimedActivity(45, Attributes(-6, -10, 7, 0, 0), cls::text("notice.study_complete"),
                         "Desk study: Academic +7, SAN -6, Energy -10.");
        return;
    }

    if (id == "dormitory_games") {
        if (gamePlayDay != timeSystem.getDay()) {
            gamePlayDay = timeSystem.getDay();
            gamesPlayedToday = 0;
        }
        ++gamesPlayedToday;
        requestStateChange(StateType::MINI_GAME);
        return;
    }

    if (id == "dormitory_rug") {
        showNotice(cls::text("notice.quiet_moment"), "You sit down and collect your thoughts. This currently does not consume time.");
        return;
    }

    if (id == "classroom_board") {
        showNotice(cls::text("notice.board_reviewed"), "The board shows today's notes. Reviewing here does not consume time.");
        return;
    }

    if (id.rfind("classroom_desk_", 0) == 0) {
        showNotice(cls::text("notice.desk"), "Morning class is handled by the 08:50 forced class event.");
        return;
    }

    showNotice(point.label, point.description);
}

void Game::runTimedActivity(int minutes, const Attributes& delta,
                            const std::string& title, const std::string& body,
                            bool countAsProgressEvent) {
    const int previousMinute = timeSystem.advanceMinutes(minutes);
    player.modifyAttributes(delta);
    startTimeSkipFlash(cls::text("time.passes"));
    showTimedResult(title, body);
    checkClassSchedule(previousMinute);
    if (countAsProgressEvent) registerProgressEvent();
}

void Game::trySpawnEnemy() {
    CombatSystem::trySpawnEnemy(player, activeEnemies, spawnCounter);
}

bool Game::startCombatWithNearestEnemy() {
    if (activeEnemies.empty() || activeCombatEnemyIndex.has_value()) return false;

    const auto nearest = CombatSystem::findNearestEnemy(player, activeEnemies);
    if (!nearest.has_value()) return false;

    activeCombatEnemyIndex = nearest;
    requestStateChange(StateType::COMBAT);
    return true;
}

void Game::finalizeCombat(bool victory, int d20Roll, int modifier, int total, int dc) {
    Enemy* enemy = getCurrentCombatEnemy();
    if (!enemy) {
        activeCombatEnemyIndex.reset();
        requestStateChange(StateType::EXPLORATION);
        return;
    }

    if (victory) {
        player.modifyAttributes(Attributes(20, 0, 0, 0, 0));
        player.getCombatBuffs().nextEventPositive = true;
        player.getCombatBuffs().nextRollModifier = 2;
        registerProgressEvent();
    } else {
        player.modifyAttributes(Attributes(-15, 0, 0, 0, 0));
        player.getCombatBuffs().nextEventPositive = false;
        player.getCombatBuffs().nextRollModifier = -2;
    }

    combatResult.show(victory, enemy->getName(), d20Roll, modifier, total, dc);
    activeEnemies.erase(activeEnemies.begin() + static_cast<std::ptrdiff_t>(*activeCombatEnemyIndex));
    activeCombatEnemyIndex.reset();
    requestStateChange(StateType::EXPLORATION);
}

std::string Game::gameplayLabel() const {
    switch (currentPlace) {
        case CampusPlace::Campus: return cls::text("page.entity");
        case CampusPlace::Dormitory: return cls::text("map.dormitory");
        case CampusPlace::Gym: return cls::text("map.gym");
        case CampusPlace::Library: return cls::text("map.library");
        case CampusPlace::Classroom: return cls::text("map.classroom");
        case CampusPlace::Cafeteria: return cls::text("map.cafeteria");
    }
    return cls::text("map.campus");
}

void Game::renderGlobalOverlays() {
    if (fontLoaded) {
        hud.setPlayer(&player);
        hud.setPageName(gameplayLabel());
        hud.setHelpText(cls::text("help.global"), cls::text("campus.move_hint"));
        hud.render(window);
        renderTimePanel();
    }

    if (combatResult.active) renderCombatOverlay();
    if (hasActiveDialog()) renderDialogOverlay();
    if (helpOverlayOpen) renderHelpOverlay();
}

void Game::renderHelpOverlay() {
    helpPanel.render(window);
}

cls::GameSettings& Game::getSettings() {
    return settings;
}

SettingsPanel& Game::getSettingsPanel() {
    return settingsPanel;
}

void Game::applyAndSaveSettings() {
    cls::clampSettings(settings);
    cls::setLanguage(settings.language);
    settingsPanel.setSettings(&settings);
    const auto& preset = cls::windowScalePresets()[settings.windowScaleIndex];
    cls::applyWindowSize(window, gameView, preset.width, preset.height);
    configManager.saveSettings(settings);
}

void Game::openSettingsScreen() {
    settingsPanel.setEditing(false);
    settingsPanel.setOverlayMode(true);
    previousScreen = screen;
    settingsModalOpen = true;
}

void Game::closeSettingsScreen() {
    settingsModalOpen = false;
    screen = previousScreen == Screen::Settings ? Screen::Title : previousScreen;
}

bool Game::isSettingsScreen() const {
    return settingsModalOpen;
}

const std::string& Game::getStatusMessage() const {
    return statusMessage;
}

void Game::setStatusMessage(const std::string& message) {
    statusMessage = message;
}

void Game::update(float deltaTime) {
    updateSharedSystems(deltaTime);

    switch (screen) {
        case Screen::Title:
            titleScreen.update(deltaTime);
            break;
        case Screen::Difficulty:
            difficultyPanel.update(deltaTime);
            break;
        case Screen::Settings:
            settingsPanel.update(deltaTime);
            break;
        case Screen::Help:
            helpPanel.update(deltaTime);
            break;
        case Screen::Gameplay:
            if (!settingsModalOpen && !sceneTransition.active && !timeSkipFlash.active && currentState && !helpOverlayOpen) {
                currentState->update(deltaTime);
            }
            break;
    }

    processPendingStateChange();
}

void Game::render() {
    window.clear(sf::Color(20, 20, 30));

    switch (screen) {
        case Screen::Title:
            titleScreen.render(window);
            break;
        case Screen::Difficulty:
            difficultyPanel.render(window);
            break;
        case Screen::Settings:
            settingsPanel.render(window);
            break;
        case Screen::Help:
            helpPanel.setOverlayMode(false);
            helpPanel.render(window);
            break;
        case Screen::Gameplay:
            if (sceneTransition.active) {
                renderSceneTransition();
            } else if (timeSkipFlash.active) {
                renderTimeSkipFlash();
            } else if (currentState) {
                currentState->render(window);
                renderGlobalOverlays();
                if (settingsModalOpen) settingsPanel.render(window);
            }
            break;
    }

    window.display();
}

void Game::handleEvent(const sf::Event& event) {
    if (event.is<sf::Event::Closed>()) {
        requestExitConfirmation();
        return;
    }

    if (settingsModalOpen && screen == Screen::Gameplay) {
        handleHelpEvent(event);
        return;
    }

    switch (screen) {
        case Screen::Title:
            handleTitleEvent(event);
            break;
        case Screen::Difficulty:
            handleDifficultyEvent(event);
            break;
        case Screen::Settings:
            handleHelpEvent(event);
            break;
        case Screen::Help:
            handleHelpEvent(event);
            break;
        case Screen::Gameplay:
            handleGameplayEvent(event);
            break;
    }
}

void Game::handleTitleEvent(const sf::Event& event) {
    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        const bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
        if (keyEv->code == sf::Keyboard::Key::Enter) {
            screen = Screen::Difficulty;
        } else if (ctrl && keyEv->code == sf::Keyboard::Key::S) {
            openSettingsScreen();
        } else if (ctrl && keyEv->code == sf::Keyboard::Key::H) {
            screen = Screen::Help;
        }
    } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
        const auto action = titleScreen.handleClick(cls::mapPixelToGameCoords(window, mouseEv->position));
        if (action == TitleAction::Start) {
            screen = Screen::Difficulty;
        } else if (action == TitleAction::Settings) {
            openSettingsScreen();
        } else if (action == TitleAction::Help) {
            screen = Screen::Help;
        }
    }
}

void Game::handleDifficultyEvent(const sf::Event& event) {
    auto startGameWithDifficulty = [this](Difficulty difficulty) {
        resetNewGame(difficulty);
    };

    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEv->code == sf::Keyboard::Key::Escape) {
            screen = Screen::Title;
        } else if (keyEv->code == sf::Keyboard::Key::Left || keyEv->code == sf::Keyboard::Key::A) {
            difficultyPanel.moveSelection(-1);
        } else if (keyEv->code == sf::Keyboard::Key::Right || keyEv->code == sf::Keyboard::Key::D) {
            difficultyPanel.moveSelection(1);
        } else if (keyEv->code == sf::Keyboard::Key::Up) {
            difficultyPanel.moveSelection(-1);
        } else if (keyEv->code == sf::Keyboard::Key::Down) {
            difficultyPanel.moveSelection(1);
        } else if (keyEv->code == sf::Keyboard::Key::Num1) {
            startGameWithDifficulty(Difficulty::Easy);
        } else if (keyEv->code == sf::Keyboard::Key::Num2) {
            startGameWithDifficulty(Difficulty::Normal);
        } else if (keyEv->code == sf::Keyboard::Key::Num3) {
            startGameWithDifficulty(Difficulty::Hard);
        } else if (keyEv->code == sf::Keyboard::Key::Enter) {
            const auto action = difficultyPanel.confirmSelection();
            if (action.type == DifficultyActionType::Back) {
                screen = Screen::Title;
            } else if (action.type == DifficultyActionType::Select) {
                startGameWithDifficulty(action.difficulty);
            }
        }
    } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
        const auto action = difficultyPanel.handleClick(cls::mapPixelToGameCoords(window, mouseEv->position));
        if (action.type == DifficultyActionType::Back) {
            screen = Screen::Title;
        } else if (action.type == DifficultyActionType::Select) {
            startGameWithDifficulty(action.difficulty);
        }
    }
}

void Game::handleHelpEvent(const sf::Event& event) {
    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        const bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
        if (screen == Screen::Settings || settingsModalOpen) {
            if (keyEv->code == sf::Keyboard::Key::Escape || (ctrl && keyEv->code == sf::Keyboard::Key::S)) {
                closeSettingsScreen();
            } else if (!settingsPanel.isEditing()) {
                if (keyEv->code == sf::Keyboard::Key::Up) {
                    settingsPanel.moveSelection(-1);
                } else if (keyEv->code == sf::Keyboard::Key::Down) {
                    settingsPanel.moveSelection(1);
                } else if (keyEv->code == sf::Keyboard::Key::Enter) {
                    const SettingsAction action = settingsPanel.confirmCurrent(settings);
                    if (action == SettingsAction::Changed) {
                        applyAndSaveSettings();
                    } else if (action == SettingsAction::Close) {
                        closeSettingsScreen();
                    }
                }
            } else {
                if (keyEv->code == sf::Keyboard::Key::Left) {
                    if (settingsPanel.adjustCurrent(settings, -1) == SettingsAction::Changed) {
                        applyAndSaveSettings();
                    }
                } else if (keyEv->code == sf::Keyboard::Key::Right) {
                    if (settingsPanel.adjustCurrent(settings, 1) == SettingsAction::Changed) {
                        applyAndSaveSettings();
                    }
                } else if (keyEv->code == sf::Keyboard::Key::Enter) {
                    if (settingsPanel.confirmCurrent(settings) == SettingsAction::Changed) {
                        applyAndSaveSettings();
                    }
                }
            }
            return;
        }

        if (keyEv->code == sf::Keyboard::Key::Escape || keyEv->code == sf::Keyboard::Key::Enter || (ctrl && keyEv->code == sf::Keyboard::Key::H)) {
            screen = Screen::Title;
        }
    } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (screen == Screen::Settings) {
            const SettingsAction action = settingsPanel.handleClick(
                cls::mapPixelToGameCoords(window, mouseEv->position), settings);
            if (action == SettingsAction::Changed) {
                applyAndSaveSettings();
            } else if (action == SettingsAction::Close) {
                closeSettingsScreen();
            }
            return;
        }
        screen = Screen::Title;
    }
}

void Game::handleGameplayEvent(const sf::Event& event) {
    if (sceneTransition.active) {
        if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
            if (sceneTransition.canContinue()
                && (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Escape)) {
                finishSceneTransition();
            }
        } else if (event.is<sf::Event::MouseButtonPressed>() && sceneTransition.canContinue()) {
            finishSceneTransition();
        }
        return;
    }

    if (timeSkipFlash.active) return;

    if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
        const bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
        if (ctrl && keyEv->code == sf::Keyboard::Key::H) {
            toggleHelpOverlay();
            return;
        }
        if (ctrl && keyEv->code == sf::Keyboard::Key::S) {
            openSettingsScreen();
            return;
        }
        if (keyEv->code == sf::Keyboard::Key::K) {
            saveGame();
            return;
        }
        if (keyEv->code == sf::Keyboard::Key::L) {
            loadGame();
            return;
        }
        if (keyEv->code == sf::Keyboard::Key::Escape && helpOverlayOpen) {
            closeHelpOverlay();
            return;
        }
        if (keyEv->code == sf::Keyboard::Key::Escape && !helpOverlayOpen && !hasActiveDialog()) {
            requestExitConfirmation();
            return;
        }
    }

    if (helpOverlayOpen) return;
    if (currentState) currentState->handleInput(event);
}

void Game::loadFont() {
#if defined(_WIN32)
    const std::vector<std::string> fontCandidates = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/arial.ttf",
    };
    for (const auto& path : fontCandidates) {
        if (font.openFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }
#elif defined(__APPLE__)
    fontLoaded = font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")
              || font.openFromFile("/System/Library/Fonts/PingFang.ttc");
#elif defined(__linux__)
    fontLoaded = font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
#endif
    if (!fontLoaded) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
    }
}

void Game::initializeMaps() {
    campusMap = std::make_unique<CampusMap>();
    dormitoryMap = std::make_unique<DormitoryInterior>();
    gymMap = std::make_unique<GymInterior>();
    libraryMap = std::make_unique<LibraryInterior>();
    classroomMap = std::make_unique<ClassroomInterior>();
    cafeteriaMap = std::make_unique<CafeteriaInterior>();

    campusMap->setFont(&font);
    campusMap->setTimeSystem(&timeSystem);
    dormitoryMap->setFont(&font);
    gymMap->setFont(&font);
    libraryMap->setFont(&font);
    classroomMap->setFont(&font);
    cafeteriaMap->setFont(&font);
}

BuildingInterior* Game::mapForPlace(CampusPlace place) const {
    switch (place) {
        case CampusPlace::Campus: return campusMap.get();
        case CampusPlace::Dormitory: return dormitoryMap.get();
        case CampusPlace::Gym: return gymMap.get();
        case CampusPlace::Library: return libraryMap.get();
        case CampusPlace::Classroom: return classroomMap.get();
        case CampusPlace::Cafeteria: return cafeteriaMap.get();
    }
    return campusMap.get();
}

void Game::updateSharedSystems(float deltaTime) {
    sceneBackground.update(deltaTime);
    sceneTransition.update(deltaTime);
    timeSkipFlash.update(deltaTime);
}

void Game::processPendingStateChange() {
    if (!pendingState.has_value()) return;
    const StateType next = *pendingState;
    pendingState.reset();
    changeState(next);
}

void Game::applyDifficulty(Difficulty difficulty) {
    if (difficultyApplied) return;

    switch (difficulty) {
        case Difficulty::Easy:
            player.modifyAttributes(Attributes(20, 15, 5, 5, 0));
            break;
        case Difficulty::Normal:
            break;
        case Difficulty::Hard:
            player.modifyAttributes(Attributes(-20, -10, 0, -5, 0));
            break;
    }
    difficultyApplied = true;
}

void Game::triggerQuestIfNeeded() {
    if (questManager.shouldTriggerQuest()) {
        requestStateChange(StateType::MAIN_QUEST);
    }
}

void Game::registerProgressEvent() {
    questManager.onEventCompleted();
    maybeTriggerRandomEvent("local-progress");
    triggerQuestIfNeeded();
}

void Game::maybeTriggerRandomEvent(const std::string& source) {
    const auto& randomEvents = randomEventLibrary.events();
    if (randomEventPending || randomEvents.empty()) return;
    if ((std::rand() % 100) >= 35) return;

    randomEventPending = true;
    const RandomEventDefinition& eventDef = randomEvents[static_cast<std::size_t>(randomEventCounter % static_cast<int>(randomEvents.size()))];
    ++randomEventCounter;
    presentRandomEvent(eventDef, source);
}

void Game::presentRandomEvent(const RandomEventDefinition& eventDef, const std::string& source) {
    std::string body = eventDef.body + "\n\nSource: " + source;
    if (eventDef.llmHookSuggested) {
        body += "\n\n" + randomEventLibrary.llmFallbackHint();
    }

    showChoice(eventDef.title, body, eventDef.options, [this, eventDef](int choice) {
        if (choice >= 0 && choice < static_cast<int>(eventDef.deltas.size())) {
            player.modifyAttributes(eventDef.deltas[choice]);
        }
        randomEventPending = false;
        std::ostringstream summary;
        summary << eventDef.title << " resolved.";
        showNotice(cls::text("notice.event_result"), summary.str());
    });
}

void Game::startMapTransition(const MapPortal& portal) {
    pendingPlace = portal.target;
    pendingSpawnPosition = portal.spawnPosition;
    hasPendingMapTransition = true;
    sceneTransition.start(portal.transitionBackground, portal.title, portal.subtitle);
}

void Game::forceMorningClass() {
    timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
    timeSystem.markClassPrompted();
    setCurrentPlace(CampusPlace::Classroom, {480.0f, 276.0f});

    std::vector<const InteractionPoint*> desks;
    for (const auto& ip : classroomMap->getInteractionPoints()) {
        if (ip.label == "Sit at Desk") desks.push_back(&ip);
    }
    if (!desks.empty()) {
        const InteractionPoint* desk = desks[std::rand() % desks.size()];
        player.setPosition(desk->area.position.x + desk->area.size.x * 0.5f,
                           desk->area.position.y + desk->area.size.y * 0.5f);
    }
    player.stopMovement();

    showChoice(timeSystem.isMidtermDay() ? cls::text("notice.midterm_morning") : cls::text("notice.morning_class"),
               timeSystem.isMidtermDay()
                   ? "It is Day 7. The midterm starts from this classroom seat."
                   : "The bell rings at 08:50. Choose how to handle this class.",
               {timeSystem.isMidtermDay() ? "Take the midterm seriously" : "Attend class carefully", "Skip class"},
               [this](int choice) { resolveClassChoice(choice == 0); });
}

void Game::checkClassSchedule(int previousMinute) {
    if (timeSystem.crossedClassTime(previousMinute) || timeSystem.shouldForceClass()) {
        forceMorningClass();
    }
}

void Game::resolveClassChoice(bool attend) {
    timeSystem.markClassResolved();

    if (attend) {
        timeSystem.setTimeAbsolute(TimeSystem::kClassEndMinute);
        std::ostringstream body;
        if (timeSystem.isMidtermDay()) {
            const int roll = (std::rand() % 20) + 1;
            const int academicBonus = (player.getAttributes().academic - 50) / 10;
            const int total = roll + academicBonus;
            const bool passed = total >= 12;
            player.modifyAttributes(passed ? Attributes(-10, -16, 12, 0, 0) : Attributes(-16, -18, 4, 0, 0));
            body << "Midterm finished. Roll " << roll
                 << " + Academic Bonus " << academicBonus
                 << " = " << total << (passed ? " (pass)." : " (struggle).");
        } else {
            player.modifyAttributes(Attributes(-8, -12, 8, 0, 0));
            body << "You focused through the morning lecture. Academic +8, SAN -8, Energy -12.";
        }
        startTimeSkipFlash(cls::text("time.class_passes"));
        showTimedResult(timeSystem.isMidtermDay() ? cls::text("notice.midterm_complete") : cls::text("notice.class_complete"), body.str());
        registerProgressEvent();
    } else {
        timeSystem.setTimeAbsolute(TimeSystem::kRollCallMinute);
        const bool called = (std::rand() % 100) < (timeSystem.isMidtermDay() ? 80 : 45);
        std::ostringstream body;
        if (called) {
            player.modifyAttributes(timeSystem.isMidtermDay()
                ? Attributes(-18, -4, -18, -12, 0)
                : Attributes(-10, -2, -10, -8, 0));
            body << "At 10:20 the teacher calls attendance. You are absent and take a penalty.";
        } else {
            player.modifyAttributes(Attributes(3, -2, -2, 0, 0));
            body << "At 10:20 there is no roll call. You avoid the immediate penalty, but lose study momentum.";
        }
        startTimeSkipFlash(cls::text("time.skip_class"));
        showTimedResult(cls::text("notice.roll_call"), body.str());
    }
}

void Game::resolveMealChoice(int mealIndex) {
    if (mealIndex < 0 || mealIndex >= static_cast<int>(mealOptions.size())) return;
    if (!timeSystem.isMealTime()) {
        showNotice(cls::text("notice.meal_closed"), "Food is available from 12:00-14:00 and 17:00-19:00.");
        return;
    }
    if (heldMealIndex >= 0) {
        showNotice(cls::text("notice.meal_holding"), "You already have a tray. Sit at a table and eat it before taking another meal.");
        return;
    }

    const int slot = timeSystem.mealSlotId();
    if (slot >= 0 && lastMealPickupSlot == slot) {
        showNotice(cls::text("notice.already_served"), "You can only take food once during the current meal period.");
        return;
    }

    const MealOption& meal = mealOptions[mealIndex];
    if (player.getAttributes().gold < meal.cost) {
        showNotice(cls::text("notice.not_enough_gold"), "You do not have enough Gold for " + meal.name + '.');
        return;
    }

    player.modifyAttributes(Attributes(0, 0, 0, 0, -meal.cost));
    heldMealIndex = mealIndex;
    lastMealPickupSlot = slot;
    showNotice(cls::text("notice.food_taken"), meal.name + " is on your tray. Find a cafeteria table and press Enter to eat.");
}

void Game::sleepFromDormitory() {
    if (!timeSystem.canSleep()) {
        showNotice(cls::text("notice.too_early"), "You can choose sleep after 22:30. Until then, the bed is only a short rest spot.");
        return;
    }

    const int sleptMinutes = timeSystem.sleepToNextDay();
    const int sleptHours = sleptMinutes / 60;
    const int sanGain = std::min(45, sleptHours * 5);
    const int energyGain = std::min(70, sleptHours * 8);
    player.modifyAttributes(Attributes(sanGain, energyGain, 0, 0, 0));
    setCurrentPlace(CampusPlace::Dormitory, {480.0f, 276.0f});
    heldMealIndex = -1;
    gamePlayDay = timeSystem.getDay();
    gamesPlayedToday = 0;

    std::ostringstream body;
    if (timeSystem.isFinished()) {
        body << "The 14-day project period is complete. Sleep recovered SAN +"
             << sanGain << " and Energy +" << energyGain << '.';
    } else {
        body << "You slept " << sleptHours << " hours and woke at 08:00. SAN +"
             << sanGain << ", Energy +" << energyGain << '.';
    }
    startTimeSkipFlash(cls::text("time.sleeping"));
    showTimedResult(timeSystem.isFinished() ? cls::text("notice.days_complete") : cls::text("notice.new_day"), body.str());
}

void Game::showTimedResult(const std::string& title, const std::string& body) {
    std::ostringstream message;
    message << body << "\n" << cls::text("time.current") << ": " << timeSystem.clockText();
    showNotice(title, message.str());
}

void Game::renderTimePanel() {
    sf::RectangleShape panel({270.0f, 40.0f});
    panel.setPosition({600.0f, 4.0f});
    panel.setFillColor(sf::Color(10, 18, 26, 230));
    panel.setOutlineColor(timeSystem.isMidtermDay() ? sf::Color(255, 190, 90) : sf::Color(80, 96, 118));
    panel.setOutlineThickness(1.0f);
    window.draw(panel);

    sf::Text clock = cls::makeText(font, timeSystem.clockText(), 12);
    clock.setFillColor(sf::Color(235, 238, 220));
    clock.setPosition({612.0f, 8.0f});
    window.draw(clock);

    const std::string statusLine = cls::format("time.day_phase", {
        {"dayLabel", timeSystem.dayLabel()},
        {"status", statusMessage}
    });
    sf::Text label = cls::makeText(font, statusLine, 10);
    label.setFillColor(timeSystem.isMidtermDay() ? sf::Color(255, 210, 120) : sf::Color(155, 180, 205));
    label.setPosition({612.0f, 24.0f});
    window.draw(label);
}

void Game::renderSceneTransition() {
    sceneBackground.render(window, sceneTransition.background, sf::Color(0, 0, 0, 70));

    sf::RectangleShape plate({640.0f, 132.0f});
    plate.setPosition({160.0f, 332.0f});
    plate.setFillColor(sf::Color(9, 22, 28, 176));
    plate.setOutlineColor(sf::Color(230, 212, 148, 170));
    plate.setOutlineThickness(2.0f);
    window.draw(plate);

    sf::Text title = cls::makeText(font, sceneTransition.title, 34);
    title.setFillColor(sf::Color(250, 240, 205));
    title.setOutlineColor(sf::Color(18, 42, 45));
    title.setOutlineThickness(2.0f);
    title.setPosition({190.0f, 352.0f});
    window.draw(title);

    sf::Text subtitle = cls::makeText(font, sceneTransition.subtitle, 18);
    subtitle.setFillColor(sf::Color(224, 238, 220));
    subtitle.setPosition({190.0f, 407.0f});
    window.draw(subtitle);

    const bool ready = sceneTransition.canContinue();
    sf::Text hint = cls::makeText(font, ready ? cls::text("ui.enter_to_enter") : cls::text("ui.entering"), 13);
    hint.setFillColor(ready ? sf::Color(210, 210, 190, 180) : sf::Color(210, 210, 190, 110));
    hint.setPosition({190.0f, 444.0f});
    window.draw(hint);
}

void Game::renderTimeSkipFlash() {
    sf::RectangleShape blackout({kRenderWidth, kRenderHeight});
    blackout.setFillColor(sf::Color(0, 0, 0, 245));
    window.draw(blackout);

    sf::Text text = cls::makeText(font, timeSkipFlash.text, 20);
    text.setFillColor(sf::Color(230, 230, 220));
    text.setPosition({360.0f, 252.0f});
    window.draw(text);
}

void Game::renderDialogOverlay() {
    sf::RectangleShape shade({kRenderWidth, kRenderHeight});
    shade.setFillColor(sf::Color(0, 0, 0, 105));
    window.draw(shade);

    sf::RectangleShape box({620.0f, 196.0f});
    box.setPosition({190.0f, 166.0f});
    box.setFillColor(sf::Color(14, 24, 31, 235));
    box.setOutlineColor(sf::Color(230, 210, 148, 180));
    box.setOutlineThickness(2.0f);
    window.draw(box);

    sf::Text heading = cls::makeText(font, dialogState.title, 22);
    heading.setFillColor(sf::Color(250, 238, 200));
    heading.setPosition({218.0f, 188.0f});
    window.draw(heading);

    std::string body = dialogState.body;
    if (!dialogState.options.empty()) {
        body += "\n\n" + formatChoiceLines(dialogState.options);
    }
    sf::Text message = cls::makeText(font, body, 15);
    message.setFillColor(sf::Color(218, 230, 220));
    message.setPosition({218.0f, 228.0f});
    window.draw(message);

    const std::string footer = dialogState.mode == DialogMode::Choice
        ? cls::format("ui.press_number_range", {{"count", std::to_string(dialogState.options.size())}})
        : cls::text("ui.press_enter_continue");
    sf::Text hint = cls::makeText(font, footer, 12);
    hint.setFillColor(sf::Color(172, 184, 178));
    hint.setPosition({218.0f, 334.0f});
    window.draw(hint);
}

void Game::renderCombatOverlay() {
    sf::RectangleShape overlay({400.0f, 130.0f});
    overlay.setPosition({280.0f, 200.0f});
    overlay.setFillColor(sf::Color(20, 20, 40, 230));
    overlay.setOutlineColor(combatResult.victory ? sf::Color(100, 200, 100) : sf::Color(200, 100, 100));
    overlay.setOutlineThickness(2.0f);
    window.draw(overlay);

    std::ostringstream css;
    css << (combatResult.victory ? cls::text("combat.victory") : cls::text("combat.defeat"))
        << " " << cls::text("combat.vs") << " " << combatResult.enemyName << "\n\n"
        << "D20: " << combatResult.d20Roll
        << " + MOD: " << (combatResult.modifier >= 0 ? "+" : "") << combatResult.modifier
        << " = " << combatResult.total
        << " vs DC " << combatResult.dc << "\n\n"
        << (combatResult.victory ? cls::text("combat.result.victory") : cls::text("combat.result.defeat"));
    sf::Text resultText = cls::makeText(font, css.str(), 14);
    resultText.setFillColor(combatResult.victory ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
    resultText.setPosition({300.0f, 215.0f});
    window.draw(resultText);
}

