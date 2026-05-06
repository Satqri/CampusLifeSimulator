#ifndef SLS_CORE_TYPES_H
#define SLS_CORE_TYPES_H

/**
 * @struct Attributes
 * @brief 角色属性数据，值域 0-100（金钱 0-9999）
 *
 * 用于 Character 及其派生类，存储角色的核心属性值
 */
struct Attributes {
    int san;      ///< 理智 (Sanity) — 核心属性，过低触发战斗
    int energy;   ///< 体力
    int academic; ///< 学业
    int social;   ///< 社交
    int gold;     ///< 金钱

    Attributes() : san(80), energy(80), academic(60), social(60), gold(100) {}
    Attributes(int s, int e, int a, int so, int g)
        : san(s), energy(e), academic(a), social(so), gold(g) {}
};

/**
 * @enum EmotionType
 * @brief 敌人情绪类型枚举
 *
 * 不同情绪对应不同的战斗对抗方式
 */
enum class EmotionType {
    ANXIETY,     ///< 焦虑 — 对抗方式: 理性分析
    DEPRESSION,  ///< 抑郁 — 对抗方式: 意志坚守
    ANGER,       ///< 愤怒 — 对抗方式: 发泄释放
    FEAR,        ///< 恐惧 — 对抗方式: 意志坚守
    LONELINESS   ///< 孤独 — 对抗方式: 倾诉求助
};

#endif // SLS_CORE_TYPES_H
