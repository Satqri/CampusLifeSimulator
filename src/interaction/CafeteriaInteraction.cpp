#include "interaction/CafeteriaInteraction.h"
#include "core/GameContext.h"
#include "core/MealConfig.h"
#include <sstream>

namespace CafeteriaInteraction {

bool handleInteraction(GameContext& ctx, const std::string& actionId,
                       const std::string& label) {
    if (actionId == "cafeteria_counter") {
        if (!ctx.timeSystem.isMealTime()) {
            ctx.activityNotice.show("Meal Time Closed",
                "Food is available from 12:00-14:00 and 17:00-19:00.");
            return true;
        }

        int slotId = ctx.timeSystem.mealSlotId();
        if (slotId == ctx.lastMealPickupSlot) {
            ctx.activityNotice.show("Already Picked Up",
                "You already picked up food during this meal slot. Come back for the next one.");
            return true;
        }

        std::ostringstream body;
        body << label << "\n\n"
             << "[1] " << ctx.mealOptions[0].description << "\n"
             << "[2] " << ctx.mealOptions[1].description << "\n"
             << "[3] " << ctx.mealOptions[2].description;
        ctx.mealChoicePrompt.show("Pick a Meal", body.str(),
                                  "Meal A", "Meal B", "Meal C");
        return true;
    }

    if (actionId.rfind("cafeteria_table_", 0) == 0) {
        ctx.activityNotice.show("Cafeteria Table",
            "A clean table. Grab a meal at the counter first.");
        return true;
    }

    return false;
}

void resolveMealChoice(GameContext& ctx, int mealIndex) {
    ctx.mealChoicePrompt.clear();
    if (mealIndex < 0 || mealIndex >= static_cast<int>(ctx.mealOptions.size()))
        return;

    if (!ctx.timeSystem.isMealTime()) {
        ctx.activityNotice.show("Meal Time Closed",
            "Food is available from 12:00-14:00 and 17:00-19:00.");
        return;
    }

    int slotId = ctx.timeSystem.mealSlotId();
    if (slotId == ctx.lastMealPickupSlot) {
        ctx.activityNotice.show("Already Picked Up",
            "You already picked up food during this meal slot.");
        return;
    }

    const MealOption& meal = ctx.mealOptions[mealIndex];
    if (ctx.player.getAttributes().gold < meal.cost) {
        ctx.activityNotice.show("Not Enough Gold",
            "You don't have enough gold for this meal. Try a cheaper option.");
        return;
    }

    ctx.lastMealPickupSlot = slotId;

    std::ostringstream body;
    body << "You enjoy " << meal.name << " (Gold -" << meal.cost
         << "). A warm meal restores some SAN and Energy.";
    ctx.runTimedActivity(20, meal.reward, "Meal Complete", body.str());
}

} // namespace CafeteriaInteraction
