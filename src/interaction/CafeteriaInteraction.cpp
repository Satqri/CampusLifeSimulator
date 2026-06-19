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

        ctx.lastMealPickupSlot = slotId;
        ctx.activityNotice.show(label,
            "你在窗口打好当前时段的餐食。具体吃什么已经不重要，重要的是这顿饭终于有着落了。去餐桌坐下吃完它。");
        return true;
    }

    if (actionId.rfind("cafeteria_table_", 0) == 0) {
        const int slotId = ctx.timeSystem.mealSlotId();
        if (!ctx.timeSystem.isMealTime()) {
            ctx.activityNotice.show("Meal Time Closed",
                "现在不是饭点。餐桌很干净，但你没有什么能吃的。");
            return true;
        }
        if (slotId != ctx.lastMealPickupSlot) {
            ctx.activityNotice.show("Cafeteria Table",
                "餐桌空着。先去窗口打好饭，再坐下来吃。");
            return true;
        }

        Attributes reward{.energy = 18, .san = -6, .social = 1};
        ctx.runTimedActivity(25, reward, cls::text("notice.meal_complete"),
            "你坐下来吃完了这顿饭。身体恢复了一些，时间也悄悄过去了。");
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
