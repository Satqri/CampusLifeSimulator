#include "interaction/RegularInteraction.h"

#include "core/GameContext.h"
#include "core/CharacterState.h"
#include "core/Localization.h"
#include "core/LibraryConfig.h"

#include <sstream>

namespace {

bool isEveningOrNight(const TimeSystem& timeSystem) {
    const TimePhase phase = timeSystem.currentPhase();
    return phase == TimePhase::Evening || phase == TimePhase::Night;
}

bool isAfternoonOrLater(const TimeSystem& timeSystem) {
    const TimePhase phase = timeSystem.currentPhase();
    return phase == TimePhase::Afternoon || phase == TimePhase::Evening || phase == TimePhase::Night;
}

bool hasJoinedInnovation(GameContext& ctx) {
    const auto& hidden = ctx.player.getHidden();
    return hidden.value("innovationJoined", false)
        || hidden.value("innovationStage", 0) > 0;
}

void runAndMergeHidden(GameContext& ctx, int minutes, const Attributes& delta,
                       const HiddenMap& hiddenDelta,
                       const std::string& title, const std::string& body) {
    ctx.runTimedActivity(minutes, delta, title, body);
    mergeHidden(ctx.player.getHidden(), hiddenDelta);
}

std::string bookSkillKey(const std::string& skill) {
    if (skill == "Research") return "researchUnlocked";
    if (skill == "Logic") return "logicUnlocked";
    if (skill == "Reflection") return "expressionUnlocked";
    if (skill == "Context") return "campusIntelUnlocked";
    return "";
}

} // namespace

namespace RegularInteraction {

bool handle(GameContext& ctx, const InteractionPoint& ip) {
    const std::string& actionId = ip.actionId;

    if (actionId == "dormitory_desk") {
        if (hasJoinedInnovation(ctx) && isEveningOrNight(ctx.timeSystem)) {
            runAndMergeHidden(ctx, 75, Attributes{.energy = -12, .san = 8, .academic = 5},
                HiddenMap{{"innovationProgress", 6}, {"lateNightLevel", 2}},
                cls::text("notice.study_complete"),
                cls::text("activity.dormitory_desk.innovation"));
            return true;
        }

        runAndMergeHidden(ctx, 60, Attributes{.energy = -10, .san = 5, .academic = 8}, HiddenMap::object(),
            cls::text("notice.study_complete"),
            cls::text("activity.dormitory_desk.study"));
        return true;
    }

    if (actionId == "dormitory_games") {
        runAndMergeHidden(ctx, 45, Attributes{.energy = -5, .san = -12, .academic = -2},
            HiddenMap{{"gameAddiction", 5}, {"lateNightLevel", 3}, {"healthIndex", -2}},
            cls::text("notice.game_break_complete"),
            cls::text("activity.dormitory_games"));
        return true;
    }

    if (actionId == "dormitory_rug") {
        runAndMergeHidden(ctx, 20, Attributes{.energy = -3, .san = -8}, HiddenMap{{"lateNightLevel", 2}},
            cls::text("notice.quiet_moment"),
            cls::text("activity.dormitory_rug"));
        return true;
    }

    if (actionId.rfind("library_shelf_", 0) == 0) {
        const int shelfIndex = actionId.back() - '0';
        if (shelfIndex < 0 || shelfIndex >= static_cast<int>(ctx.libraryBooks.size())) return false;

        const LibraryBook& book = ctx.libraryBooks[shelfIndex];
        HiddenMap hiddenDelta{{"libraryVisitCount", 1}};
        const std::string skillKey = bookSkillKey(book.skill);
        if (!skillKey.empty()) hiddenDelta[skillKey] = true;

        std::ostringstream body;
        body << cls::format("activity.library_shelf",
            {{"book", book.displayName()}, {"skill", book.displaySkill()}});
        runAndMergeHidden(ctx, 50, book.delta, hiddenDelta,
            cls::text("notice.reading_complete"), body.str());
        return true;
    }

    if (actionId == "library_table") {
        if (hasJoinedInnovation(ctx) && isAfternoonOrLater(ctx.timeSystem)) {
            runAndMergeHidden(ctx, 70, Attributes{.energy = -10, .san = 6, .academic = 6},
                HiddenMap{{"innovationProgress", 7}, {"innovationTeamTrust", 1}},
                cls::text("notice.study_complete"),
                cls::text("activity.library_table.innovation"));
            return true;
        }

        runAndMergeHidden(ctx, 60, Attributes{.energy = -8, .san = -4, .academic = 6},
            HiddenMap{{"libraryVisitCount", 1}},
            cls::text("notice.reading_complete"),
            cls::text("activity.library_table.reading"));
        return true;
    }

    if (actionId == "classroom_board") {
        runAndMergeHidden(ctx, 20, Attributes{.energy = -2, .academic = 3},
            HiddenMap{{"teacherTrust", 1}},
            cls::text("notice.board_reviewed"),
            cls::text("activity.classroom_board"));
        return true;
    }

    if (actionId.rfind("classroom_desk_", 0) == 0) {
        const bool inClassTime = ctx.timeSystem.getMinuteOfDay() >= TimeSystem::kClassMinute
            && ctx.timeSystem.getMinuteOfDay() < TimeSystem::kClassEndMinute;
        if (inClassTime) {
            runAndMergeHidden(ctx, 45, Attributes{.energy = -10, .academic = 10},
                HiddenMap{{"classAttendCount", 1}, {"teacherTrust", 2}},
                cls::text("notice.class_complete"),
                cls::text("activity.classroom_desk.class_time"));
        } else if (hasJoinedInnovation(ctx) && isAfternoonOrLater(ctx.timeSystem)) {
            runAndMergeHidden(ctx, 60, Attributes{.energy = -8, .san = 4, .social = 3},
                HiddenMap{{"innovationProgress", 4}, {"innovationTeamTrust", 3}},
                cls::text("notice.study_complete"),
                cls::text("activity.classroom_desk.innovation"));
        } else {
            runAndMergeHidden(ctx, 35, Attributes{.energy = -5, .san = -2, .academic = 5}, HiddenMap::object(),
                cls::text("notice.study_complete"),
                cls::text("activity.classroom_desk.study"));
        }
        return true;
    }

    if (actionId.rfind("gym_treadmill_", 0) == 0) {
        runAndMergeHidden(ctx, 45, Attributes{.energy = -10, .san = -8},
            HiddenMap{{"exerciseCount", 1}, {"healthIndex", 5}},
            cls::text("notice.training_complete"),
            cls::text("activity.gym_treadmill"));
        return true;
    }

    if (actionId.rfind("gym_barbell_", 0) == 0) {
        runAndMergeHidden(ctx, 50, Attributes{.energy = -15, .san = -2},
            HiddenMap{{"exerciseCount", 1}, {"healthIndex", 5}},
            cls::text("notice.training_complete"),
            cls::text("activity.gym_barbell"));
        return true;
    }

    if (actionId == "gym_front_desk") {
        if (!isEveningOrNight(ctx.timeSystem)) {
            ctx.activityNotice.show(cls::text("notice.too_early"),
                cls::text("activity.gym_front_desk.too_early"));
            return true;
        }
        runAndMergeHidden(ctx, 75, Attributes{.energy = -16, .gold = 30, .san = 4},
            HiddenMap{{"partTimeCount", 1}, {"healthIndex", -1}},
            cls::text("notice.training_complete"),
            cls::text("activity.gym_front_desk.work"));
        return true;
    }

    if (actionId == "store_shelf") {
        if (ctx.player.getAttributes().gold < 12) {
            ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
                cls::text("activity.store_shelf.not_enough_gold"));
            return true;
        }
        runAndMergeHidden(ctx, 10, Attributes{.energy = 15, .gold = -12},
            HiddenMap{{"healthIndex", -3}},
            cls::text("notice.meal_complete"),
            cls::text("activity.store_shelf.buy"));
        return true;
    }

