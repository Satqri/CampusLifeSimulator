#ifndef CLS_CORE_GAMESESSION_H
#define CLS_CORE_GAMESESSION_H

#include <SFML/Graphics.hpp>
#include "config/LibraryConfig.h"
#include "config/MealConfig.h"
#include "core/ActivityRunner.h"
#include "core/GameContext.h"
#include "core/SettlementResolver.h"
#include "core/SleepSystem.h"
#include "core/Types.h"
#include "entity/Enemy.h"
#include "entity/Player.h"
#include "event/EventRunner.h"
#include "map/BuildingInterior.h"
#include "map/CafeteriaInterior.h"
#include "map/CampusMap.h"
#include "map/ClassroomInterior.h"
#include "map/ConvenienceStoreInterior.h"
#include "map/DormitoryInterior.h"
#include "map/GymInterior.h"
#include "map/LibraryInterior.h"
#include "ui/ActivityNotice.h"
#include "ui/ChoicePrompt.h"
#include "ui/DifficultyPanel.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

constexpr char kDormitoryDeskPromptPurpose[] = "dormitory_desk_choice";

class GameSession {
public:
    explicit GameSession(sf::Font& font);

    void resetForNewGame(Difficulty difficulty);

    bool maybeFinalizeRun();
    void showTimedResult(const std::string& title, const std::string& body);
    bool checkEventTriggers(int previousMinute);

    bool canEnterPlace(CampusPlace target);
    void startMapTransition(const MapPortal& portal, SceneTransition& sceneTransition);
    void finishSceneTransition(SceneTransition& sceneTransition);

    bool canTriggerInteraction(const InteractionPoint& ip);
    void handleInteraction(const InteractionPoint& ip);
    void resolveMealChoice(int mealIndex);
    void resolveDormitoryDeskChoice(int choiceIndex);

    bool fightNearestEnemy();
    void trySpawnEnemy();

    BuildingInterior* mapForPlace(CampusPlace place) const;
    std::string buildSettlementBody(const SettlementResult& result, int page) const;

    GameContext& context() { return mContext; }
    const GameContext& context() const { return mContext; }

    Player& player() { return mPlayer; }
    const Player& player() const { return mPlayer; }

    TimeSystem& timeSystem() { return mTimeSystem; }
    const TimeSystem& timeSystem() const { return mTimeSystem; }

    EventRunner& eventRunner() { return mEventRunner; }
    const EventRunner& eventRunner() const { return mEventRunner; }

    ActivityNotice& activityNotice() { return mActivityNotice; }
    const ActivityNotice& activityNotice() const { return mActivityNotice; }

    ChoicePrompt& mealChoicePrompt() { return mMealChoicePrompt; }
    const ChoicePrompt& mealChoicePrompt() const { return mMealChoicePrompt; }

    TimeSkipFlash& timeSkipFlash() { return mTimeSkipFlash; }
    const TimeSkipFlash& timeSkipFlash() const { return mTimeSkipFlash; }

    CombatResult& combatResult() { return mCombatResult; }
    const CombatResult& combatResult() const { return mCombatResult; }

    PendingTimedActivity& pendingTimedActivity() { return mPendingTimedActivity; }
    PendingSleep& pendingSleep() { return mPendingSleep; }

    BuildingInterior*& currentMap() { return mCurrentMap; }
    CampusPlace& currentPlace() { return mCurrentPlace; }

    std::vector<std::unique_ptr<Enemy>>& activeEnemies() { return mActiveEnemies; }

    bool& settlementActive() { return mSettlementActive; }
    int& settlementPage() { return mSettlementPage; }
    SettlementResult& settlementResult() { return mSettlementResult; }

private:
    void injectContextCallbacks();
    void loadEventConfigs();
    void configureMaps();

    sf::Font& mFont;

    std::unique_ptr<CampusMap> mCampusMap;
    std::unique_ptr<DormitoryInterior> mDormitoryMap;
    std::unique_ptr<GymInterior> mGymMap;
    std::unique_ptr<LibraryInterior> mLibraryMap;
    std::unique_ptr<ClassroomInterior> mClassroomMap;
    std::unique_ptr<CafeteriaInterior> mCafeteriaMap;
    std::unique_ptr<ConvenienceStoreInterior> mStoreMap;

    Player mPlayer;
    CampusPlace mCurrentPlace = CampusPlace::Campus;
    BuildingInterior* mCurrentMap = nullptr;
    CampusPlace mPendingPlace = CampusPlace::Campus;
    sf::Vector2f mPendingSpawnPosition = {480.0f, 276.0f};
    bool mHasPendingMapTransition = false;

    TimeSystem mTimeSystem;
    ActivityNotice mActivityNotice;
    ChoicePrompt mMealChoicePrompt;
    EventRunner mEventRunner;
    SettlementResolver mSettlementResolver;
    TimeSkipFlash mTimeSkipFlash;
    CombatResult mCombatResult;

    PendingTimedActivity mPendingTimedActivity;
    PendingSleep mPendingSleep;

    std::vector<std::unique_ptr<Enemy>> mActiveEnemies;
    std::vector<MealOption> mMealOptions;
    std::vector<LibraryBook> mLibraryBooks;
    std::array<int, 4> mLibraryBookProgress = {0, 0, 0, 0};
    int mSelectedLibraryBook = 0;
    int mLastMealPickupSlot = -1;
    int mGamePlayDay = 1;
    int mGamesPlayedToday = 0;
    int mSpawnCounter = 0;

    bool mSettlementActive = false;
    int mSettlementPage = 0;
    SettlementResult mSettlementResult;

    GameContext mContext;
};

#endif
