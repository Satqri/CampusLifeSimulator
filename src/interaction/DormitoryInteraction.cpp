#include "interaction/DormitoryInteraction.h"
#include "core/GameContext.h"
#include <sstream>

namespace DormitoryInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip) {
    const std::string& id = ip.actionId;

    if (id == "dormitory_bed") {
        ctx.sleepFromDormitory();
        return true;
    }

    if (id == "dormitory_desk") {
        std::ostringstream body;
        body << ip.label << ". A focused study session sharpens the mind.";
        ctx.runTimedActivity(45, Attributes(-6, -10, 7, 0, 0),
                             "Study Complete", body.str());
        return true;
    }

    if (id == "dormitory_games") {
        if (ctx.gamesPlayedToday >= 3) {
            ctx.activityNotice.show("Games Played Enough",
                "You've already played enough today. Try studying or resting.");
            return true;
        }

        ctx.gamesPlayedToday++;
        if (ctx.gamesPlayedToday <= 2) {
            std::ostringstream body;
            body << ip.label << " for 60 minutes. A good break clears the mind.";
            ctx.runTimedActivity(60, Attributes(12, 8, 0, 0, 0),
                                 "Game Break Complete", body.str());
        } else {
            std::ostringstream body;
            body << ip.label << " again for 60 minutes. Fun but draining — maybe stop here.";
            ctx.runTimedActivity(60, Attributes(4, -12, -2, 0, 0),
                                 "Overplayed", body.str());
        }
        return true;
    }

    if (id == "dormitory_rug") {
        ctx.activityNotice.show("Quiet Moment",
            "A green rug in the middle of the room. A quiet place to sit and gather your thoughts.");
        return true;
    }

    return false;
}

} // namespace DormitoryInteraction
