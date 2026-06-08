#ifndef CLS_EVENT_RANDOMEVENT_H
#define CLS_EVENT_RANDOMEVENT_H

#include "core/Types.h"

#include <string>
#include <vector>

/**
 * @struct RandomEventDefinition
 * @brief 本地随机事件定义
 */
struct RandomEventDefinition {
    std::string id;
    std::string title;
    std::string body;
    std::vector<std::string> options;
    std::vector<Attributes> deltas;
    bool llmHookSuggested = false;
};

/**
 * @class RandomEventLibrary
 * @brief 随机事件库，提供本地事件与未来 LLM 回退提示
 */
class RandomEventLibrary {
public:
    /** @brief 构造默认本地事件库 */
    RandomEventLibrary();

    /** @brief 获取全部本地事件 */
    const std::vector<RandomEventDefinition>& events() const;

    /** @brief 获取未来 LLM 事件的占位提示 */
    const std::string& llmFallbackHint() const;

private:
    std::vector<RandomEventDefinition> localEvents;
    std::string llmHint;
};

#endif // CLS_EVENT_RANDOMEVENT_H
