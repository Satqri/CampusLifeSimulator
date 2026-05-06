#include "quest/MainQuest.h"

MainQuest::MainQuest(const std::string& id, const std::string& name,
                     const std::string& desc, const Attributes& reward,
                     MainQuestType type)
    : questId(id)
    , questName(name)
    , description(desc)
    , currentPhase(QuestPhase::NOT_STARTED)
    , completed(false)
    , completionReward(reward)
    , questType(type)
    , selectedChoiceIndex(-1)
{
}

bool MainQuest::advancePhase() {
    if (currentPhase == QuestPhase::COMPLETED) return false;

    switch (currentPhase) {
        case QuestPhase::NOT_STARTED:  currentPhase = QuestPhase::ANNOUNCEMENT; break;
        case QuestPhase::ANNOUNCEMENT: currentPhase = QuestPhase::CHOICE;        break;
        case QuestPhase::CHOICE:       currentPhase = QuestPhase::FINAL_RESULT;  break;
        case QuestPhase::FINAL_RESULT: currentPhase = QuestPhase::COMPLETED;     break;
        default: return false;
    }
    return true;
}

void MainQuest::setPhase(QuestPhase phase) {
    currentPhase = phase;
}
