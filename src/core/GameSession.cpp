#include "core/GameSession.h"

#include "core/CharacterState.h"
#include "core/CombatSystem.h"
#include "core/Localization.h"
#include "core/SleepSystem.h"
#include "map/MapPortal.h"
#include "interaction/CafeteriaInteraction.h"
#include "interaction/DormitoryInteraction.h"
#include "interaction/RegularInteraction.h"
#include "utils/AssetPath.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace {

void applyDifficulty(Player& player, Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:
            player.modifyAttributes(Attributes{.energy = 15, .health = 10, .san = 20, .academic = 5, .social = 5});
            break;
        case Difficulty::Normal:
            break;
        case Difficulty::Hard:
            player.modifyAttributes(Attributes{.energy = -10, .health = -5, .san = -20, .social = -5});
            break;
    }
}

} // namespace

GameSession::GameSession(sf::Font& font)
    : mFont(font)
    , mCampusMap(std::make_unique<CampusMap>())
    , mDormitoryMap(std::make_unique<DormitoryInterior>())
    , mGymMap(std::make_unique<GymInterior>())
    , mLibraryMap(std::make_unique<LibraryInterior>())
    , mClassroomMap(std::make_unique<ClassroomInterior>())
    , mCafeteriaMap(std::make_unique<CafeteriaInterior>())
    , mStoreMap(std::make_unique<ConvenienceStoreInterior>())
    , mPlayer(480.0f, 280.0f)
    , mMealOptions(loadMealConfig(cls::resolveAssetPath("assets/config/meals.json")))
    , mLibraryBooks(loadLibraryConfig(cls::resolveAssetPath("assets/config/library.json")))
    , mContext{
        mPlayer, mCurrentPlace, mCurrentMap,
        mCampusMap.get(), mDormitoryMap.get(), mGymMap.get(),
        mLibraryMap.get(), mClassroomMap.get(), mCafeteriaMap.get(), mStoreMap.get(),
        mTimeSystem, mCombatResult, mTimeSkipFlash,
        mActivityNotice, mMealChoicePrompt,
        mPendingPlace, mPendingSpawnPosition, mHasPendingMapTransition,
        mActiveEnemies, mMealOptions, mLibraryBooks, mLibraryBookProgress,
        mSelectedLibraryBook, mLastMealPickupSlot,
        mGamePlayDay, mGamesPlayedToday, mSpawnCounter,
        {}, {}, {}, {}, {}, {}
    } {
    mPlayer.setName("Protagonist");
    initializeHiddenState(mPlayer.getHidden());
    mCurrentMap = mCampusMap.get();

    configureMaps();
    loadEventConfigs();
    mSettlementResolver.load(cls::resolveAssetPath("assets/config/events/endings.json"),
        cls::resolveAssetPath("assets/config/events/titles.json"));
    injectContextCallbacks();
}

void GameSession::configureMaps() {
    mCampusMap->setFont(&mFont);
    mCampusMap->setTimeSystem(&mTimeSystem);
    mDormitoryMap->setFont(&mFont);
    mGymMap->setFont(&mFont);
    mLibraryMap->setFont(&mFont);
    mClassroomMap->setFont(&mFont);
    mCafeteriaMap->setFont(&mFont);
    mStoreMap->setFont(&mFont);
}

void GameSession::loadEventConfigs() {
    for (const auto& entry : std::filesystem::directory_iterator(
             cls::resolveAssetPath("assets/config/events"))) {
        if (entry.path().extension() == ".json") {
            mEventRunner.loadEvents(entry.path().string());
        }
    }
}

void GameSession::injectContextCallbacks() {
    mContext.runTimedActivity = [this](int minutes, const Attributes& delta,
            const std::string& title, const std::string& body,
            const std::string& activityId, bool customDuration) {
        runTimedActivity(mContext, mPendingTimedActivity, minutes, delta, title, body, activityId, customDuration);
    };

    mContext.runTimedActivityWithHidden = [this](int minutes, const Attributes& delta,
            const HiddenMap& hiddenDelta,
            const std::string& title, const std::string& body,
            const std::string& activityId, bool customDuration) {
        runTimedActivityWithHidden(mContext, mPendingTimedActivity, minutes, delta, hiddenDelta, title, body, activityId, customDuration);
    };

    mContext.showTimedResult = [this](const std::string& title, const std::string& body) {
        showTimedResult(title, body);
    };
    mContext.checkEventTriggers = [this](int previousMinute) {
        return checkEventTriggers(previousMinute);
    };
    mContext.sleepFromDormitory = [this]() {
        sleepFromDormitory(mContext, mPendingSleep);
    };
    mContext.finalizeStateChange = [this]() {
        return maybeFinalizeRun();
    };
}

