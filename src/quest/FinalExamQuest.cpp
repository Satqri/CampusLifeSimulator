#include "quest/FinalExamQuest.h"

FinalExamQuest::FinalExamQuest()
    : ExamQuest(
        "final_exam",
        "Final Exam",
        "End of the semester -- the most critical final exam is here! All your hard work comes down to this moment. Take a deep breath and focus.",
        "Major Comprehensive",
        16,     // DC
        7,      // totalRounds
        4,      // requiredPasses
        Attributes{.energy = -10, .academic = 20, .social = 5},  // reward: energy -10, academic +20, social +5
        30,     // reviewEnergyCost
        4       // reviewBonusAmount
    )
{
    questType = MainQuestType::FINAL_EXAM;
}
