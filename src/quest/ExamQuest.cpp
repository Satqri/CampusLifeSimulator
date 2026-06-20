#include "quest/ExamQuest.h"
#include "entity/Player.h"
#include <cstdlib>

ExamQuest::ExamQuest(const std::string& id, const std::string& name,
                     const std::string& desc, const std::string& subject,
                     int dc, int totalRounds, int requiredPasses,
                     const Attributes& reward,
                     int reviewEnergyCost, int reviewBonusAmount)
    : MainQuest(id, name, desc, reward, MainQuestType::MIDTERM_EXAM)
    , examSubject(subject)
    , subjectDC(dc)
    , totalRounds(totalRounds)
    , currentRound(0)
    , score(0)
    , requiredPasses(requiredPasses)
    , hasReviewed(false)
    , reviewEnergyCost(reviewEnergyCost)
    , reviewBonus(0)
    , passed(false)
    , subState(ExamSubState::ANNOUNCEMENT)
{
    currentPhase = QuestPhase::ANNOUNCEMENT;
    this->reviewBonusAmount = reviewBonusAmount;
}

void ExamQuest::execute(Player& player) {
    applyResult(player);
    player.modifyAttributes(completionReward);
    syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
    completed = true;
}

void ExamQuest::render(sf::RenderWindow& window) {
    (void)window;
    // 渲染由 MainQuestState 统一管理
}

bool ExamQuest::handleInput(const sf::Event& event, Player& player, int& choiceMade) {
    (void)choiceMade;

    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) return false;

    using sf::Keyboard::Key;
    auto code = key->code;

    if (code != Key::Enter && code != Key::Up && code != Key::Down)
        return false;

    switch (subState) {
        case ExamSubState::ANNOUNCEMENT:
            if (code == Key::Enter) {
                subState = ExamSubState::PREP_CHOICE;
                currentPhase = QuestPhase::PREPARATION;
                return true;
            }
            break;

        case ExamSubState::PREP_CHOICE:
            if (code == Key::Up || code == Key::Down) {
                hasReviewed = !hasReviewed;
                return true;
            }
            if (code == Key::Enter) {
                if (hasReviewed) {
                    review(player);
                }
                currentRound = 1;
                subState = ExamSubState::EXAM_ROLL;
                currentPhase = QuestPhase::EXAM_ROUND;
                return true;
            }
            break;

        case ExamSubState::EXAM_ROLL:
            if (code == Key::Enter) {
                performRoll(player);
                subState = ExamSubState::ROUND_RESULT;
                currentPhase = QuestPhase::ROUND_RESULT;
                return true;
            }
            break;

        case ExamSubState::ROUND_RESULT:
            if (code == Key::Enter) {
                advanceRound();
                // advanceRound sets subState → need to sync phase
                if (subState == ExamSubState::EXAM_ROLL)
                    currentPhase = QuestPhase::EXAM_ROUND;
                else
                    currentPhase = QuestPhase::FINAL_RESULT;
                return true;
            }
            break;

        case ExamSubState::FINAL_RESULT:
            if (code == Key::Enter) {
                currentPhase = QuestPhase::COMPLETED;
                completed = true;
                return true;
            }
            break;

        default:
            break;
    }
    return false;
}

void ExamQuest::performRoll(Player& player) {
    const Attributes& attrs = player.getAttributes();
    int academicBonus = (attrs.academic - 50) / 10;

    lastRoll.d20Roll = (std::rand() % 20) + 1;
    lastRoll.academicBonus = academicBonus;
    lastRoll.reviewBonus = reviewBonus;
    lastRoll.total = lastRoll.d20Roll + academicBonus + reviewBonus;
    lastRoll.dc = subjectDC;
    lastRoll.success = lastRoll.total >= subjectDC;

    if (lastRoll.success) {
        score++;
    }
}

void ExamQuest::advanceRound() {
    if (currentRound < totalRounds) {
        currentRound++;
        subState = ExamSubState::EXAM_ROLL;
    } else {
        calculateResult();
        subState = ExamSubState::FINAL_RESULT;
    }
}

void ExamQuest::calculateResult() {
    passed = (score >= requiredPasses);
}

void ExamQuest::applyResult(Player& player) {
    Attributes delta;
    if (passed) {
        if (score >= static_cast<int>(totalRounds * 0.8f)) {
            // 优异成绩: >=80% 通过率
            delta.academic = 15;
            delta.social = 5;
            delta.energy = -10;
        } else {
            // 勉强通过
            delta.academic = 10;
            delta.energy = -10;
        }
        delta.san = 10; // 通过考试恢复理智
    } else {
        // 未通过
        delta.academic = 3;
        delta.san = -10;
        delta.energy = -10;
    }
    player.modifyAttributes(delta);
    syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
}

void ExamQuest::review(Player& player) {
    Attributes delta;
    delta.energy = -reviewEnergyCost;
    player.modifyAttributes(delta);
    syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
    reviewBonus = reviewBonusAmount;
}
