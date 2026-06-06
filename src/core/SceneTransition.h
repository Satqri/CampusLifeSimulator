#ifndef CLS_CORE_SCENETRANSITION_H
#define CLS_CORE_SCENETRANSITION_H

#include <string>
#include "ui/SceneBackground.h"

/**
 * @struct SceneTransition
 * @brief 场景过渡状态 — 控制淡入过渡动画的计时和内容
 */
struct SceneTransition {
    bool active = false;
    float timer = 0.0f;
    SceneBackgroundType background = SceneBackgroundType::Dormitory;
    std::string title;
    std::string subtitle;

    void start(SceneBackgroundType bg, const std::string& heading, const std::string& line) {
        active = true;
        timer = 0.0f;
        background = bg;
        title = heading;
        subtitle = line;
    }

    void update(float dt) {
        if (!active) return;
        timer += dt;
    }

    bool canContinue() const {
        return timer >= 0.45f;
    }

    void skip() {
        active = false;
        timer = 0.0f;
    }
};

#endif
