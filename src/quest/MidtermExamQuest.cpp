#include "quest/MidtermExamQuest.h"

MidtermExamQuest::MidtermExamQuest()
    : ExamQuest(
        "midterm_exam",
        "Midterm Exam",
        "Half the semester has passed, and midterms have arrived. You feel a bit nervous, but this is also a chance to test what you've learned.",
        "Core Subjects",
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