void GameSession::resetForNewGame(Difficulty difficulty) {
    mPlayer.setAttributes(defaultPlayerAttributes());
    mCurrentPlace = CampusPlace::Campus;
    mCurrentMap = mCampusMap.get();
    mPlayer.setPosition(480.0f, 276.0f);
    mPlayer.stopMovement();
    mTimeSystem = TimeSystem();
    initializeHiddenState(mPlayer.getHidden());
    applyDifficulty(mPlayer, difficulty);
    syncVisibleHealthFromHidden(mPlayer.getAttributes(), mPlayer.getHidden());
    mActivityNotice.clear();
    mMealChoicePrompt.clear();
    mPendingTimedActivity.clear();
    mPendingSleep.clear();
    mLastMealPickupSlot = -1;
    mGamePlayDay = mTimeSystem.getDay();
    mGamesPlayedToday = 0;
    mSpawnCounter = 0;
    mActiveEnemies.clear();
    mCombatResult.clear();
    mTimeSkipFlash.active = false;
    mSettlementActive = false;
    mSettlementPage = 0;
    mSettlementResult = SettlementResult{};
    mPendingPlace = CampusPlace::Campus;
    mPendingSpawnPosition = {480.0f, 276.0f};
    mHasPendingMapTransition = false;
}

void GameSession::showTimedResult(const std::string& title, const std::string& body) {
    std::ostringstream message;
    message << body << "\n" << cls::text("time.current") << ": " << mTimeSystem.clockText();
    mActivityNotice.show(title, message.str());
}

bool GameSession::checkEventTriggers(int previousMinute) {
    return mEventRunner.checkTriggers(mContext, previousMinute);
}

std::string GameSession::buildSettlementBody(const SettlementResult& result, int page) const {
    std::ostringstream body;
    if (page == 0) {
        body << (result.ending.nameKey.empty() ? result.ending.name : cls::text(result.ending.nameKey));
        const std::string tagline = result.ending.taglineKey.empty() ? result.ending.tagline : cls::text(result.ending.taglineKey);
        if (!tagline.empty()) body << "\n" << tagline;
        body << "\n\n" << (result.ending.descriptionKey.empty() ? result.ending.description : cls::text(result.ending.descriptionKey));
        body << "\n\n" << cls::text("quest.return_title");
        return body.str();
    }
    if (page == 1) {
        if (result.titles.empty()) {
            body << cls::text("settlement.no_extra_titles");
        } else {
            for (const auto& title : result.titles) {
                const std::string titleName = title.nameKey.empty() ? title.name : cls::text(title.nameKey);
                const std::string subtitle = title.subtitleKey.empty() ? title.subtitle : cls::text(title.subtitleKey);
                body << "- " << titleName;
                if (!subtitle.empty()) body << "（" << subtitle << "）";
                body << "\n";
            }
        }
        body << "\n" << cls::text("quest.return_title");
        return body.str();
    }
    body << result.summary << "\n\n" << cls::text("quest.return_title");
    return body.str();
}

bool GameSession::maybeFinalizeRun() {
    if (mSettlementActive) return true;
    normalizeHidden(mPlayer.getHidden());
    mPlayer.syncDailyCountersFromHidden();
    syncVisibleHealthFromHidden(mPlayer.getAttributes(), mPlayer.getHidden());
    mSettlementResult = mSettlementResolver.resolveImmediate(mPlayer);
    if (mSettlementResult.resolved) {
        mSettlementActive = true;
        mSettlementPage = 0;
        mActivityNotice.show(cls::text("quest.final_result"), buildSettlementBody(mSettlementResult, mSettlementPage));
        return true;
    }
    if (mTimeSystem.isFinished()) {
        mSettlementResult = mSettlementResolver.resolveFinal(mPlayer);
        mSettlementActive = true;
        mSettlementPage = 0;
        mActivityNotice.show(cls::text("quest.final_result"), buildSettlementBody(mSettlementResult, mSettlementPage));
        return true;
    }
    return false;
}

