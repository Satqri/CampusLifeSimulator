#ifndef CLS_EVENT_EVENTRUNNER_H
#define CLS_EVENT_EVENTRUNNER_H

#include "event/EventTypes.h"
#include <SFML/Window/Keyboard.hpp>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace sf { class RenderWindow; }
class ModalBox;
struct GameContext;

struct DebugEventInfo {
    std::string id;
    std::string triggerType;
    std::string triggerLabel;
    std::string actionId;
    bool currentContext = false;
    bool conditionsPass = false;
    bool gatePass = false;
    bool eligible = false;
    int chance = 100;
    int triggerCount = 0;
    int lastDay = -1;
    bool once = false;
    int cooldownDays = 0;
    std::string reason;
};

class EventRunner {
public:
    EventRunner();

    /** @brief 从 JSON 文件加载事件定义 */
    bool loadEvents(const std::string& filepath);

    /** @brief 当前是否有事件正在运行（阻塞游戏输入） */
    bool isActive() const { return mActive; }

    /** @brief 当前事件 ID（用于避免重复触发） */
    const std::string& currentEventId() const { return mCurrentEventId; }

    /** @brief 当前事件节点 ID（Debug/QA 面板使用） */
    const std::string& currentNodeId() const { return mCurrentNodeId; }

    /** @brief 最近事件运行记录（Debug/QA 面板使用） */
    const std::vector<std::string>& debugHistory() const { return mDebugHistory; }

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

    /** @brief 固定事件随机数种子，用于复现随机检定结果 */
    void setRandomSeed(unsigned int seed);

    /** @brief 供 Debug/QA 面板读取当前事件池与触发状态 */
    std::vector<DebugEventInfo> debugEvents(GameContext& ctx);

    /** @brief 清空 Debug/QA 事件运行记录 */
    void clearDebugHistory();

private:
    /** @brief 递归跳转到目标节点，自动解析 RANDOM_CHECK/CHECK */
    void transitionTo(const std::string& nodeId, GameContext& ctx);

    /** @brief 执行节点的附属效果（delta + time_advance + flash），返回 true 表示触发结算中断 */
    bool applyEffects(const EventNode& node, GameContext& ctx);

    /** @brief 解析 RANDOM_CHECK 节点（掷骰 + 跳转），供 transitionTo/handleInput 共用 */
    void resolveRandomCheck(const EventNode& node, GameContext& ctx);

    /** @brief 解析 CHECK 节点（条件求值 + 跳转），供 transitionTo/handleInput 共用 */
    void resolveCheck(const EventNode& node, GameContext& ctx);

    /** @brief 添加一条 Debug/QA 事件运行记录 */
    void appendDebugHistory(const std::string& message);

    /** @brief 求值 conditions 数组 */
    bool evaluateConditions(const std::vector<Condition>& conditions,
                            const std::string& requireMode, GameContext& ctx);

    /** @brief 求值单条 condition */
    bool evaluateCondition(const Condition& cond, GameContext& ctx);

    /** @brief 通过 mCurrentEventId 安全查找当前事件定义（避免裸指针悬垂） */
    const EventDefinition* currentEventDef() const;

    std::unordered_map<std::string, EventDefinition> mEvents;
    std::vector<std::string> mEventOrder;
    std::string mCurrentEventId;
    std::string mCurrentNodeId;
    bool mActive = false;
    bool mWaitingForEnter = false; ///< 等待玩家按 Enter 后再解析
    std::mt19937 mRng;
    std::vector<std::string> mDebugHistory;
};

#endif
