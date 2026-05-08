#ifndef CLS_QUEST_MAINQUEST_H
#define CLS_QUEST_MAINQUEST_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "core/Types.h"

class Player;

/**
 * @class MainQuest
 * @brief 主线任务抽象基类，所有主线任务的根类
 *
 * 继承关系: 根类，被 SimpleQuest 和 ExamQuest 继承（第 1 层）
 * 属性: questId, questName, description, currentPhase, choiceTexts/choiceOutcomes
 * 行为: 提供 execute()/render()/handleInput() 纯虚接口，子类多态实现
 * 派生关系: 派生 SimpleQuest 具体类和 ExamQuest 抽象中间类
 */
class MainQuest {
public:
    MainQuest(const std::string& id, const std::string& name,
              const std::string& desc, const Attributes& reward,
              MainQuestType type);
    virtual ~MainQuest() = default;

    /**
     * @brief 执行任务效果，修改玩家属性
     * @param player 玩家引用
     *
     * 纯虚函数，由具体子类实现不同的结算逻辑
     */
    virtual void execute(Player& player) = 0;

    /**
     * @brief 渲染任务 UI
     * @param window SFML 渲染窗口引用
     *
     * 纯虚函数，由具体子类实现不同阶段的 UI 渲染
     */
    virtual void render(sf::RenderWindow& window) = 0;

    /**
     * @brief 处理任务中的键盘/鼠标输入
     * @param event SFML 事件
     * @param player 玩家引用（用于复习等需要修改属性的操作）
     * @param choiceMade 输出参数：玩家做出的选择索引，-1 表示未选择
     * @return true 输入被消费（阶段推进），false 无变化
     */
    virtual bool handleInput(const sf::Event& event, Player& player, int& choiceMade) = 0;

    // --- Getters ---
    const std::string& getQuestId() const { return questId; }
    const std::string& getQuestName() const { return questName; }
    const std::string& getDescription() const { return description; }
    QuestPhase getCurrentPhase() const { return currentPhase; }
    bool isCompleted() const { return completed; }
    const Attributes& getCompletionReward() const { return completionReward; }
    MainQuestType getQuestType() const { return questType; }
    const std::vector<std::string>& getChoiceTexts() const { return choiceTexts; }
    const std::vector<Attributes>& getChoiceOutcomes() const { return choiceOutcomes; }
    int getSelectedChoiceIndex() const { return selectedChoiceIndex; }
    virtual int getHoveredChoiceIndex() const { return 0; }

    /**
     * @brief 推进到下一阶段
     * @return true 推进成功, false 已完成无法推进
     */
    bool advancePhase();

    /** @brief 直接设置阶段（子类使用） */
    void setPhase(QuestPhase phase);

protected:
    std::string questId;              ///< 任务唯一标识
    std::string questName;            ///< 任务显示名称
    std::string description;          ///< 任务描述文本
    QuestPhase currentPhase;          ///< 当前阶段
    bool completed;                   ///< 是否已完成
    Attributes completionReward;      ///< 完成奖励属性
    MainQuestType questType;          ///< 任务类型标识

    std::vector<std::string> choiceTexts;     ///< 选项文本列表
    std::vector<Attributes> choiceOutcomes;   ///< 选项属性变化列表
    int selectedChoiceIndex;                  ///< 玩家选择的索引 (-1 未选择)
};

#endif // CLS_QUEST_MAINQUEST_H
