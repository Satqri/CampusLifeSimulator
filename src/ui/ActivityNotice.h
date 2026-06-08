#ifndef CLS_UI_ACTIVITYNOTICE_H
#define CLS_UI_ACTIVITYNOTICE_H

#include <string>

/**
 * @struct ActivityNotice
 * @brief 活动通知弹窗状态 — 标题 + 正文 + Enter 继续
 */
struct ActivityNotice {
    bool active = false;
    std::string title;
    std::string body;

    void show(const std::string& heading, const std::string& message) {
        active = true;
        title = heading;
        body = message;
    }

    void clear() { active = false; title.clear(); body.clear(); }
};

#endif
