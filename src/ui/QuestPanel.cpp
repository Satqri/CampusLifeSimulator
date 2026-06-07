#include "ui/QuestPanel.h"

#include "core/Localization.h"
#include "core/TextUtils.h"
#include "core/Types.h"
#include "quest/ExamQuest.h"
#include "quest/MainQuest.h"

#include <sstream>

QuestPanel::QuestPanel(sf::Font& font)
    : font(font)
    , quest(nullptr)
    , overlay({960.0f, 540.0f})
    , panel({840.0f, 450.0f})
{
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    panel.setPosition({60.0f, 45.0f});
    panel.setFillColor(sf::Color(18, 18, 28, 225));
    panel.setOutlineColor(sf::Color(90, 95, 125));
    panel.setOutlineThickness(1.0f);
}

void QuestPanel::setQuest(MainQuest* q) {
    quest = q;
}

void QuestPanel::update(float deltaTime) {
    (void)deltaTime;
}

void QuestPanel::render(sf::RenderWindow& window) {
    if (!quest) return;

    window.draw(overlay);
    window.draw(panel);

    sf::Text title = cls::makeText(font, quest->getQuestName(), 28);
    title.setFillColor(sf::Color::White);
    title.setPosition({80.0f, 60.0f});
    window.draw(title);

    sf::Text desc = cls::makeText(font, quest->getDescription(), 17);
    desc.setFillColor(sf::Color(220, 220, 220));
    desc.setPosition({80.0f, 110.0f});
    window.draw(desc);

    const std::string examStatus = buildExamStatus();
    if (!examStatus.empty()) {
        sf::Text stat = cls::makeText(font, examStatus, 15);
        stat.setFillColor(sf::Color(180, 220, 255));
        stat.setPosition({80.0f, 170.0f});
        window.draw(stat);
    }

    QuestPhase phase = quest->getCurrentPhase();
    const auto& choices = quest->getChoiceTexts();
    if ((phase == QuestPhase::CHOICE || phase == QuestPhase::PREPARATION) && !choices.empty()) {
        const int hovered = quest->getHoveredChoiceIndex();
        const int selected = quest->getSelectedChoiceIndex();
        for (int i = 0; i < static_cast<int>(choices.size()); ++i) {
            sf::Color color = sf::Color(200, 200, 200);
            if (i == selected && selected >= 0) {
                color = sf::Color(100, 255, 100);
            } else if (i == hovered) {
                color = sf::Color(255, 255, 100);
            }

            sf::Text choice = cls::makeText(font, buildChoiceLine(i), 16);
            choice.setFillColor(color);
            choice.setPosition({120.0f, 220.0f + i * 40.0f});
            window.draw(choice);
        }
    }

    sf::Text prompt = cls::makeText(font, buildPrompt(), 14);
    prompt.setFillColor(sf::Color(150, 150, 150));
    prompt.setPosition({80.0f, 460.0f});
    window.draw(prompt);
}

