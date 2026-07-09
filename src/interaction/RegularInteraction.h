#ifndef CLS_INTERACTION_REGULARINTERACTION_H
#define CLS_INTERACTION_REGULARINTERACTION_H

#include <string>

struct GameContext;
struct InteractionPoint;

namespace RegularInteraction {

/**
 * @brief 处理非事件类的常规交互结算
 */
bool handle(GameContext& ctx, const InteractionPoint& ip);
bool handleDormitoryDeskStudy(GameContext& ctx, const std::string& actionId);
bool handleDormitoryGames(GameContext& ctx, const std::string& actionId);

}

#endif // CLS_INTERACTION_REGULARINTERACTION_H
