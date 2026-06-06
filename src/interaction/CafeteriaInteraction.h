#ifndef CLS_INTERACTION_CAFETERIAINTERACTION_H
#define CLS_INTERACTION_CAFETERIAINTERACTION_H

#include <string>

struct GameContext;

namespace CafeteriaInteraction {

/**
 * @brief 处理食堂交互（柜台取餐 + 餐桌用餐 + 选项确认）
 */
bool handleInteraction(GameContext& ctx, const std::string& actionId, const std::string& label);

/**
 * @brief 玩家选择了餐食选项后的结算逻辑
 */
void resolveMealChoice(GameContext& ctx, int mealIndex);

} // namespace CafeteriaInteraction

#endif
