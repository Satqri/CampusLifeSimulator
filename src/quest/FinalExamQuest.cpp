#include "quest/FinalExamQuest.h"

FinalExamQuest::FinalExamQuest()
    : ExamQuest(
        "final_exam",
        "期末考试",
        "学期末，最关键的期末考试来了！整个学期的努力将在这一刻见分晓。深呼吸，集中精力。",
        "专业综合",
        16,     // DC
        7,      // totalRounds
        4,      // requiredPasses
        Attributes(0, -10, 20, 5, 0),  // reward: energy -10, academic +20, social +5
        30,     // reviewEnergyCost
        4       // reviewBonusAmount
    )
{
    questType = MainQuestType::FINAL_EXAM;
}
