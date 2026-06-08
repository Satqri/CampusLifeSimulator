#ifndef CLS_GAME_GAME_H
#define CLS_GAME_GAME_H

#include "core/GameSettings.h"
#include "core/TimeSystem.h"
#include "core/Types.h"
#include "entity/Enemy.h"
#include "entity/Player.h"
#include "event/RandomEvent.h"
#include "fileio/ConfigManager.h"
#include "fileio/SaveManager.h"
#include "map/BuildingInterior.h"
#include "map/CafeteriaInterior.h"
#include "map/CampusMap.h"
#include "map/ClassroomInterior.h"
#include "map/DormitoryInterior.h"
#include "map/GymInterior.h"
#include "map/LibraryInterior.h"
#include "quest/MainQuest.h"
#include "quest/QuestManager.h"
#include "state/CombatState.h"
#include "state/EventDialogState.h"
#include "state/ExplorationState.h"
#include "state/GameState.h"
#include "state/MainQuestState.h"
#include "state/MiniGameState.h"
#include "ui/DifficultyPanel.h"
#include "ui/HelpPanel.h"
#include "ui/HUD.h"
#include "ui/SceneBackground.h"
#include "ui/SettingsPanel.h"
#include "ui/TitleScreen.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @class Game
 * @brief 游戏主控制器，拥有共享系统并驱动顶层循环
 *
 * 负责窗口、字体、玩家、任务、时间、地图、UI 覆盖层与状态机。
 */
class Game {
public:
    /**
     * @brief 构造游戏对象
     */
    Game();

    /**
     * @brief 析构游戏对象
     */
    ~Game();

    /**
     * @brief 运行游戏主循环
     * @return 进程退出码
     */
    int run();

    /** @brief 获取渲染窗口 */
    sf::RenderWindow& getWindow();

    /** @brief 获取共享设置 */
    cls::GameSettings& getSettings();

    /** @brief 获取设置面板 */
    SettingsPanel& getSettingsPanel();

    /** @brief 应用并保存当前设置 */
    void applyAndSaveSettings();

    /** @brief 打开设置界面 */
    void openSettingsScreen();

    /** @brief 关闭设置界面 */
    void closeSettingsScreen();

    /** @brief 获取当前是否位于设置界面 */
    bool isSettingsScreen() const;

    /** @brief 设置鼠标移动目标 */
    void setMoveTarget(sf::Vector2f target);

    /** @brief 清除鼠标移动目标 */
    void clearMoveTarget();

    /** @brief 获取是否存在鼠标移动目标 */
    bool hasMoveDestination() const;

    /** @brief 获取鼠标移动目标 */
    sf::Vector2f getMoveTarget() const;

    /** @brief 显示退出确认 */
    void requestExitConfirmation();

    /** @brief 当前是否处于探索状态 */
    bool isExplorationState() const;

    /** @brief 获取共享字体 */
    sf::Font& getFont();

    /** @brief 获取玩家对象 */
    Player& getPlayer();

    /** @brief 获取时间系统 */
    TimeSystem& getTimeSystem();

    /** @brief 获取任务管理器 */
    QuestManager& getQuestManager();

    /** @brief 获取当前地图 */
    BuildingInterior* getCurrentMap();

    /** @brief 获取当前地点 */
    CampusPlace getCurrentPlace() const;

    /** @brief 获取活跃敌人列表 */
    const std::vector<std::unique_ptr<Enemy>>& getActiveEnemies() const;

    /** @brief 获取当前战斗敌人 */
    Enemy* getCurrentCombatEnemy();

    /** @brief 获取当前难度 */
    Difficulty getDifficulty() const;

    /** @brief 获取当前场景是否处于玩法界面 */
    bool isGameplayScreen() const;

    /** @brief 获取帮助覆盖层是否打开 */
    bool isHelpOpen() const;

    /** @brief 切换帮助覆盖层 */
    void toggleHelpOverlay();

    /** @brief 关闭帮助覆盖层 */
    void closeHelpOverlay();

    /** @brief 请求切换到指定游戏状态 */
    void requestStateChange(StateType nextState);

    /** @brief 立即进入新状态 */
    void changeState(StateType nextState);

