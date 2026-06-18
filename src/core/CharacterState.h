#ifndef CLS_CORE_CHARACTERSTATE_H
#define CLS_CORE_CHARACTERSTATE_H

#include <nlohmann/json.hpp>

/**
 * @struct Attributes
 * @brief 可见角色属性数据
 *
 * 体力/健康/压力/知识/社交 值域 0-100，金钱 0-9999。
 * 类内初始值即游戏起始属性，难度调整在 applyDifficulty() 中叠加。
 */
struct Attributes {
    int energy   = 80;  ///< 体力
    int health   = 80;  ///< 健康
    int gold     = 100; ///< 金钱
    int san      = 80;  ///< 压力 / SAN 值 — 过低触发战斗
    int academic = 60;  ///< 知识
    int social   = 60;  ///< 社交
};

/**
 * @brief 隐藏变量类型 — 存储不影响六维属性的叙事累积状态
 *
 * 支持 int / bool / string 三种值类型，用于结局判定和条件分支。
 * 合并规则：数值累加（int），布尔/字符串覆盖。
 *
 * 常用 key（详见 events/数据说明.md）:
 *   int:  teacherTrust, friendBond, innovationProgress, lateNightLevel, ...
 *   bool: owedFavor, innovationJoined, sharedNotes, ...
 *   str:  clubType, innovationTopic, innovationDemoMode, ...
 */
using HiddenMap = nlohmann::json;

/** @brief 将 JSON hidden_delta 合并到现有 HiddenMap 中 */
inline void mergeHidden(HiddenMap& target, const HiddenMap& delta) {
    if (delta.is_null()) return;
    for (auto it = delta.begin(); it != delta.end(); ++it) {
        if (it.value().is_number_integer()) {
            int cur = target.value(it.key(), 0);
            int add = it.value();
            target[it.key()] = cur + add;
        } else {
            target[it.key()] = it.value();  // bool / string 直接覆盖
        }
    }
}

#endif
