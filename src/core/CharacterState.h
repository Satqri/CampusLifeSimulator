#ifndef CLS_CORE_CHARACTERSTATE_H
#define CLS_CORE_CHARACTERSTATE_H

#include "core/HiddenVariableConfig.h"
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

// ── 全局配置访问 ──────────────────────────────────────────

namespace cls_hidden_detail {
    inline const HiddenVariableConfig* gHiddenConfig = nullptr;
}

/** @brief 初始化隐藏变量配置（main() 启动时调用一次） */
inline void initHiddenVariableConfig(const HiddenVariableConfig& config) {
    cls_hidden_detail::gHiddenConfig = &config;
}

/** @brief 获取隐藏变量配置（内部使用） */
inline const HiddenVariableConfig& getHiddenConfig() {
    return *cls_hidden_detail::gHiddenConfig;
}

/** @brief 用配置中的初始值初始化 HiddenMap */
inline void initializeHiddenState(HiddenMap& target) {
    cls_hidden_detail::gHiddenConfig->initialize(target);
}

// ── 基本查询（委托到 JSON 配置）──────────────────────────

inline bool isHiddenAssignmentKey(const std::string& key) {
    return getHiddenConfig().isAssignment(key);
}

inline int clampHiddenInteger(const std::string& key, int value) {
    return getHiddenConfig().clamp(key, value);
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

// ── 重复活动缩放 ───────────────────────────────────────

inline int repeatedBenefitPercent(int streak) {
    return std::max(40, 100 - (streak - 1) * 20);
}

inline int repeatedPenaltyPercent(int streak) {
    return std::min(200, 100 + (streak - 1) * 25);
}

inline int scalePositiveBenefit(int value, int streak) {
    if (value <= 0 || streak <= 1) return value;
    return (value * repeatedBenefitPercent(streak) + 50) / 100;
}

inline int scalePenaltyMagnitude(int value, int streak) {
    if (value <= 0 || streak <= 1) return value;
    return (value * repeatedPenaltyPercent(streak) + 99) / 100;
}

inline int scaleNormalDelta(int value, int streak) {
    if (value > 0) return scalePositiveBenefit(value, streak);
    if (value < 0) return -scalePenaltyMagnitude(-value, streak);
    return 0;
}

inline int scaleBadAccumulationDelta(int value, int streak) {
    if (value > 0) return scalePenaltyMagnitude(value, streak);
    if (value < 0) return -scalePositiveBenefit(-value, streak);
    return 0;
}

inline bool isScalableHiddenBenefit(const std::string& key) {
    return getHiddenConfig().scalingCategory(key) == HiddenVarScaling::Benefit;
}

inline bool isScalableHiddenBurden(const std::string& key) {
    return getHiddenConfig().scalingCategory(key) == HiddenVarScaling::Burden;
}

inline Attributes adjustAttributesForRepetition(const Attributes& delta, int streak) {
    if (streak <= 1) return delta;

    Attributes adjusted = delta;
    adjusted.energy = scaleNormalDelta(delta.energy, streak);
    adjusted.health = scaleNormalDelta(delta.health, streak);
    adjusted.gold = delta.gold > 0 ? scalePositiveBenefit(delta.gold, streak) : delta.gold;
    adjusted.san = scaleNormalDelta(delta.san, streak);
    adjusted.academic = scaleNormalDelta(delta.academic, streak);
    adjusted.social = scaleNormalDelta(delta.social, streak);
    return adjusted;
}

inline HiddenMap adjustHiddenForRepetition(const HiddenMap& hiddenDelta, int streak) {
    if (streak <= 1 || !hiddenDelta.is_object()) return hiddenDelta;

    HiddenMap adjusted = hiddenDelta;
    for (auto it = adjusted.begin(); it != adjusted.end(); ++it) {
        if (!it.value().is_number_integer()) continue;

        const std::string key = it.key();
        const int value = it.value().get<int>();
        if (isScalableHiddenBenefit(key)) {
            it.value() = scaleNormalDelta(value, streak);
        } else if (isScalableHiddenBurden(key)) {
            it.value() = scaleBadAccumulationDelta(value, streak);
        }
    }
    return adjusted;
}

// ── 时长缩放 ──────────────────────────────────────────

inline int scaledIntegerDelta(int value, int actualMinutes, int baseMinutes) {
    if (value == 0 || actualMinutes <= 0 || baseMinutes <= 0) return 0;
    const int sign = value < 0 ? -1 : 1;
    const int magnitude = std::abs(value);
    return sign * ((magnitude * actualMinutes + baseMinutes / 2) / baseMinutes);
}

inline Attributes scaleAttributesByDuration(const Attributes& delta, int actualMinutes, int baseMinutes) {
    Attributes scaled = delta;
    scaled.energy = scaledIntegerDelta(delta.energy, actualMinutes, baseMinutes);
    scaled.health = scaledIntegerDelta(delta.health, actualMinutes, baseMinutes);
    scaled.gold = scaledIntegerDelta(delta.gold, actualMinutes, baseMinutes);
    scaled.san = scaledIntegerDelta(delta.san, actualMinutes, baseMinutes);
    scaled.academic = scaledIntegerDelta(delta.academic, actualMinutes, baseMinutes);
    scaled.social = scaledIntegerDelta(delta.social, actualMinutes, baseMinutes);
    return scaled;
}

inline bool isDurationScaledHiddenKey(const std::string& key) {
    auto cat = getHiddenConfig().scalingCategory(key);
    return cat == HiddenVarScaling::Benefit || cat == HiddenVarScaling::Burden;
}

inline HiddenMap scaleHiddenByDuration(const HiddenMap& hiddenDelta, int actualMinutes, int baseMinutes) {
    if (!hiddenDelta.is_object() || actualMinutes <= 0 || baseMinutes <= 0) return hiddenDelta;

    HiddenMap scaled = hiddenDelta;
    for (auto it = scaled.begin(); it != scaled.end(); ++it) {
        if (it.value().is_number_integer() && isDurationScaledHiddenKey(it.key())) {
            it.value() = scaledIntegerDelta(it.value().get<int>(), actualMinutes, baseMinutes);
        }
    }
    return scaled;
}

#endif
