#ifndef CLS_QUEST_EXAMQUEST_H
#define CLS_QUEST_EXAMQUEST_H

#include "quest/MainQuest.h"
#include <string>

/**
 * @class ExamQuest
 * @brief 考试任务抽象中间类
 *
 * 继承关系: MainQuest → ExamQuest（第 2 层），被 MidtermExamQuest/FinalExamQuest 继承（第 3 层）
 * 属性: examSubject, subjectDC, totalRounds, currentRound, score, lastRoll
 * 行为: 公告 → 复习选择 → 多轮 d20 检定 → 结算
 * 派生关系: 派生 MidtermExamQuest 和 FinalExamQuest 具体类
 */
class ExamQuest : public MainQuest {
public:
    ExamQuest(const std::string& id, const std::string& name,
              const std::string& desc, const std::string& subject,
              int dc, int totalRounds, int requiredPasses,
              const Attributes& reward,
              int reviewEnergyCost, int reviewBonusAmount);

    void execute(Player& player) override;
    void render(sf::RenderWindow& window) override;
    bool handleInput(const sf::Event& event, Player& player, int& choiceMade) override;

    // --- Exam-specific getters (used by MainQuestState for display) ---
    const std::string& getExamSubject() const { return examSubject; }
    int getSubjectDC() const { return subjectDC; }
    int getTotalRounds() const { return totalRounds; }
    int getCurrentRound() const { return currentRound; }
    int getScore() const { return score; }
    int getRequiredPasses() const { return requiredPasses; }
    bool getHasReviewed() const { return hasReviewed; }
    int getReviewEnergyCost() const { return reviewEnergyCost; }
    int getReviewBonus() const { return reviewBonus; }
    const ExamRollResult& getLastRoll() const { return lastRoll; }
    bool getPassed() const { return passed; }

protected:
    /**
     * @brief 执行一次 d20 检定
     * @param player 玩家引用
     *
     * 算法步骤:
     * 1. 计算学业加值 = (academic - 50) / 10
     * 2. 掷 d20 (1-20)
     * 3. total = d20Roll + academicBonus + reviewBonus
     * 4. 判定 success = total >= subjectDC
     */
    void performRoll(Player& player);

    /** @brief 推进到下一轮或结算阶段 */
    void advanceRound();

    /** @brief 根据累计分数计算是否通过 */
    void calculateResult();

    /** @brief 将考试结果应用到玩家属性 */
    void applyResult(Player& player);

    /** @brief 玩家选择复习：消耗体力，获得临时学业加值 */
    void review(Player& player);

    std::string examSubject;    ///< 考试科目名称
    int subjectDC;              ///< 基础 DC
    int totalRounds;            ///< 总轮数
    int currentRound;           ///< 当前轮次 (1-indexed)
    int score;                  ///< 已通过轮数
    int requiredPasses;         ///< 需要通过的最小轮数
    bool hasReviewed;           ///< 是否已复习
    int reviewEnergyCost;       ///< 复习消耗的体力
    int reviewBonusAmount;      ///< 复习加值配置量
    int reviewBonus;            ///< 复习获得的临时加值（实际生效值）
    ExamRollResult lastRoll;    ///< 最近一轮检定结果
    bool passed;                ///< 最终是否通过

    enum class ExamSubState {
        ANNOUNCEMENT,
        PREP_CHOICE,
        EXAM_ROLL,
        ROUND_RESULT,
        FINAL_RESULT
    };
    ExamSubState subState;      ///< 考试内部子阶段
};

#endif // CLS_QUEST_EXAMQUEST_H
