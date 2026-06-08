#ifndef CLS_INTERACTION_GYMINTERACTION_H
#define CLS_INTERACTION_GYMINTERACTION_H

struct GameContext;
struct InteractionPoint;

namespace GymInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip);

} // namespace GymInteraction

#endif
