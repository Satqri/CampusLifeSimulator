#include "quest/MidtermExamQuest.h"

MidtermExamQuest::MidtermExamQuest()
    : ExamQuest(
        "midterm_exam",
        "期中考试",
        "半个学期过去了，期中考试如期而至。你感到一丝紧张，但这也是检验自己学习成果的机会。",
        "基础学科",
        14,     // DC
        5,      // totalRounds
        3,      // requiredPasses
        Attributes(0, -5, 10, 0, 0),  // reward: energy -5, academic +10
        20,     // reviewEnergyCost
        3       // reviewBonusAmount
    )
{
    questType = MainQuestType::MIDTERM_EXAM;
}
