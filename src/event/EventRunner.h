#ifndef CLS_EVENT_EVENTRUNNER_H
#define CLS_EVENT_EVENTRUNNER_H

#include "event/EventTypes.h"
#include <SFML/Window/Keyboard.hpp>
#include <random>
#include <string>
#include <unordered_map>

namespace sf { class RenderWindow; }
class ModalBox;
struct GameContext;

class EventRunner {
public:
    EventRunner();

    /** @brief 从 JSON 文件加载事件定义 */
    bool loadEvents(const std::string& filepath);

    /** @brief 当前是否有事件正在运行（阻塞游戏输入） */
    bool isActive() const { return mActive; }

    /** @brief 当前事件 ID（用于避免重复触发） */
    const std::string& currentEventId() const { return mCurrentEventId; }

    /** @brief 启动指定事件 */
    bool startEvent(const std::string& eventId, GameContext& ctx);

    /** @brief 处理键盘输入（Num1-3=选项, Enter=确认, Escape=取消） */
    void handleInput(sf::Keyboard::Key key, GameContext& ctx);

    /** @brief 渲染当前节点到 ModalBox */
    void render(sf::RenderWindow& window, ModalBox& modalBox);

    /** @brief 按交互点 actionId 查找并启动匹配事件 */
    bool triggerByAction(const std::string& actionId, GameContext& ctx);

    /** @brief 轮询所有触发器，首个命中则启动事件 */
    bool checkTriggers(GameContext& ctx, int previousMinute);

    /** @brief 重置当前事件 */
    void clear();

private:
    /** @brief 递归跳转到目标节点，自动解析 RANDOM_CHECK/CHECK */
    void transitionTo(const std::string& nodeId, GameContext& ctx);

    /** @brief 执行节点的附属效果（delta + time_advance + flash） */
    void applyEffects(const EventNode& node, GameContext& ctx);

    /** @brief 求值 conditions 数组 */
    bool evaluateConditions(const std::vector<Condition>& conditions,
                            const std::string& requireMode, GameContext& ctx);

    /** @brief 求值单条 condition */
    bool evaluateCondition(const Condition& cond, GameContext& ctx);

    std::unordered_map<std::string, EventDefinition> mEvents;
    EventDefinition* mCurrentEvent = nullptr;
    std::string mCurrentEventId;
    std::string mCurrentNodeId;
    bool mActive = false;
    bool mWaitingForEnter = false; ///< 等待玩家按 Enter 后再解析
    std::mt19937 mRng;
};

#endif
