#ifndef CLS_STATE_MAINQUESTSTATE_H
#define CLS_STATE_MAINQUESTSTATE_H

#include "state/GameState.h"
#include "core/Types.h"
#include <SFML/Graphics.hpp>
#include <memory>

class QuestManager;
class MainQuest;
class Player;

/**
 * @class MainQuestState
 * @brief 主线任务游戏状态
 *
 * 继承关系: GameState → MainQuestState（第 2 层）
 * 属性: questManager, player, currentQuest, UI 元素
 * 行为: 驱动主线任务的完整生命周期（公告→选择/复习→检定→结果）
 * 派生关系: 无（具体类，不再派生）
 */
class MainQuestState : public GameState {
public:
    MainQuestState(Game* game, QuestManager* questManager, Player* player);

    void handleInput(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    /** @brief 进入状态时调用，创建当前任务 */
    void onEnter();

    /** @brief 退出状态时调用，结算任务效果 */
    void onExit();

private:
    QuestManager* questManager;
    Player* player;
    std::unique_ptr<MainQuest> currentQuest;

    // font 必须在 sf::Text 之前声明（Text 构造时需引用 font）
    sf::Font font;
    sf::Text titleText;
    sf::Text descriptionText;
    sf::Text choiceText0;
    sf::Text choiceText1;
    sf::Text choiceText2;
    sf::Text choiceText3;
    sf::Text promptText;
    sf::Text statText;
    sf::RectangleShape background;

    int selectedChoice;
    int hoveredChoice;

    /** @brief 根据当前任务阶段重建文本显示 */
    void updateTextDisplay();

    /** @brief 格式化属性变化为显示字符串 */
    std::string formatDelta(const Attributes& delta) const;

    /** @brief 获取 choiceTexts 数组的指针，方便遍历 */
    sf::Text* choiceTextPtr(int index);
};

#endif // CLS_STATE_MAINQUESTSTATE_H