BuildingInterior* GameSession::mapForPlace(CampusPlace place) const {
    switch (place) {
        case CampusPlace::Campus:    return mCampusMap.get();
        case CampusPlace::Dormitory: return mDormitoryMap.get();
        case CampusPlace::Gym:       return mGymMap.get();
        case CampusPlace::Library:   return mLibraryMap.get();
        case CampusPlace::Classroom: return mClassroomMap.get();
        case CampusPlace::Cafeteria: return mCafeteriaMap.get();
        case CampusPlace::Store:     return mStoreMap.get();
    }
    return mCampusMap.get();
}

bool GameSession::canEnterPlace(CampusPlace target) {
    const int minute = normalizedMinute(mTimeSystem.getMinuteOfDay());
    std::string body;
    switch (target) {
        case CampusPlace::Gym:
            if (!isWithinClockWindow(minute, 9 * 60, 22 * 60)) {
                body = cls::text("place.closed.gym");
            }
            break;
        case CampusPlace::Library:
            if (!isWithinClockWindow(minute, 7 * 60, 23 * 60)) {
                body = cls::text("place.closed.library");
            }
            break;
        case CampusPlace::Cafeteria:
            if (!isWithinClockWindow(minute, TimeSystem::kBreakfastStartMinute, 22 * 60)) {
                body = cls::text("place.closed.cafeteria");
            }
            break;
        default:
            break;
    }
    if (body.empty()) return true;
    mActivityNotice.show(cls::text("place.closed.title"), body);
    return false;
}

void GameSession::startMapTransition(const MapPortal& portal, SceneTransition& sceneTransition) {
    if (!canEnterPlace(portal.target)) return;

    auto& hidden = mPlayer.getHidden();
    normalizeHidden(hidden);
    CampusPlace commuteFrom = mCurrentPlace;
    if (mCurrentPlace == CampusPlace::Campus && hidden.contains("lastIndoorPlace")
        && hidden["lastIndoorPlace"].is_string()) {
        const std::string last = hidden["lastIndoorPlace"].get<std::string>();
        if (last == "dormitory") commuteFrom = CampusPlace::Dormitory;
        else if (last == "cafeteria") commuteFrom = CampusPlace::Cafeteria;
        else if (last == "classroom") commuteFrom = CampusPlace::Classroom;
        else if (last == "library") commuteFrom = CampusPlace::Library;
        else if (last == "gym") commuteFrom = CampusPlace::Gym;
        else if (last == "store") commuteFrom = CampusPlace::Store;
    }

    const bool enteringIndoor = mCurrentPlace == CampusPlace::Campus && portal.target != CampusPlace::Campus;
    int travelMinutes = enteringIndoor ? commuteMinutes(commuteFrom, portal.target) : 0;
    if (enteringIndoor && portal.target == CampusPlace::Store) {
        travelMinutes = std::max(travelMinutes, 30);
    }
    if (travelMinutes > 0) {
        const int startDay = mTimeSystem.getDay();
        const int prev = mTimeSystem.advanceMinutes(travelMinutes);
        mTimeSkipFlash.start(cls::format("time.commuting", {{"minutes", std::to_string(travelMinutes)}}));
        if (mTimeSystem.getDay() != startDay) {
            markNoSleepForSkippedDays(mContext);
            mPlayer.dailyAttributeCheck();
            mGamePlayDay = mTimeSystem.getDay();
            mGamesPlayedToday = 0;
            if (maybeFinalizeRun()) return;
        }
        if (checkEventTriggers(prev)) {
            maybeFinalizeRun();
            return;
        }
        if (maybeFinalizeRun()) return;
    }

    if (enteringIndoor && portal.target == CampusPlace::Dormitory
        && isWithinClockWindow(normalizedMinute(mTimeSystem.getMinuteOfDay()), 0, 6 * 60)) {
        auto& afterTravelHidden = mPlayer.getHidden();
        const int seed = mTimeSystem.getDay() * 101 + normalizedMinute(mTimeSystem.getMinuteOfDay()) * 17
            + afterTravelHidden.value("lateNightLevel", 0) * 7;
        if (seed % 100 < 35) {
            mPlayer.modifyAttributes(Attributes{.san = -6});
            mergeHidden(afterTravelHidden, HiddenMap{{"lateNightLevel", 2}});
            syncVisibleHealthFromHidden(mPlayer.getAttributes(), afterTravelHidden);
            mActivityNotice.show(cls::text("notice.late_return.title"),
                cls::text("notice.late_return.body"));
            if (maybeFinalizeRun()) return;
        }
    }

    if (portal.target == CampusPlace::Campus && mCurrentPlace != CampusPlace::Campus) {
        hidden["lastIndoorPlace"] = campusPlaceKey(mCurrentPlace);
    } else if (portal.target != CampusPlace::Campus) {
        hidden["lastIndoorPlace"] = campusPlaceKey(portal.target);
    }

    mPendingPlace = portal.target;
    mPendingSpawnPosition = portal.spawnPosition;
    mHasPendingMapTransition = true;
    sceneTransition.start(portal.transitionBackground, portal.title, portal.subtitle);
}

