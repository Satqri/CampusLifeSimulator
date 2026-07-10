#ifndef CLS_CORE_ACTIVITYRUNNER_H
#define CLS_CORE_ACTIVITYRUNNER_H

#include "core/CharacterState.h"
#include "core/GameContext.h"
#include "core/Localization.h"
#include "core/SleepSystem.h"
#include "core/TimeSystem.h"
#include <sstream>

constexpr int kMinActivityMinutes = 10;
constexpr int kMaxActivityMinutes = 120;
constexpr char kDurationPromptPurpose[] = "activity_duration";

struct PendingTimedActivity {
    bool active = false;
    int baseMinutes = 0;
    Attributes delta{};
    HiddenMap hiddenDelta = HiddenMap::object();
    std::string title;
    std::string body;
    std::string activityId;
    bool hasHidden = false;

    void clear() {
        active = false;
        baseMinutes = 0;
        delta = Attributes{};
        hiddenDelta = HiddenMap::object();
        title.clear();
        body.clear();
        activityId.clear();
        hasHidden = false;
    }
};

/** @brief 在活动结果文本末尾追加重复活动提示 */
inline std::string appendRepeatNotice(const std::string& body, int streak) {
    if (streak <= 1) return body;
    return body + "\n\n" + cls::format("activity.repeat_penalty",
        {{"streak", std::to_string(streak)}});
}

/** @brief 执行限时活动（时间推进 + 属性变化 + 事件触发） */
inline void executeTimedActivity(GameContext& ctx,
                                  int baseMinutes, int requestedMinutes,
                                  const Attributes& delta,
                                  const HiddenMap& hiddenDelta, bool hasHidden,
                                  const std::string& title, const std::string& body,
                                  const std::string& activityId) {
    const int startMinute = ctx.timeSystem.getMinuteOfDay();
    const int startDay = ctx.timeSystem.getDay();
    int actualMinutes = std::clamp(requestedMinutes, kMinActivityMinutes, kMaxActivityMinutes);
    const int rollCallMinute = ctx.timeSystem.getRollCallMinute();
    const bool interruptedByRollCall = !ctx.timeSystem.isClassPrompted()
        && ctx.currentPlace != CampusPlace::Classroom
        && startDay == ctx.timeSystem.getDay()
        && startMinute < rollCallMinute
        && startMinute + actualMinutes >= rollCallMinute;
    if (interruptedByRollCall) {
        actualMinutes = std::max(0, rollCallMinute - startMinute);
    }

    const int prev = ctx.timeSystem.advanceMinutes(actualMinutes);
    if (ctx.timeSystem.getDay() != startDay) {
        markNoSleepForSkippedDays(ctx);
        ctx.player.dailyAttributeCheck();
        ctx.gamePlayDay = ctx.timeSystem.getDay();
        ctx.gamesPlayedToday = 0;
    }
    if (actualMinutes <= 0) {
        if (interruptedByRollCall && ctx.checkEventTriggers && ctx.checkEventTriggers(prev)) {
            if (ctx.finalizeStateChange) ctx.finalizeStateChange();
            return;
        }
        ctx.showTimedResult(title, body);
        return;
    }

    const std::string activityKey = activityId.empty()
        ? activityKeyFor(title, body) : activityId;
    const int streak = updateActivityStreak(ctx.player.getHidden(), activityKey);
    const Attributes durationDelta = scaleAttributesByDuration(delta, actualMinutes, baseMinutes);
    const Attributes adjustedDelta = adjustAttributesForRepetition(durationDelta, streak);
    ctx.player.modifyAttributes(adjustedDelta);
    if (hasHidden) {
        const HiddenMap durationHiddenDelta = scaleHiddenByDuration(hiddenDelta, actualMinutes, baseMinutes);
        const HiddenMap adjustedHiddenDelta = adjustHiddenForRepetition(durationHiddenDelta, streak);
        if (!adjustedHiddenDelta.is_null()) {
            mergeHidden(ctx.player.getHidden(), adjustedHiddenDelta);
        }
    }
    syncVisibleHealthFromHidden(ctx.player.getAttributes(), ctx.player.getHidden());
    ctx.timeSkipFlash.start("Time passes...");
    if (ctx.finalizeStateChange && ctx.finalizeStateChange()) return;

    if (interruptedByRollCall) {
        if (ctx.checkEventTriggers && ctx.checkEventTriggers(prev)) {
            if (ctx.finalizeStateChange) ctx.finalizeStateChange();
            return;
        }
    }

    std::string resultBody = body;
    resultBody += "\n\n" + cls::format("activity.duration_result",
        {{"minutes", std::to_string(actualMinutes)}, {"target", std::to_string(requestedMinutes)}});
    ctx.showTimedResult(title, appendRepeatNotice(resultBody, streak));
    if (ctx.checkEventTriggers) ctx.checkEventTriggers(prev);
    if (ctx.finalizeStateChange) ctx.finalizeStateChange();
}

/** @brief 弹出时长选择器 */
inline void promptTimedActivityDuration(GameContext& ctx, PendingTimedActivity& pending,
                                         int minutes, const Attributes& delta,
                                         const HiddenMap& hiddenDelta, bool hasHidden,
                                         const std::string& title, const std::string& body,
                                         const std::string& activityId) {
    pending.active = true;
    pending.baseMinutes = std::max(1, minutes);
    pending.delta = delta;
    pending.hiddenDelta = hiddenDelta;
    pending.title = title;
    pending.body = body;
    pending.activityId = activityId;
    pending.hasHidden = hasHidden;

    const int initialMinutes = std::clamp(
        ((minutes + kMinActivityMinutes / 2) / kMinActivityMinutes) * kMinActivityMinutes,
        kMinActivityMinutes,
        kMaxActivityMinutes);
    ctx.mealChoicePrompt.showRange(
        cls::text("activity.duration.title"),
        cls::format("activity.duration.body",
            {{"activity", title}, {"base", durationLabel(minutes)}}),
        kDurationPromptPurpose,
        initialMinutes,
        kMinActivityMinutes,
        kMaxActivityMinutes,
        kMinActivityMinutes);
}

/** @brief 运行限时活动（可选时长） */
inline void runTimedActivity(GameContext& ctx, PendingTimedActivity& pending,
                              int minutes, const Attributes& delta,
                              const std::string& title, const std::string& body,
                              const std::string& activityId, bool customDuration) {
    if (customDuration) {
        promptTimedActivityDuration(ctx, pending,
            minutes, delta, HiddenMap::object(), false,
            title, body, activityId);
        return;
    }
    executeTimedActivity(ctx, minutes, minutes,
        delta, HiddenMap::object(), false,
        title, body, activityId);
}

/** @brief 运行限时活动（带隐藏变量） */
inline void runTimedActivityWithHidden(GameContext& ctx, PendingTimedActivity& pending,
                                        int minutes, const Attributes& delta,
                                        const HiddenMap& hiddenDelta,
                                        const std::string& title, const std::string& body,
                                        const std::string& activityId, bool customDuration) {
    if (customDuration) {
        promptTimedActivityDuration(ctx, pending,
            minutes, delta, hiddenDelta, true,
            title, body, activityId);
        return;
    }
    executeTimedActivity(ctx, minutes, minutes,
        delta, hiddenDelta, true,
        title, body, activityId);
}

#endif
