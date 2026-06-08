#ifndef CLS_UI_CHOICEPROMPT_H
#define CLS_UI_CHOICEPROMPT_H

#include <string>

/**
 * @struct ChoicePrompt
 * @brief 选项选择弹窗状态 — 2-3 个选项供玩家按下数字键选择
 */
struct ChoicePrompt {
    bool active = false;
    std::string title;
    std::string body;
    std::string first;
    std::string second;
    std::string third;

    void show(const std::string& heading, const std::string& message,
              const std::string& optionA, const std::string& optionB,
              const std::string& optionC = "") {
        active = true;
        title = heading;
        body = message;
        first = optionA;
        second = optionB;
        third = optionC;
    }

    void clear() { active = false; title.clear(); body.clear(); first.clear(); second.clear(); third.clear(); }
};

#endif