void GameSession::finishSceneTransition(SceneTransition& sceneTransition) {
    if (mHasPendingMapTransition) {
        mCurrentPlace = mPendingPlace;
        mCurrentMap = mapForPlace(mPendingPlace);
        mPlayer.setPosition(mPendingSpawnPosition.x, mPendingSpawnPosition.y);
        mPlayer.stopMovement();
        mHasPendingMapTransition = false;
    }
    sceneTransition.skip();
}

bool GameSession::canTriggerInteraction(const InteractionPoint& ip) {
    const std::string& actionId = ip.actionId;
    const int minute = normalizedMinute(mTimeSystem.getMinuteOfDay());
    std::string body;

    if (actionId.rfind("classroom_", 0) == 0
        && isWithinClockWindow(minute, 23 * 60, 7 * 60)) {
        body = cls::text("interaction.closed.classroom_night");
    } else if ((actionId.rfind("gym_treadmill_", 0) == 0
            || actionId.rfind("gym_barbell_", 0) == 0
            || actionId == "gym_front_desk")
        && !isWithinClockWindow(minute, 9 * 60, 22 * 60)) {
        body = cls::text("interaction.closed.gym");
    } else if ((actionId.rfind("library_shelf_", 0) == 0
            || actionId == "library_table")
        && !isWithinClockWindow(minute, 7 * 60, 23 * 60)) {
        body = cls::text("interaction.closed.library");
    } else if ((actionId == "cafeteria_counter"
            || actionId.rfind("cafeteria_table_", 0) == 0)
        && !mTimeSystem.isMealTime()) {
        body = cls::text("activity.cafeteria.meal_time_closed");
    }

    if (body.empty()) return true;
    mActivityNotice.show(cls::text("interaction.closed.title"), body);
    return false;
}

void GameSession::handleInteraction(const InteractionPoint& ip) {
    if (!canTriggerInteraction(ip)) return;
    if (ip.actionId == "dormitory_desk") {
        mMealChoicePrompt.show(
            cls::text("prompt.dormitory_desk.title"),
            cls::text("prompt.dormitory_desk.body"),
            std::vector<std::string>{
                cls::text("prompt.dormitory_desk.study"),
                cls::text("prompt.dormitory_desk.games")
            },
            kDormitoryDeskPromptPurpose,
            std::vector<int>{0, 1});
        return;
    }
    if (mEventRunner.triggerByAction(ip.actionId, mContext)) return;
    if (CafeteriaInteraction::handleInteraction(mContext, ip.actionId, ip.displayLabel())) return;
    if (DormitoryInteraction::handle(mContext, ip)) return;
    if (RegularInteraction::handle(mContext, ip)) return;
    mActivityNotice.show(ip.displayLabel(), ip.displayDescription());
}

void GameSession::resolveMealChoice(int mealIndex) {
    CafeteriaInteraction::resolveMealChoice(mContext, mealIndex);
}

void GameSession::resolveDormitoryDeskChoice(int choiceIndex) {
    mMealChoicePrompt.clear();
    if (choiceIndex == 0) {
        RegularInteraction::handleDormitoryDeskStudy(mContext, "dormitory_desk");
        return;
    }
    if (choiceIndex == 1) {
        if (mEventRunner.triggerByAction("dormitory_games", mContext)) return;
        RegularInteraction::handleDormitoryGames(mContext, "dormitory_games");
    }
}

bool GameSession::fightNearestEnemy() {
    return CombatSystem::fightNearestEnemy(mContext);
}

void GameSession::trySpawnEnemy() {
    CombatSystem::trySpawnEnemy(mContext);
}
