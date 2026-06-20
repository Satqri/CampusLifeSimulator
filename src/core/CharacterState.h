#ifndef CLS_CORE_CHARACTERSTATE_H
#define CLS_CORE_CHARACTERSTATE_H

#include <nlohmann/json.hpp>
#include <algorithm>
#include <string>

/**
 * @struct Attributes
 * @brief 可见角色属性数据
 *
 * 体力/健康/SAN/知识/社交值域 0-100，金钱 0-9999。
 * 本结构既用于完整属性快照，也用于变化量 delta；因此成员默认值必须为 0。
 * 玩家初始值由 defaultPlayerAttributes() 明确提供。
 */
struct Attributes {
    int energy   = 0; ///< 体力
    int health   = 0; ///< 身体状态，显示 healthIndex 的 0-100 截断值
    int gold     = 0; ///< 金钱
    int san      = 0; ///< SAN / 心态稳定度，越高越好；stress = 100 - san
    int academic = 0; ///< 知识
    int social   = 0; ///< 社交
};

inline Attributes defaultPlayerAttributes() {
    return Attributes{
        .energy = 78,
        .health = 100,
        .gold = 100,
        .san = 76,
        .academic = 42,
        .social = 42
    };
}

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

inline bool isHiddenAssignmentKey(const std::string& key) {
    if (key == "friendStage" || key == "clubStage" || key == "innovationStage")
        return true;
    if (key == "lastSleepMinutes" || key == "alarmSleepMinutes"
        || key == "consecutiveNoSleepDays" || key == "lastSleepDay"
        || key == "dailyExerciseDay" || key == "dailyExerciseCount"
        || key == "lastExerciseAbsoluteMinute") {
        return true;
    }
    if (key.rfind("_event_", 0) == 0 && key.size() >= 9
        && key.ends_with("_last_day"))
        return true;
    return false;
}

inline int clampHiddenInteger(const std::string& key, int value) {
    if (key == "healthIndex") return std::clamp(value, 0, 130);
    if (key == "innovationProgress") return std::clamp(value, 0, 100);
    if (key == "innovationDefenseScore") return std::clamp(value, -30, 100);
    if (key == "innovationTeamTrust") return std::clamp(value, -20, 60);
    if (key == "teacherTrust") return std::clamp(value, -20, 40);
    if (key == "friendBond") return std::clamp(value, -10, 20);
    if (key == "storeTrust") return std::clamp(value, -10, 15);
    if (key == "clubContribution" || key == "clubRelation") return std::clamp(value, -10, 25);
    if (key == "clubShowcaseScore") return std::clamp(value, -20, 40);
    if (key == "lateNightLevel" || key == "gameAddiction") return std::clamp(value, 0, 60);
    if (key == "lowEnergyDays" || key == "lowHealthDays") return std::clamp(value, 0, 14);
    if (key == "consecutiveNoSleepDays") return std::clamp(value, 0, 14);
    if (key == "lastSleepMinutes" || key == "alarmSleepMinutes") return std::clamp(value, 0, 12 * 60);
    if (key == "lastSleepDay" || key == "dailyExerciseDay") return std::clamp(value, 0, 14);
    if (key == "dailyExerciseCount") return std::clamp(value, 0, 12);
    if (key == "lastExerciseAbsoluteMinute") return std::clamp(value, -999999, 999999);
    if (key == "activityStreak") return std::clamp(value, 0, 99);
    if (key == "friendStage" || key == "clubStage") return std::clamp(value, -1, 4);
    if (key == "innovationStage") return std::clamp(value, -1, 5);
    if (key.rfind("_event_", 0) == 0 && key.ends_with("_count")) return std::clamp(value, 0, 99);
    if (key.rfind("_event_", 0) == 0 && key.ends_with("_last_day")) return std::clamp(value, -1, 14);

    static const std::string countSuffix = "Count";
    if (key.size() >= countSuffix.size()
        && key.compare(key.size() - countSuffix.size(), countSuffix.size(), countSuffix) == 0) {
        return std::clamp(value, 0, 99);
    }
    return value;
}

inline void normalizeHidden(HiddenMap& target) {
    if (!target.is_object()) target = HiddenMap::object();
    for (auto it = target.begin(); it != target.end(); ++it) {
        if (it.value().is_number_integer()) {
            it.value() = clampHiddenInteger(it.key(), it.value().get<int>());
        }
    }
}

/** @brief 将 JSON hidden_delta 合并到现有 HiddenMap 中 */
inline void mergeHidden(HiddenMap& target, const HiddenMap& delta) {
    if (!target.is_object()) target = HiddenMap::object();
    if (delta.is_null()) return;
    for (auto it = delta.begin(); it != delta.end(); ++it) {
        if (it.value().is_number_integer()) {
            if (isHiddenAssignmentKey(it.key())) {
                target[it.key()] = it.value();
            } else {
                int cur = target.value(it.key(), 0);
                int add = it.value();
                target[it.key()] = cur + add;
            }
        } else {
            target[it.key()] = it.value();  // bool / string 直接覆盖
        }
    }
    normalizeHidden(target);
}

inline void syncVisibleHealthFromHidden(Attributes& attributes, HiddenMap& hidden) {
    if (!hidden.is_object()) hidden = HiddenMap::object();
    if (!hidden.contains("healthIndex") || !hidden["healthIndex"].is_number_integer()) {
        hidden["healthIndex"] = attributes.health > 0 ? attributes.health : 100;
    }
    normalizeHidden(hidden);
    attributes.health = std::clamp(hidden.value("healthIndex", 100), 0, 100);
}

#endif
