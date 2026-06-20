#include "core/TimeSystem.h"
#include "core/Localization.h"

#include <iomanip>
#include <random>
#include <sstream>

TimeSystem::TimeSystem() {
    generateRollCallMinute();
}

int TimeSystem::advanceMinutes(int minutes) {
    if (minutes <= 0 || finished) return minuteOfDay;
    const int previous = displayMinute();
    int remaining = minutes;
    while (remaining > 0 && !finished) {
        const int currentMinute = displayMinute();
        const int untilMidnight = 24 * 60 - currentMinute;
        if (remaining < untilMidnight) {
            minuteOfDay = currentMinute + remaining;
            remaining = 0;
        } else {
            remaining -= untilMidnight;
            if (day >= kMaxDay) {
                finished = true;
                minuteOfDay = kDayStartMinute;
            } else {
                ++day;
                minuteOfDay = 0;
                resetDailyFlags();
            }
        }
    }
    return previous;
}

void TimeSystem::setTime(int hour, int minute) {
    setTimeAbsolute(hour * 60 + minute);
}

void TimeSystem::setTimeAbsolute(int minutes) {
    if (finished) return;
    minuteOfDay = minutes;
}

void TimeSystem::generateRollCallMinute() {
    static std::mt19937 rng(std::random_device{}());
    constexpr int start = kClassMinute;
    constexpr int end = kClassEndMinute;
    constexpr int slotCount = (end - start) / kRollCallStepMinutes;
    std::uniform_int_distribution<int> dist(0, slotCount);
    rollCallMinute = start + dist(rng) * kRollCallStepMinutes;
    if (rollCallMinute > end) rollCallMinute = end;
}

int TimeSystem::sleepToNextDay() {
    if (finished) return 0;

    const int targetNextMorning = minuteOfDay <= kDayStartMinute
        ? kDayStartMinute
        : 24 * 60 + kDayStartMinute;
    const int sleptMinutes = targetNextMorning - minuteOfDay;

    if (day >= kMaxDay) {
        finished = true;
        minuteOfDay = kDayStartMinute;
        return sleptMinutes > 0 ? sleptMinutes : 0;
    }

    ++day;
    minuteOfDay = kDayStartMinute;
    resetDailyFlags();
    return sleptMinutes > 0 ? sleptMinutes : 0;
}

int TimeSystem::sleepForMinutes(int minutes) {
    if (finished || minutes <= 0) return 0;
    int remaining = minutes;
    int slept = 0;
    while (remaining > 0 && !finished) {
        const int untilMidnight = 24 * 60 - displayMinute();
        if (remaining < untilMidnight) {
            minuteOfDay = displayMinute() + remaining;
            slept += remaining;
            remaining = 0;
        } else {
            remaining -= untilMidnight;
            slept += untilMidnight;
            if (day >= kMaxDay) {
                finished = true;
                minuteOfDay = kDayStartMinute;
            } else {
                ++day;
                minuteOfDay = 0;
                resetDailyFlags();
            }
        }
    }

    return slept;
}

bool TimeSystem::crossedRollCallTime(int previousMinute) const {
    return !classPrompted
        && previousMinute < rollCallMinute
        && minuteOfDay >= rollCallMinute
        && minuteOfDay < 24 * 60;
}

bool TimeSystem::shouldForceClass() const {
    return !classPrompted && minuteOfDay >= rollCallMinute && minuteOfDay < 24 * 60;
}

bool TimeSystem::isMealTime() const {
    const int minute = displayMinute();
    const bool breakfast = minute >= kBreakfastStartMinute && minute < kBreakfastEndMinute;
    const bool lunch = minute >= kLunchStartMinute && minute < kLunchEndMinute;
    const bool dinner = minute >= kDinnerStartMinute && minute < kDinnerEndMinute;
    return breakfast || lunch || dinner;
}

int TimeSystem::mealSlotId() const {
    const int minute = displayMinute();
    if (minute >= kBreakfastStartMinute && minute < kBreakfastEndMinute) return day * 10 + 1;
    if (minute >= kLunchStartMinute && minute < kLunchEndMinute) return day * 10 + 2;
    if (minute >= kDinnerStartMinute && minute < kDinnerEndMinute) return day * 10 + 3;
    return -1;
}

bool TimeSystem::canSleep() const {
    const int minute = displayMinute();
    return minute >= kSleepAllowedMinute || minute < 12 * 60;
}

TimePhase TimeSystem::currentPhase() const {
    const int minute = displayMinute();
    if (minute >= 6 * 60 && minute < 11 * 60) return TimePhase::EarlyMorning;
    if (minute >= 11 * 60 && minute < 14 * 60) return TimePhase::Noon;
    if (minute >= 14 * 60 && minute < 17 * 60 + 30) return TimePhase::Afternoon;
    if (minute >= 17 * 60 + 30 && minute < 18 * 60 + 30) return TimePhase::Evening;
    return TimePhase::Night;
}

std::string TimeSystem::clockText() const {
    const int minute = displayMinute();
    const int hour = minute / 60;
    const int minutePart = minute % 60;

    std::ostringstream ss;
    ss << cls::format("time.day", {{"day", std::to_string(day)}})
       << "/" << kMaxDay << "  "
       << std::setw(2) << std::setfill('0') << hour << ":" << std::setw(2) << minutePart;
    return ss.str();
}

std::string TimeSystem::dayLabel() const {
    if (finished) return cls::text("time.finished");
    if (isMidtermDay()) return cls::text("page.midterm");

    switch (currentPhase()) {
        case TimePhase::EarlyMorning: return cls::text("time.early_morning");
        case TimePhase::Noon:         return cls::text("time.noon");
        case TimePhase::Afternoon:    return cls::text("time.afternoon");
        case TimePhase::Evening:      return cls::text("time.evening");
        case TimePhase::Night:        return cls::text("time.night");
    }
    return cls::text("time.afternoon");
}

int TimeSystem::displayMinute() const {
    int minute = minuteOfDay % (24 * 60);
    if (minute < 0) minute += 24 * 60;
    return minute;
}

void TimeSystem::resetDailyFlags() {
    classPrompted = false;
    classResolved = false;
    generateRollCallMinute();
}
