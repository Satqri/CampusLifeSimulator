#ifndef CLS_CORE_TIMESYSTEM_H
#define CLS_CORE_TIMESYSTEM_H

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

#endif // CLS_CORE_TIMESYSTEM_H
