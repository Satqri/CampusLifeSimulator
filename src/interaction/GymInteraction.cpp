#include "interaction/GymInteraction.h"
#include "core/GameContext.h"
#include <sstream>

namespace GymInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip) {
    const std::string& id = ip.actionId;

    if (id.rfind("gym_treadmill_", 0) == 0) {
        std::ostringstream body;
        body << ip.label << " for 40 minutes. Energy spent, muscles warmed.";
        ctx.runTimedActivity(40, Attributes{.energy = -14, .san = -4, .social = 2},
                             "Training Complete", body.str());
        return true;
    }

    if (id.rfind("gym_barbell_", 0) == 0) {
        std::ostringstream body;
        body << ip.label << " for 40 minutes. Strength training complete.";
        ctx.runTimedActivity(40, Attributes{.energy = -16, .san = -5, .social = 1},
                             "Training Complete", body.str());
        return true;
    }

    return false;
}

} // namespace GymInteraction
