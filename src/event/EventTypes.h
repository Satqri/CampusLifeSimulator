#ifndef CLS_EVENT_EVENTTYPES_H
#define CLS_EVENT_EVENTTYPES_H

#include "core/Types.h"
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

/** @brief 事件节点类型 */
enum class EventNodeType {
    DISPLAY,
    CHOICE,
    RANDOM_CHECK,
    CHECK,
    OUTCOME
};

/** @brief 条件判定类型 */
enum class ConditionKind {
    LOCATION,
    STAT,
    FLAG,
    GROUP,
    META,
    ENUM,
    TIME
};

/** @brief 单条条件 — 用于 CHECK 节点 */
struct Condition {
    ConditionKind kind = ConditionKind::LOCATION;
    std::string place;   ///< LOCATION: classroom/dormitory/cafeteria/gym/library/campus/store
    std::string stat;    ///< STAT/ENUM: visible attr or hidden key; TIME: phase/day/minute/meal_time
    std::string op;      ///< >= <= < > eq neq
    int value = 0;       ///< 兼容旧整数阈值
    std::string flag;    ///< FLAG: "is_midterm_day" 等
    std::string requireMode;              ///< GROUP: all/any
    std::vector<Condition> conditions;    ///< GROUP: 嵌套条件
    nlohmann::json valueJson;             ///< 原始比较值（int/bool/string）
};

/** @brief CHOICE 节点的单条选项 */
struct ChoiceOption {
    std::string text;     ///< 选项显示文本
    std::string nextNode; ///< 跳转目标节点 ID
};

/** @brief 事件触发器 */
struct EventTrigger {
    enum Type { NONE, TIME_SCHEDULE, INTERACTION };
    Type type = NONE;
    std::string method;   ///< TIME_SCHEDULE: "crossed_class_time"
    std::string actionId; ///< INTERACTION: 匹配 actionId
    std::vector<Condition> conditions; ///< 触发前置条件
    std::string requireMode = "all";
    int chance = 100;              ///< 触发概率，0-100，默认必定触发
    bool once = false;             ///< true 时每局只触发一次
    int cooldownDays = 0;          ///< 成功触发后隔几天才能再次触发
    bool fallbackToRegular = false;///< 不中概率/冷却时允许常规交互继续执行
};

/** @brief 事件图中的一个节点 */
struct EventNode {
    EventNodeType type = EventNodeType::DISPLAY;

    /// 通用显示字段
    std::string title;
    std::string body;
    std::string footer;

    /// CHOICE 节点
    std::vector<ChoiceOption> options;

    /// DISPLAY 节点
    std::string thenNode;

    /// RANDOM_CHECK 节点
    int probability = 50;
    std::string successNode;
    std::string failureNode;

    /// CHECK 节点
    std::vector<Condition> conditions;
    std::string requireMode; ///< "all"（AND）或 "any"（OR），默认 "all"
    std::string elseNode;    ///< CHECK 失败跳转

    /// OUTCOME / 附属效果（所有非 DISPLAY/CHOICE 节点都可带效果）
    Attributes delta = {};
    nlohmann::json hiddenDelta;           ///< 隐藏变量变化（所有节点类型通用）
    int timeAdvanceMinutes = 0;
    std::string flashText;
};

/** @brief 一个完整的事件定义 */
struct EventDefinition {
    std::string id;
    EventTrigger trigger;
    std::string rootNode;
    std::unordered_map<std::string, EventNode> steps;
};

#endif