    /** @brief 创建新游戏并进入探索 */
    void resetNewGame(Difficulty difficulty);

    /** @brief 保存当前进度 */
    bool saveGame();

    /** @brief 读取已有进度 */
    bool loadGame();

    /** @brief 显示普通提示框 */
    void showNotice(const std::string& title, const std::string& body,
                    std::function<void()> onClose = {});

    /** @brief 显示选择框 */
    void showChoice(const std::string& title, const std::string& body,
                    const std::vector<std::string>& options,
                    std::function<void(int)> onChoose);

    /** @brief 清空当前事件对话 */
    void clearDialog();

    /** @brief 确认当前对话 */
    void confirmDialog(int choiceIndex);

    /** @brief 当前是否存在活动对话 */
    bool hasActiveDialog() const;

    /** @brief 获取当前对话标题 */
    const std::string& getDialogTitle() const;

    /** @brief 获取当前对话正文 */
    const std::string& getDialogBody() const;

    /** @brief 获取当前对话选项 */
    const std::vector<std::string>& getDialogOptions() const;

    /** @brief 获取当前对话是否为选择型 */
    bool isChoiceDialog() const;

    /** @brief 启动时间跳过闪屏 */
    void startTimeSkipFlash(const std::string& text);

    /** @brief 获取时间跳过闪屏是否活动 */
    bool isTimeSkipFlashActive() const;

    /** @brief 获取场景切换是否活动 */
    bool isSceneTransitionActive() const;

    /** @brief 完成当前场景切换 */
    void finishSceneTransition();

    /** @brief 按地点切换当前地图 */
    void setCurrentPlace(CampusPlace place, sf::Vector2f spawnPosition, bool stopMovement = true);

    /** @brief 启动地图传送门切换 */
    void startMapTransition(const MapPortal& portal);

    /** @brief 处理地图交互 */
    void handleInteraction(const InteractionPoint& point);

    /** @brief 处理定时活动 */
    void runTimedActivity(int minutes, const Attributes& delta,
                          const std::string& title, const std::string& body,
                          bool countAsProgressEvent = true);

    /** @brief 根据 SAN 尝试生成敌人 */
    void trySpawnEnemy();

    /** @brief 开始与最近敌人的战斗 */
    bool startCombatWithNearestEnemy();

    /** @brief 完成当前战斗 */
    void finalizeCombat(bool victory, int d20Roll, int modifier, int total, int dc);

    /** @brief 返回顶部玩法状态名称 */
    std::string gameplayLabel() const;

    /** @brief 渲染公共覆盖层 */
    void renderGlobalOverlays();

    /** @brief 渲染帮助覆盖层 */
    void renderHelpOverlay();

    /** @brief 获取最近一次保存/读取提示 */
    const std::string& getStatusMessage() const;

    /** @brief 设置状态提示文本 */
    void setStatusMessage(const std::string& message);

private:
    enum class Screen {
        Title,
        Difficulty,
        Settings,
        Gameplay,
        Help
    };

    enum class DialogMode {
        None,
        Notice,
        Choice
    };

    /**
     * @struct SceneTransition
     * @brief 地图切换过场数据
     */
    struct SceneTransition {
        bool active = false;
        float timer = 0.0f;
        SceneBackgroundType background = SceneBackgroundType::Dormitory;
        std::string title;
        std::string subtitle;

        void start(SceneBackgroundType bg, const std::string& heading, const std::string& line);
        void update(float deltaTime);
        bool canContinue() const;
        void clear();
    };

    /**
     * @struct TimeSkipFlash
     * @brief 时间流逝黑场提示
     */
    struct TimeSkipFlash {
        bool active = false;
        float timer = 0.0f;
        std::string text;

        void start(const std::string& message);
        void update(float deltaTime);
        void clear();
    };

    /**
     * @struct CombatResult
     * @brief 战斗结算展示数据
     */
    struct CombatResult {
        bool active = false;
        bool victory = false;
        std::string enemyName;
        int d20Roll = 0;
        int modifier = 0;
        int total = 0;
        int dc = 0;

        void show(bool win, const std::string& name, int d20, int mod, int finalTotal, int difficultyClass);
        void clear();
    };

