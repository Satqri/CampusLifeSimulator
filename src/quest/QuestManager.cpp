#include "quest/QuestManager.h"
#include "quest/MainQuest.h"
#include "quest/SimpleQuest.h"
#include "quest/ExamQuest.h"
#include "quest/MidtermExamQuest.h"
#include "quest/FinalExamQuest.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

QuestManager::QuestManager()
    : completedEventCount(0)
    , currentQuestIndex(0)
    , allCompleted(false)
    , questActive(false)
{
}

bool QuestManager::loadQuestChain(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    json data;
    file >> data;
    file.close();

    questChain.clear();

    for (const auto& q : data["quests"]) {
        QuestDef def;
        std::string typeStr = q.value("type", "");
        if (typeStr == "orientation")       def.type = MainQuestType::ORIENTATION;
        else if (typeStr == "course_selection") def.type = MainQuestType::COURSE_SELECTION;
        else if (typeStr == "club_activity")    def.type = MainQuestType::CLUB_ACTIVITY;
        else if (typeStr == "midterm_exam")     def.type = MainQuestType::MIDTERM_EXAM;
        else if (typeStr == "final_exam")       def.type = MainQuestType::FINAL_EXAM;
        else if (typeStr == "graduation")       def.type = MainQuestType::GRADUATION;
        else continue;

        def.eventThreshold = q.value("threshold", 0);
        def.customId = q.value("id", "");
        def.displayName = q.value("name", "");
        def.description = q.value("description", "");

        // Choices (for SimpleQuest types)
        if (q.contains("choices")) {
            for (const auto& c : q["choices"]) {
                std::string text = c.value("text", "");
                Attributes delta;
                if (c.contains("delta")) {
                    const auto& d = c["delta"];
                    delta.san     = d.value("san", 0);
                    delta.energy  = d.value("energy", 0);
                    delta.academic = d.value("academic", 0);
                    delta.social  = d.value("social", 0);
                    delta.gold    = d.value("gold", 0);
                }
                def.choices.push_back({text, delta});
            }
        }

        // Reward
        if (q.contains("reward")) {
            const auto& r = q["reward"];
            def.reward.san     = r.value("san", 0);
            def.reward.energy  = r.value("energy", 0);
            def.reward.academic = r.value("academic", 0);
            def.reward.social  = r.value("social", 0);
            def.reward.gold    = r.value("gold", 0);
        }

        // ExamQuest params
        def.subject           = q.value("subject", "");
        def.dc                = q.value("dc", 14);
        def.totalRounds       = q.value("totalRounds", 5);
        def.requiredPasses    = q.value("requiredPasses", 3);
        def.reviewEnergyCost  = q.value("reviewEnergyCost", 20);
        def.reviewBonus       = q.value("reviewBonus", 3);

        questChain.push_back(def);
    }

    currentQuestIndex = 0;
    allCompleted = questChain.empty();
    return true;
}

void QuestManager::onEventCompleted() {
    completedEventCount++;
}

bool QuestManager::shouldTriggerQuest() const {
    if (questActive || allCompleted) return false;
    if (currentQuestIndex >= static_cast<int>(questChain.size())) return false;
    return completedEventCount >= questChain[currentQuestIndex].eventThreshold;
}

std::unique_ptr<MainQuest> QuestManager::createNextQuest() {
    if (currentQuestIndex >= static_cast<int>(questChain.size())) return nullptr;

    const QuestDef& def = questChain[currentQuestIndex];
    std::unique_ptr<MainQuest> quest;

    switch (def.type) {
        case MainQuestType::ORIENTATION:
        case MainQuestType::COURSE_SELECTION:
        case MainQuestType::CLUB_ACTIVITY:
        case MainQuestType::GRADUATION:
            quest = std::make_unique<SimpleQuest>(
                def.customId, def.displayName, def.description,
                def.choices, def.reward);
            break;

        case MainQuestType::MIDTERM_EXAM:
            quest = std::make_unique<MidtermExamQuest>();
            break;

        case MainQuestType::FINAL_EXAM:
            quest = std::make_unique<FinalExamQuest>();
            break;
    }

    questActive = true;
    return quest;
}

void QuestManager::onQuestCompleted() {
    questActive = false;
    currentQuestIndex++;
    if (currentQuestIndex >= static_cast<int>(questChain.size())) {
        allCompleted = true;
    }
}

int QuestManager::getNextThreshold() const {
    if (allCompleted || currentQuestIndex >= static_cast<int>(questChain.size()))
        return -1;
    return questChain[currentQuestIndex].eventThreshold;
}

float QuestManager::getSemesterProgress() const {
    if (questChain.empty()) return 0.0f;
    return static_cast<float>(currentQuestIndex) / static_cast<float>(questChain.size());
}
