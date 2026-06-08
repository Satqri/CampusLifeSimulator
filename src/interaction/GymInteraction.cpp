#include "interaction/GymInteraction.h"
#include "core/GameContext.h"
#include <sstream>

namespace GymInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip) {
    const std::string& id = ip.actionId;

    if (id.rfind("gym_treadmill_", 0) == 0) {
        std::ostringstream body;
        body << ip.label << " for 40 minutes. Energy spent, muscles warmed.";
        ctx.runTimedActivity(40, Attributes(-4, -14, 0, 2, 0),
                             "Training Complete", body.str());
        return true;
    }

    if (id.rfind("gym_barbell_", 0) == 0) {
        std::ostringstream body;
        body << ip.label << " for 40 minutes. Strength training complete.";
        ctx.runTimedActivity(40, Attributes(-5, -16, 0, 1, 0),
                             "Training Complete", body.str());
        return true;
    }

    return false;
}

} // namespace GymInteraction
