#include "state/MainQuestState.h"
#include "quest/QuestManager.h"
#include "quest/MainQuest.h"
#include "quest/ExamQuest.h"
#include "entity/Player.h"
#include <sstream>

MainQuestState::MainQuestState(Game* game, QuestManager* qm, Player* p)
    : GameState(game)
    , questManager(qm)
    , player(p)
    , font()
    , titleText(font, "", 28)
    , descriptionText(font, "", 18)
    , choiceText0(font, "", 16)
    , choiceText1(font, "", 16)
    , choiceText2(font, "", 16)
    , choiceText3(font, "", 16)
    , promptText(font, "", 14)
    , statText(font, "", 16)
    , background({960.0f, 540.0f})
    , selectedChoice(-1)
    , hoveredChoice(0)
{
#if defined(__APPLE__)
    static_cast<void>(font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf"));
#elif defined(_WIN32)
    static_cast<void>(font.openFromFile("C:/Windows/Fonts/arial.ttf"));
#elif defined(__linux__)
    static_cast<void>(font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"));
#endif

    background.setFillColor(sf::Color(0, 0, 0, 180));
}

sf::Text* MainQuestState::choiceTextPtr(int index) {
    switch (index) {
        case 0: return &choiceText0;
        case 1: return &choiceText1;
        case 2: return &choiceText2;
        case 3: return &choiceText3;
        default: return nullptr;
    }
}

void MainQuestState::onEnter() {
    currentQuest = questManager->createNextQuest();
    selectedChoice = -1;
    hoveredChoice = 0;
    updateTextDisplay();
}

void MainQuestState::onExit() {
    if (currentQuest) {
        currentQuest->execute(*player);
        questManager->onQuestCompleted();
    }
    currentQuest.reset();
}

void MainQuestState::handleInput(const sf::Event& event) {
    if (!currentQuest) return;

    int choiceMade = -1;
    bool advanced = currentQuest->handleInput(event, *player, choiceMade);

    if (choiceMade >= 0) {
        selectedChoice = choiceMade;
    }

    if (advanced) {
        if (currentQuest->isCompleted()) {
            onExit();
        }
        updateTextDisplay();
    }
}

void MainQuestState::update(float deltaTime) {
    (void)deltaTime;
}

void MainQuestState::render(sf::RenderWindow& window) {
    if (!currentQuest) return;

    window.draw(background);
    window.draw(titleText);
    window.draw(descriptionText);

    QuestPhase phase = currentQuest->getCurrentPhase();
    MainQuestType qtype = currentQuest->getQuestType();
    bool isExam = (qtype == MainQuestType::MIDTERM_EXAM || qtype == MainQuestType::FINAL_EXAM);

    if (isExam && phase != QuestPhase::COMPLETED) {
        window.draw(statText);
    }

    // 渲染选项文本
    if (phase == QuestPhase::CHOICE || phase == QuestPhase::PREPARATION) {
        for (int i = 0; i < static_cast<int>(currentQuest->getChoiceTexts().size()); ++i) {
            if (auto* t = choiceTextPtr(i)) window.draw(*t);
        }
    }

    window.draw(promptText);
}

void MainQuestState::updateTextDisplay() {
    if (!currentQuest) return;

    QuestPhase phase = currentQuest->getCurrentPhase();
    MainQuestType qtype = currentQuest->getQuestType();
    bool isExam = (qtype == MainQuestType::MIDTERM_EXAM || qtype == MainQuestType::FINAL_EXAM);

    titleText.setString(currentQuest->getQuestName());
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition({80.0f, 60.0f});

    descriptionText.setString(currentQuest->getDescription());
    descriptionText.setFillColor(sf::Color(220, 220, 220));
    descriptionText.setPosition({80.0f, 110.0f});

    // 清空所有选项文本
    for (int i = 0; i < 4; ++i) {
        if (auto* t = choiceTextPtr(i)) {
            t->setString("");
            t->setFillColor(sf::Color(200, 200, 200));
            t->setPosition({120.0f, 220.0f + i * 40.0f});
        }
    }

    // 填充选项文本
    const auto& choices = currentQuest->getChoiceTexts();
    if ((phase == QuestPhase::CHOICE || phase == QuestPhase::PREPARATION) && !choices.empty()) {
        for (int i = 0; i < static_cast<int>(choices.size()); ++i) {
            auto* t = choiceTextPtr(i);
            if (!t) continue;

            std::string prefix;
            if (phase == QuestPhase::PREPARATION) {
                prefix = (i == hoveredChoice) ? "> " : "  ";
            } else {
                prefix = (i == hoveredChoice) ? "> " : "  ";
            }

            const auto& outcomes = currentQuest->getChoiceOutcomes();
            std::string line = prefix + choices[i];
            if (i < static_cast<int>(outcomes.size()) && phase != QuestPhase::PREPARATION) {
                line += "  [" + formatDelta(outcomes[i]) + "]";
            }
            t->setString(line);
            t->setFillColor(
                i == hoveredChoice ? sf::Color(255, 255, 100) : sf::Color(200, 200, 200));
        }
    }

    // 考试特殊 UI
    if (isExam && phase != QuestPhase::COMPLETED) {
        auto* exam = dynamic_cast<ExamQuest*>(currentQuest.get());
        if (exam) {
            std::ostringstream ss;
            if (phase == QuestPhase::PREPARATION || phase == QuestPhase::ANNOUNCEMENT) {
                ss << "Subject: " << exam->getExamSubject()
                   << " | DC: " << exam->getSubjectDC()
                   << " | Total " << exam->getTotalRounds() << " rounds"
                   << " | Need " << exam->getRequiredPasses() << " passes";
                if (phase == QuestPhase::PREPARATION) {
                    ss << "\n\nReview before exam?";
                    if (exam->getHasReviewed())
                        ss << "  > [YES]    [NO]  (-" << exam->getReviewEnergyCost() << " Energy, +" << exam->getReviewBonus() << " bonus)";
                    else
                        ss << "    [YES]  > [NO]  (skip review)";
                }
            } else if (phase == QuestPhase::EXAM_ROUND || phase == QuestPhase::ROUND_RESULT) {
                ss << "Round " << exam->getCurrentRound() << "/" << exam->getTotalRounds()
                   << " | Subject: " << exam->getExamSubject()
                   << " | DC: " << exam->getSubjectDC()
                   << " | Passed: " << exam->getScore() << " rounds";
                if (phase == QuestPhase::ROUND_RESULT) {
                    const auto& roll = exam->getLastRoll();
                    ss << "\n\nD20: " << roll.d20Roll
                       << " | Academic Bonus: " << roll.academicBonus
                       << " | Review Bonus: " << roll.reviewBonus
                       << " | Total: " << roll.total
                       << " vs DC " << roll.dc
                       << " -> " << (roll.success ? "Pass!" : "Failed");
                }
            } else if (phase == QuestPhase::FINAL_RESULT) {
                bool passed = exam->getPassed();
                ss << "Result: " << (passed ? "Pass!" : "Failed")
                   << " | Score: " << exam->getScore() << "/" << exam->getTotalRounds();
            }
            statText.setString(ss.str());
            statText.setFillColor(sf::Color(180, 220, 255));
            statText.setPosition({80.0f, 180.0f});
        }
    }

    // 提示文本
    promptText.setFillColor(sf::Color(150, 150, 150));
    promptText.setPosition({80.0f, 460.0f});

    switch (phase) {
        case QuestPhase::ANNOUNCEMENT:
            promptText.setString("[Press Enter to continue]");
            break;
        case QuestPhase::CHOICE:
            promptText.setString("[Up/Down: Select  |  Enter: Confirm]");
            break;
        case QuestPhase::PREPARATION:
            promptText.setString("[Up/Down: Toggle Review/Skip  |  Enter: Confirm]");
            break;
        case QuestPhase::EXAM_ROUND:
            promptText.setString("[Press Enter to roll!]");
            break;
        case QuestPhase::ROUND_RESULT:
            promptText.setString("[Press Enter to continue]");
            break;
        case QuestPhase::FINAL_RESULT:
            promptText.setString("[Press Enter to return]");
            break;
        default:
            promptText.setString("");
            break;
    }
}

std::string MainQuestState::formatDelta(const Attributes& delta) const {
    std::ostringstream ss;
    bool first = true;
    auto add = [&](const char* name, int val) {
        if (val == 0) return;
        if (!first) ss << " ";
        first = false;
        ss << name << (val > 0 ? "+" : "") << val;
    };
    add("SAN", delta.san);
    add("Energy", delta.energy);
    add("Academic", delta.academic);
    add("Social", delta.social);
    add("Gold", delta.gold);
    return ss.str();
}
