#ifndef SLS_QUEST_SIMPLEQUEST_H
#define SLS_QUEST_SIMPLEQUEST_H

#include "quest/MainQuest.h"
#include <utility>

/**
 * @class SimpleQuest
 * @brief 简单主线任务，配置驱动
 *
 * 继承关系: MainQuest → SimpleQuest（第 2 层）
 * 属性: outcomeTexts, hoveredChoiceIndex
 * 行为: 公告 → 选项选择 → 结果展示 三段式流程
 * 派生关系: 无（具体类，不再派生）
 *
 * 用于: Orientation / CourseSelection / ClubActivity / Graduation
 */
class SimpleQuest : public MainQuest {
public:
    /**
     * @brief 构造简单任务
     * @param id 任务标识
     * @param name 显示名称
     * @param desc 任务描述
     * @param choices 选项列表 (选项文本, 属性变化)
     * @param reward 完成奖励
     */
    SimpleQuest(const std::string& id, const std::string& name,
                const std::string& desc,
                const std::vector<std::pair<std::string, Attributes>>& choices,
                const Attributes& reward);

    void execute(Player& player) override;
    void render(sf::RenderWindow& window) override;
    bool handleInput(const sf::Event& event, Player& player, int& choiceMade) override;

private:
    std::vector<std::string> outcomeTexts; ///< 每个选项的结果描述文本
    int hoveredChoiceIndex;                ///< 当前悬停的选项索引
};

#endif // SLS_QUEST_SIMPLEQUEST_H
