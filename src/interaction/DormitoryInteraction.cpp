#include "interaction/DormitoryInteraction.h"
#include "core/GameContext.h"
#include "core/Localization.h"

namespace DormitoryInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip) {
    if (ip.actionId == "dormitory_bed") {
        if (ctx.timeSystem.canSleep()) {
            ctx.sleepFromDormitory();
        } else {
            ctx.runTimedActivity(30, Attributes{.energy = 15, .san = 5},
                cls::text("notice.quiet_moment"),
                cls::text("activity.dormitory_bed.rest"));
        }
        return true;
    }
    return false;
}

} // namespace DormitoryInteraction
