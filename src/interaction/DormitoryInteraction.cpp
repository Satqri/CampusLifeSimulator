#include "interaction/DormitoryInteraction.h"
#include "core/GameContext.h"

namespace DormitoryInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip) {
    if (ip.actionId == "dormitory_bed") {
        ctx.sleepFromDormitory();
        return true;
    }
    return false;
}

} // namespace DormitoryInteraction
