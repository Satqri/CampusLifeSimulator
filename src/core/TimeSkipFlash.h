#ifndef CLS_CORE_TIMESKIPFLASH_H
#define CLS_CORE_TIMESKIPFLASH_H

#include <string>

/**
 * @struct TimeSkipFlash
 * @brief 时间跳过时的全屏黑屏闪烁提示
 */
struct TimeSkipFlash {
    bool active = false;
    float timer = 0.0f;
    std::string text;

    void start(const std::string& message) {
        active = true;
        timer = 0.58f;
        text = message;
    }

    void update(float dt) {
        if (!active) return;
        timer -= dt;
        if (timer <= 0.0f) {
            active = false;
            timer = 0.0f;
        }
    }
};

#endif
