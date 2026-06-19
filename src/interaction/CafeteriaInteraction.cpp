#include "interaction/CafeteriaInteraction.h"
#include "core/GameContext.h"
#include "core/CharacterState.h"
#include "core/MealConfig.h"
#include "core/Localization.h"
#include <sstream>

namespace CafeteriaInteraction {

bool handleInteraction(GameContext& ctx, const std::string& actionId,
                       const std::string& label) {
    if (actionId == "cafeteria_counter") {
        if (!ctx.timeSystem.isMealTime()) {
            ctx.activityNotice.show(cls::text("notice.meal_closed"),
                cls::text("activity.cafeteria.meal_time_closed"));
            return true;
        }

        int slotId = ctx.timeSystem.mealSlotId();
        if (slotId == ctx.lastMealPickupSlot) {
            ctx.activityNotice.show(cls::text("notice.already_served"),
                cls::text("activity.cafeteria.already_picked_up"));
            return true;
        }

        ctx.lastMealPickupSlot = slotId;
        ctx.activityNotice.show(label,
            cls::text("activity.cafeteria.food_taken"));
        return true;
    }

    if (actionId.rfind("cafeteria_table_", 0) == 0) {
        const int slotId = ctx.timeSystem.mealSlotId();
        if (!ctx.timeSystem.isMealTime()) {
            ctx.activityNotice.show(cls::text("notice.meal_closed"),
                cls::text("activity.cafeteria.table_closed"));
            return true;
        }
        if (slotId != ctx.lastMealPickupSlot) {
            ctx.activityNotice.show(cls::text("interaction.cafeteria_table.label"),
                cls::text("activity.cafeteria.no_food_at_table"));
            return true;
        }

        Attributes reward{.energy = 18, .san = -6, .social = 1};
        ctx.runTimedActivity(25, reward, cls::text("notice.meal_complete"),
            cls::text("activity.cafeteria.eat_at_table"));
        mergeHidden(ctx.player.getHidden(), HiddenMap{{"mealCount", 1}});
        ctx.lastMealPickupSlot = -1;
        return true;
    }

    return false;
}

void resolveMealChoice(GameContext& ctx, int mealIndex) {
    ctx.mealChoicePrompt.clear();
    if (mealIndex < 0 || mealIndex >= static_cast<int>(ctx.mealOptions.size()))
        return;

    if (!ctx.timeSystem.isMealTime()) {
        ctx.activityNotice.show(cls::text("notice.meal_closed"),
            cls::text("activity.cafeteria.meal_time_closed"));
        return;
    }

    int slotId = ctx.timeSystem.mealSlotId();
    if (slotId == ctx.lastMealPickupSlot) {
        ctx.activityNotice.show(cls::text("notice.already_served"),
            cls::text("activity.cafeteria.already_picked_up_short"));
        return;
    }

    const MealOption& meal = ctx.mealOptions[mealIndex];
    if (ctx.player.getAttributes().gold < meal.cost) {
        ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
            cls::text("activity.cafeteria.not_enough_gold"));
        return;
    }

    ctx.lastMealPickupSlot = slotId;

    std::ostringstream body;
    body << cls::format("activity.cafeteria.meal_choice_result",
        {{"meal", meal.displayName()}, {"cost", std::to_string(meal.cost)}});
    ctx.runTimedActivity(20, meal.reward, cls::text("notice.meal_complete"), body.str());
}

} // namespace CafeteriaInteraction
