#ifndef CLS_CORE_SLEEPSYSTEM_H
#define CLS_CORE_SLEEPSYSTEM_H

#include "core/CharacterState.h"
#include "core/GameContext.h"
#include "core/Localization.h"
#include "core/TimeSystem.h"
#include <sstream>

constexpr int kDefaultSleepMinutes = 7 * 60 + 30;
constexpr int kMinimumHealthySleepMinutes = 6 * 60 + 30;
constexpr int kOversleepFloorMinutes = 7 * 60;
constexpr char kSleepPromptPurpose[] = "sleep_alarm";

struct PendingSleep {
    bool active = false;
    void clear() { active = false; }
};

/** @brief 跨天未睡觉的惩罚标记 */
inline void markNoSleepForSkippedDays(GameContext& ctx) {
    auto& hidden = ctx.player.getHidden();
    normalizeHidden(hidden);
    const int currentDay = ctx.timeSystem.getDay();
    const int lastSleepDay = hidden.value("lastSleepDay", 0);
    if (currentDay > lastSleepDay + 1) {
        HiddenMap delta = HiddenMap::object();
        delta["consecutiveNoSleepDays"] = hidden.value("consecutiveNoSleepDays", 0)
            + (currentDay - lastSleepDay - 1);
        delta["lastSleepDay"] = currentDay - 1;
        mergeHidden(hidden, delta);
    }
}

/** @brief 执行睡觉逻辑 */
inline void executeSleep(GameContext& ctx, bool& settlementActive,
                         int requestedMinutes, bool explicitAlarm) {
    auto& hidden = ctx.player.getHidden();
    normalizeHidden(hidden);

    int targetMinutes = requestedMinutes > 0
        ? requestedMinutes
        : hidden.value("lastSleepMinutes", kDefaultSleepMinutes);
    targetMinutes = std::clamp(targetMinutes, 60, 12 * 60);

    bool overslept = false;
    if (explicitAlarm && targetMinutes < 6 * 60) {
        const int seed = ctx.timeSystem.getDay() * 97
            + normalizedMinute(ctx.timeSystem.getMinuteOfDay()) * 13
            + hidden.value("consecutiveNoSleepDays", 0) * 31;
        if (seed % 100 < 65) {
            targetMinutes = kOversleepFloorMinutes;
            overslept = true;
        }
    }

    const int startDay = ctx.timeSystem.getDay();
    const int sleptMinutes = ctx.timeSystem.sleepForMinutes(targetMinutes);
    const int sleptHours = sleptMinutes / 60;
    const int sleptRestMinutes = sleptMinutes % 60;

    int sanGain = std::min(45, sleptMinutes / 12);
    int energyGain = std::min(70, sleptMinutes / 8);
    int healthDelta = 0;
    int lateNightRelief = -5;
    if (sleptMinutes < 6 * 60) {
        sanGain = std::max(0, sanGain - 8);
        energyGain = std::max(5, energyGain - 12);
        healthDelta = -5;
        lateNightRelief = 1;
    } else if (sleptMinutes < kMinimumHealthySleepMinutes) {
        healthDelta = -2;
        lateNightRelief = -2;
    }

    if (startDay != ctx.timeSystem.getDay()) {
        ctx.player.dailyAttributeCheck();
    }

    HiddenMap sleepDelta = HiddenMap::object();
    sleepDelta["lastSleepMinutes"] = sleptMinutes;
    sleepDelta["alarmSleepMinutes"] = targetMinutes;
    sleepDelta["consecutiveNoSleepDays"] = 0;
    sleepDelta["lastSleepDay"] = ctx.timeSystem.getDay();
    sleepDelta["lateNightLevel"] = lateNightRelief;
    if (healthDelta != 0) sleepDelta["healthIndex"] = healthDelta;
    mergeHidden(hidden, sleepDelta);

    ctx.player.modifyAttributes(Attributes{.energy = energyGain, .san = sanGain});
    resetActivityStreak(hidden);
    syncVisibleHealthFromHidden(ctx.player.getAttributes(), hidden);
    ctx.player.setPosition(480.0f, 276.0f);
    ctx.player.stopMovement();
    ctx.currentPlace = CampusPlace::Dormitory;
    ctx.currentMap = ctx.dormitoryMap;
    ctx.gamePlayDay = ctx.timeSystem.getDay();
    ctx.gamesPlayedToday = 0;

    std::ostringstream body;
    if (ctx.timeSystem.isFinished()) {
        body << cls::text("sleep.semester_complete") << "\n";
    }
    body << cls::format("sleep.result",
        {{"hours", std::to_string(sleptHours)},
         {"minutes", std::to_string(sleptRestMinutes)},
         {"energy", std::to_string(energyGain)},
         {"san", std::to_string(sanGain)}});
    if (overslept) {
        body << "\n" << cls::text("sleep.overslept");
    }
    if (sleptMinutes < kMinimumHealthySleepMinutes) {
        body << "\n" << cls::text("sleep.short_penalty");
    }

    ctx.timeSkipFlash.start(cls::text("time.sleeping"));
    if (ctx.finalizeStateChange) ctx.finalizeStateChange();
    if (!settlementActive) {
        ctx.showTimedResult(
            ctx.timeSystem.isFinished()
                ? cls::text("notice.days_complete")
                : (startDay == ctx.timeSystem.getDay()
                    ? cls::text("sleep.result.title")
                    : cls::text("notice.new_day")),
            body.str());
    }
}

/** @brief 从宿舍交互点触发睡觉流程 */
inline void sleepFromDormitory(GameContext& ctx, PendingSleep& pendingSleep) {
    if (!ctx.timeSystem.canSleep()) {
        ctx.activityNotice.show(cls::text("notice.too_early"),
            cls::text("sleep.too_early"));
        return;
    }
    auto& hidden = ctx.player.getHidden();
    normalizeHidden(hidden);
    const int previousSleep = hidden.value("lastSleepMinutes", kDefaultSleepMinutes);
    pendingSleep.active = true;
    ctx.mealChoicePrompt.show(
        cls::text("sleep.alarm.title"),
        cls::format("sleep.alarm.body", {{"last", durationLabel(previousSleep)}}),
        std::vector<std::string>{
            cls::text("sleep.alarm.option_default"),
            cls::text("sleep.alarm.option_7_5h"),
            cls::text("sleep.alarm.option_6h"),
            cls::text("sleep.alarm.option_5h")
        },
        kSleepPromptPurpose,
        std::vector<int>{0, kDefaultSleepMinutes, 6 * 60, 5 * 60});
}

#endif