    if (actionId == "store_drink_fridge") {
        if (ctx.player.getAttributes().gold < 8) {
            ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
                cls::text("activity.store_drink.not_enough_gold"));
            return true;
        }
        runAndMergeHidden(ctx, 5, Attributes{.energy = 10, .gold = -8, .san = -3},
            HiddenMap{{"healthIndex", -5}},
            cls::text("notice.meal_complete"),
            cls::text("activity.store_drink.buy"));
        return true;
    }

    if (actionId == "store_hot_water") {
        if (ctx.player.getAttributes().gold < 10) {
            ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
                cls::text("activity.store_hot_water.not_enough_gold"));
            return true;
        }
        runAndMergeHidden(ctx, 15, Attributes{.energy = 10, .gold = -10, .san = -10},
            HiddenMap{{"healthIndex", -5}, {"lateNightLevel", 1}},
            cls::text("notice.meal_complete"),
            cls::text("activity.store_hot_water.buy"));
        return true;
    }

    if (actionId == "store_counter") {
        if (ctx.player.getAttributes().gold < 6) {
            ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
                cls::text("activity.store_counter.not_enough_gold"));
            return true;
        }
        runAndMergeHidden(ctx, 8, Attributes{.energy = 5, .gold = -6, .san = -2},
            HiddenMap{{"healthIndex", -1}},
            cls::text("notice.meal_complete"),
            cls::text("activity.store_counter.buy"));
        return true;
    }

    if (actionId == "store_night_shift") {
        if (!isEveningOrNight(ctx.timeSystem)) {
            ctx.activityNotice.show(cls::text("notice.too_early"),
                cls::text("activity.store_night_shift.too_early"));
            return true;
        }
        runAndMergeHidden(ctx, 90, Attributes{.energy = -20, .gold = 40, .san = 5},
            HiddenMap{{"partTimeCount", 1}, {"storeNightShiftCount", 1}, {"lateNightLevel", 5}, {"healthIndex", -2}},
            cls::text("notice.training_complete"),
            cls::text("activity.store_night_shift.work"));
        return true;
    }

    return false;
}

} // namespace RegularInteraction
