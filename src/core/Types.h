#ifndef CLS_CORE_TYPES_H
#define CLS_CORE_TYPES_H

#include <string>

struct Attributes {
    int san;
    int energy;   // Stamina - init 100
    int academic;
    int social;
    int gold;

    Attributes() : san(80), energy(100), academic(60), social(60), gold(100) {}
    Attributes(int s, int e, int a, int so, int g)
        : san(s), energy(e), academic(a), social(so), gold(g) {}
};

enum class EmotionType {
    ANXIETY,
    DEPRESSION,
    ANGER,
    FEAR,
    LONELINESS
};

enum class QuestPhase {
    NOT_STARTED,
    ANNOUNCEMENT,
    CHOICE,
    PREPARATION,
    EXAM_ROUND,
    ROUND_RESULT,
    FINAL_RESULT,
    COMPLETED
};

enum class MainQuestType {
    ORIENTATION,
    COURSE_SELECTION,
    CLUB_ACTIVITY,
    MIDTERM_EXAM,
    FINAL_EXAM,
    GRADUATION
};

struct ExamRollResult {
    int d20Roll;
    int academicBonus;
    int reviewBonus;
    int total;
    int dc;
    bool success;
};

enum class StateType {
    EXPLORATION,
    EVENT_DIALOG,
    COMBAT,
    MAIN_QUEST,
    MENU,
    GAME_OVER
};

struct CanteenMealResult {
    int stapleValue;
    int stapleScore;
    const char* stapleName;

    int mainDishValue;
    int mainDishScore;
    const char* mainDishName;

    int drinkValue;
    int drinkScore;
    const char* drinkName;

    int bonusScore;
    std::string bonusName;
    bool isLeopard;

    int finalRecovery;

    CanteenMealResult()
        : stapleValue(0), stapleScore(0), stapleName("")
        , mainDishValue(0), mainDishScore(0), mainDishName("")
        , drinkValue(0), drinkScore(0), drinkName("")
        , bonusScore(0), isLeopard(false), finalRecovery(0) {}
};

#endif