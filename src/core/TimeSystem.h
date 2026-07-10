#ifndef CLS_CORE_TIMESYSTEM_H
#define CLS_CORE_TIMESYSTEM_H

#include "core/Localization.h"
#include <string>

enum class TimePhase {
    EarlyMorning,
    Noon,
    Afternoon,
    Evening,
    Night
};

class TimeSystem {
public:
    static constexpr int kMaxDay = 14;
    static constexpr int kDayStartMinute = 8 * 60;
    static constexpr int kClassMinute = 8 * 60 + 50;
    static constexpr int kClassEndMinute = 12 * 60 + 15;
    static constexpr int kRollCallMinute = 10 * 60 + 20;
    static constexpr int kRollCallStepMinutes = 10;
    static constexpr int kSleepAllowedMinute = 21 * 60;
    static constexpr int kBreakfastStartMinute = 6 * 60 + 30;
    static constexpr int kBreakfastEndMinute = 10 * 60;
    static constexpr int kLunchStartMinute = 11 * 60 + 30;
    static constexpr int kLunchEndMinute = 13 * 60 + 30;
    static constexpr int kDinnerStartMinute = 16 * 60 + 40;
    static constexpr int kDinnerEndMinute = 19 * 60 + 30;

    TimeSystem();

    int getDay() const { return day; }
    int getMinuteOfDay() const { return minuteOfDay; }
    int getRollCallMinute() const { return rollCallMinute; }
    void setRollCallMinute(int minutes) { rollCallMinute = minutes; }
    bool isMidtermDay() const { return day == 7; }
    bool isFinished() const { return finished; }

    bool isClassPrompted() const { return classPrompted; }
    bool isClassResolved() const { return classResolved; }
    void markClassPrompted() { classPrompted = true; }
    void markClassResolved() { classResolved = true; }

    int advanceMinutes(int minutes);
    void generateRollCallMinute();
    void setTime(int hour, int minute);
    void setTimeAbsolute(int minutes);
    int sleepForMinutes(int minutes);
    int sleepToNextDay();

    bool crossedRollCallTime(int previousMinute) const;
    bool shouldForceClass() const;
    bool isMealTime() const;
    int mealSlotId() const;
    bool canSleep() const;
    TimePhase currentPhase() const;

    std::string clockText() const;
    std::string dayLabel() const;

private:
    int displayMinute() const;
    void resetDailyFlags();

    int day = 1;
    int minuteOfDay = kDayStartMinute;
    int rollCallMinute = kRollCallMinute;
    bool classPrompted = false;
    bool classResolved = false;
    bool finished = false;
};

// ── 时间工具函数 ──────────────────────────────────────

constexpr int kMinutesPerDay = 24 * 60;

/** @brief 将分钟数归一化到 [0, 1440) 范围 */
inline int normalizedMinute(int minute) {
    int result = minute % kMinutesPerDay;
    if (result < 0) result += kMinutesPerDay;
    return result;
}

/** @brief 判断 minute 是否在 [start, end) 时钟窗口内（支持跨日） */
inline bool isWithinClockWindow(int minute, int start, int end) {
    minute = normalizedMinute(minute);
    start = normalizedMinute(start);
    end = normalizedMinute(end);
    if (start <= end) return minute >= start && minute < end;
    return minute >= start || minute < end;
}

/** @brief 从 TimeSystem 计算绝对游戏分钟（从第 1 天 00:00 起） */
inline int absoluteGameMinute(const TimeSystem& timeSystem) {
    return (timeSystem.getDay() - 1) * kMinutesPerDay
        + normalizedMinute(timeSystem.getMinuteOfDay());
}

/** @brief 将分钟数格式化为可读时长文本 */
inline std::string durationLabel(int minutes) {
    if (minutes < 60)
        return cls::format("activity.duration.minutes", {{"minutes", std::to_string(minutes)}});
    const int hours = minutes / 60;
    const int rest = minutes % 60;
    if (rest == 0)
        return cls::format("activity.duration.hours", {{"hours", std::to_string(hours)}});
    return cls::format("activity.duration.hours_minutes",
        {{"hours", std::to_string(hours)}, {"minutes", std::to_string(rest)}});
}

#endif // CLS_CORE_TIMESYSTEM_H