std::string QuestPanel::buildExamStatus() const {
    if (!quest) return "";

    auto* exam = dynamic_cast<ExamQuest*>(quest);
    QuestPhase phase = quest->getCurrentPhase();
    if (!exam || phase == QuestPhase::COMPLETED) return "";

    std::ostringstream ss;
    if (phase == QuestPhase::PREPARATION || phase == QuestPhase::ANNOUNCEMENT) {
        ss << cls::text("quest.subject") << ": " << exam->getExamSubject()
           << " | DC: " << exam->getSubjectDC()
           << " | " << cls::format("quest.rounds_total", {{"count", std::to_string(exam->getTotalRounds())}})
           << " | " << cls::format("quest.need_passes", {{"count", std::to_string(exam->getRequiredPasses())}});
        if (phase == QuestPhase::PREPARATION) {
            ss << "\n\n" << cls::text("quest.review_prompt");
            if (exam->getHasReviewed()) {
                ss << "  " << cls::text("quest.review_yes") << "    " << cls::text("quest.review_no")
                   << "  " << cls::format("quest.review_cost", {
                       {"cost", std::to_string(exam->getReviewEnergyCost())},
                       {"bonus", std::to_string(exam->getReviewBonus())}
                   });
            } else {
                ss << "    [YES]  > [NO]  " << cls::text("quest.review_skip");
            }
        }
    } else if (phase == QuestPhase::EXAM_ROUND || phase == QuestPhase::ROUND_RESULT) {
        ss << cls::format("quest.round", {
                {"current", std::to_string(exam->getCurrentRound())},
                {"total", std::to_string(exam->getTotalRounds())}
             })
           << " | DC: " << exam->getSubjectDC()
           << " | " << cls::format("quest.passed_rounds", {{"count", std::to_string(exam->getScore())}});
        if (phase == QuestPhase::ROUND_RESULT) {
            const auto& roll = exam->getLastRoll();
            ss << "\n\n  D20: " << roll.d20Roll
               << " | " << cls::text("quest.academic_bonus") << ": " << (roll.academicBonus >= 0 ? "+" : "")
               << roll.academicBonus
               << " | " << cls::text("quest.review_bonus") << ": " << (roll.reviewBonus >= 0 ? "+" : "")
               << roll.reviewBonus
               << " | " << cls::text("quest.total") << ": " << roll.total
               << " vs DC " << roll.dc
               << "  ->  " << (roll.success ? cls::text("quest.pass") : cls::text("quest.fail"));
        }
    } else if (phase == QuestPhase::FINAL_RESULT) {
        ss << cls::text("quest.final_result") << ": "
           << (exam->getPassed() ? cls::text("quest.passed") : cls::text("quest.failed"))
           << " | " << cls::text("quest.passed_rounds_short") << ": "
           << exam->getScore() << "/" << exam->getTotalRounds();
    }

    return ss.str();
}

std::string QuestPanel::buildChoiceLine(int index) const {
    if (!quest) return "";

    const auto& choices = quest->getChoiceTexts();
    if (index < 0 || index >= static_cast<int>(choices.size())) return "";

    std::string line = (index == quest->getHoveredChoiceIndex()) ? "> " : "  ";
    line += choices[index];

    const auto& outcomes = quest->getChoiceOutcomes();
    if (index < static_cast<int>(outcomes.size())) {
        const std::string delta = formatDelta(outcomes[index]);
        if (!delta.empty()) {
            line += "  [" + delta + "]";
        }
    }

    return line;
}

std::string QuestPanel::buildPrompt() const {
    if (!quest) return "";

    switch (quest->getCurrentPhase()) {
        case QuestPhase::ANNOUNCEMENT: return cls::text("quest.prompt.continue");
        case QuestPhase::CHOICE:       return cls::text("quest.prompt.choice");
        case QuestPhase::PREPARATION:  return cls::text("quest.prompt.prep");
        case QuestPhase::EXAM_ROUND:   return cls::text("quest.prompt.roll");
        case QuestPhase::ROUND_RESULT: return cls::text("quest.prompt.continue");
        case QuestPhase::FINAL_RESULT: return cls::text("quest.prompt.final");
        case QuestPhase::COMPLETED:    return cls::text("quest.prompt.completed");
        default:                       return "";
    }
}

std::string QuestPanel::formatDelta(const Attributes& delta) const {
    std::ostringstream ss;
    bool first = true;
    auto add = [&](const std::string& name, int val) {
        if (val == 0) return;
        if (!first) ss << " ";
        first = false;
        ss << name << (val > 0 ? "+" : "") << val;
    };
    add(cls::text("hud.san"), delta.san);
    add(cls::text("hud.energy"), delta.energy);
    add(cls::text("hud.academic"), delta.academic);
    add(cls::text("hud.social"), delta.social);
    add(cls::text("hud.gold"), delta.gold);
    return ss.str();
}
