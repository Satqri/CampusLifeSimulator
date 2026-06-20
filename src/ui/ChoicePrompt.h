#ifndef CLS_UI_CHOICEPROMPT_H
#define CLS_UI_CHOICEPROMPT_H

#include <string>
#include <vector>

/**
 * @struct ChoicePrompt
 * @brief 选项选择弹窗状态 — 供玩家按下数字键选择
 */
struct ChoicePrompt {
    bool active = false;
    std::string title;
    std::string body;
    std::string first;
    std::string second;
    std::string third;
    std::string fourth;
    std::string purpose;
    std::vector<int> values;
    int selectedValue = 0;
    int minValue = 0;
    int maxValue = 0;
    int stepValue = 0;

    void show(const std::string& heading, const std::string& message,
              const std::string& optionA, const std::string& optionB,
              const std::string& optionC = "") {
        active = true;
        title = heading;
        body = message;
        first = optionA;
        second = optionB;
        third = optionC;
        fourth.clear();
        purpose.clear();
        values.clear();
        selectedValue = 0;
        minValue = 0;
        maxValue = 0;
        stepValue = 0;
    }

    void show(const std::string& heading, const std::string& message,
              const std::vector<std::string>& options,
              const std::string& promptPurpose = "",
              const std::vector<int>& optionValues = {}) {
        active = true;
        title = heading;
        body = message;
        first = options.size() > 0 ? options[0] : "";
        second = options.size() > 1 ? options[1] : "";
        third = options.size() > 2 ? options[2] : "";
        fourth = options.size() > 3 ? options[3] : "";
        purpose = promptPurpose;
        values = optionValues;
        selectedValue = optionValues.empty() ? 0 : optionValues.front();
        minValue = 0;
        maxValue = 0;
        stepValue = 0;
    }

    void showRange(const std::string& heading, const std::string& message,
                   const std::string& promptPurpose,
                   int initialValue, int minAllowed, int maxAllowed, int step) {
        active = true;
        title = heading;
        body = message;
        first.clear();
        second.clear();
        third.clear();
        fourth.clear();
        purpose = promptPurpose;
        values.clear();
        selectedValue = initialValue;
        minValue = minAllowed;
        maxValue = maxAllowed;
        stepValue = step;
    }

    void clear() {
        active = false;
        title.clear();
        body.clear();
        first.clear();
        second.clear();
        third.clear();
        fourth.clear();
        purpose.clear();
        values.clear();
        selectedValue = 0;
        minValue = 0;
        maxValue = 0;
        stepValue = 0;
    }
};

#endif
