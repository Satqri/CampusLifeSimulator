#include "core/TimeSystem.h"

#include <iomanip>
#include <sstream>

int TimeSystem::advanceMinutes(int minutes) {
    if (minutes <= 0 || finished) return minuteOfDay;
    const int previous = minuteOfDay;
    minuteOfDay += minutes;
    return previous;
}

void TimeSystem::setTime(int hour, int minute) {
    setTimeAbsolute(hour * 60 + minute);
}

void TimeSystem::setTimeAbsolute(int minutes) {
    if (finished) return;
    minuteOfDay = minutes;
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

bool TimeSystem::crossedClassTime(int previousMinute) const {
    return !classPrompted
        && previousMinute < kClassMinute
        && minuteOfDay >= kClassMinute
        && minuteOfDay < 24 * 60;
}

bool TimeSystem::shouldForceClass() const {
    return !classPrompted && minuteOfDay >= kClassMinute && minuteOfDay < 24 * 60;
}

bool TimeSystem::isMealTime() const {
    const int minute = displayMinute();
    const bool lunch = minute >= 12 * 60 && minute < 14 * 60;
    const bool dinner = minute >= 17 * 60 && minute < 19 * 60;
    return lunch || dinner;
}

int TimeSystem::mealSlotId() const {
    const int minute = displayMinute();
    if (minute >= 12 * 60 && minute < 14 * 60) return day * 10 + 1;
    if (minute >= 17 * 60 && minute < 19 * 60) return day * 10 + 2;
    return -1;
}

bool TimeSystem::canSleep() const {
    return minuteOfDay >= kSleepAllowedMinute;
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
    ss << "Day " << std::setw(2) << std::setfill('0') << day
       << "/" << kMaxDay << "  "
       << std::setw(2) << hour << ":" << std::setw(2) << minutePart;
    return ss.str();
}

std::string TimeSystem::dayLabel() const {
    if (finished) return "Project days finished";
    if (isMidtermDay()) return "Midterm Day";
    return "Campus Day";
}

int TimeSystem::displayMinute() const {
    int minute = minuteOfDay % (24 * 60);
    if (minute < 0) minute += 24 * 60;
    return minute;
}

void TimeSystem::resetDailyFlags() {
    classPrompted = false;
    classResolved = false;
}