    /**
     * @struct DialogState
     * @brief 事件对话共享数据
     */
    struct DialogState {
        DialogMode mode = DialogMode::None;
        std::string title;
        std::string body;
        std::vector<std::string> options;
        std::function<void()> onClose;
        std::function<void(int)> onChoose;
    };

    /**
     * @struct MealOption
     * @brief 食堂餐食配置
     */
    struct MealOption {
        std::string name;
        int cost = 0;
        Attributes reward;
        std::string description;
    };

    sf::RenderWindow window;
    sf::View gameView;
    sf::Font font;
    bool fontLoaded;
    ConfigManager configManager;
    SaveManager saveManager;
    cls::GameSettings settings;
    std::string statusMessage;
    Screen screen;
    Screen previousScreen;
    bool helpOverlayOpen;
    bool settingsModalOpen;
    Difficulty selectedDifficulty;
    bool difficultyApplied;

    SettingsPanel settingsPanel;
    TitleScreen titleScreen;
    DifficultyPanel difficultyPanel;
    HelpPanel helpPanel;
    HUD hud;
    SceneBackground sceneBackground;

    TimeSystem timeSystem;
    Player player;
    QuestManager questManager;

    std::unique_ptr<CampusMap> campusMap;
    std::unique_ptr<DormitoryInterior> dormitoryMap;
    std::unique_ptr<GymInterior> gymMap;
    std::unique_ptr<LibraryInterior> libraryMap;
    std::unique_ptr<ClassroomInterior> classroomMap;
    std::unique_ptr<CafeteriaInterior> cafeteriaMap;
    BuildingInterior* currentMap;
    CampusPlace currentPlace;
    CampusPlace pendingPlace;
    sf::Vector2f pendingSpawnPosition;
    bool hasPendingMapTransition;

    std::unique_ptr<MainQuestState> mainQuestState;
    std::unique_ptr<ExplorationState> explorationState;
    std::unique_ptr<EventDialogState> eventDialogState;
    std::unique_ptr<CombatState> combatState;
    std::unique_ptr<MiniGameState> miniGameState;
    GameState* currentState;
    std::optional<StateType> currentStateType;
    std::optional<StateType> pendingState;
    StateType dialogReturnState;

    SceneTransition sceneTransition;
    TimeSkipFlash timeSkipFlash;
    CombatResult combatResult;
    DialogState dialogState;

    std::vector<std::unique_ptr<Enemy>> activeEnemies;
    std::optional<std::size_t> activeCombatEnemyIndex;
    RandomEventLibrary randomEventLibrary;
    int randomEventCounter;
    int spawnCounter;
    bool randomEventPending;

    bool hasMoveTarget;
    sf::Vector2f moveTarget;

    int selectedLibraryBook;
    std::array<int, 4> libraryBookProgress;
    std::array<std::string, 4> libraryBookNames;
    std::array<std::string, 4> librarySkillNames;
    std::array<MealOption, 3> mealOptions;
    int heldMealIndex;
    int lastMealPickupSlot;
    int gamePlayDay;
    int gamesPlayedToday;

    void update(float deltaTime);
    void render();
    void handleEvent(const sf::Event& event);
    void handleTitleEvent(const sf::Event& event);
    void handleDifficultyEvent(const sf::Event& event);
    void handleHelpEvent(const sf::Event& event);
    void handleGameplayEvent(const sf::Event& event);

    void loadFont();
    void initializeMaps();
    BuildingInterior* mapForPlace(CampusPlace place) const;

    void updateSharedSystems(float deltaTime);
    void processPendingStateChange();
    void applyDifficulty(Difficulty difficulty);
    void triggerQuestIfNeeded();
    void registerProgressEvent();
    void maybeTriggerRandomEvent(const std::string& source);
    void presentRandomEvent(const RandomEventDefinition& eventDef, const std::string& source);

    void forceMorningClass();
    void checkClassSchedule(int previousMinute);
    void resolveClassChoice(bool attend);
    void resolveMealChoice(int mealIndex);
    void sleepFromDormitory();
    void showTimedResult(const std::string& title, const std::string& body);

    void renderTimePanel();
    void renderSceneTransition();
    void renderTimeSkipFlash();
    void renderDialogOverlay();
    void renderCombatOverlay();
};

#endif // CLS_GAME_GAME_H
