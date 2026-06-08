#ifndef CLS_CORE_COMBATRESULT_H
#define CLS_CORE_COMBATRESULT_H

#include <string>

/**
 * @struct CombatResult
 * @brief 战斗结果状态 — 追踪 d20 掷骰结果和显示计时器
 */
struct CombatResult {
    bool active = false;
    bool victory = false;
    float timer = 0.0f;
    std::string enemyName;
    int d20Roll = 0;
    int modifier = 0;
    int total = 0;
    int dc = 0;

    void show(bool win, const std::string& name, int d20, int mod, int tot, int d) {
        active = true; victory = win; timer = 3.0f;
        enemyName = name; d20Roll = d20; modifier = mod; total = tot; dc = d;
    }
    void update(float dt) {
        if (active) { timer -= dt; if (timer <= 0.0f) active = false; }
    }
    void clear() { active = false; timer = 0.0f; }
};

#endif
