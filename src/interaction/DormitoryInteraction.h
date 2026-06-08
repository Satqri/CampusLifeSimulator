#ifndef CLS_INTERACTION_DORMITORYINTERACTION_H
#define CLS_INTERACTION_DORMITORYINTERACTION_H

struct GameContext;
struct InteractionPoint;

namespace DormitoryInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip);

} // namespace DormitoryInteraction

#endif
