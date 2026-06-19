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
                "你调整大创 PPT 到很晚。页面更像样了，你也更不像样了。");
            return true;
        }

        runAndMergeHidden(ctx, 60, Attributes{.energy = -10, .san = 5, .academic = 8}, HiddenMap::object(),
            cls::text("notice.study_complete"),
            "你打开台灯复习了一阵。宿舍并不安静，但至少你真的坐下来学了。 ");
        return true;
    }

    if (actionId == "dormitory_games") {
        runAndMergeHidden(ctx, 45, Attributes{.energy = -5, .san = -12, .academic = -2},
            HiddenMap{{"gameAddiction", 5}, {"lateNightLevel", 3}, {"healthIndex", -2}},
            cls::text("notice.game_break_complete"),
            "你打开电脑打了几局游戏，心情短暂变好。只是再来一局的念头开始变得危险。 ");
        return true;
    }

    if (actionId == "dormitory_rug") {
        runAndMergeHidden(ctx, 20, Attributes{.energy = -3, .san = -8}, HiddenMap{{"lateNightLevel", 2}},
            cls::text("notice.quiet_moment"),
            "你坐在地毯上刷了一会儿手机。消息很多，真正重要的也许没几个。 ");
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
        body << "你翻阅了《" << book.name << "》，安静地读了一段时间。"
             << " 知识与经验慢慢积累，" << book.skill << " 相关能力也更熟练了。";
        runAndMergeHidden(ctx, 50, book.delta, hiddenDelta,
            cls::text("notice.reading_complete"), body.str());
        return true;
    }

    if (actionId == "library_table") {
        if (hasJoinedInnovation(ctx) && isAfternoonOrLater(ctx.timeSystem)) {
            runAndMergeHidden(ctx, 70, Attributes{.energy = -10, .san = 6, .academic = 6},
                HiddenMap{{"innovationProgress", 7}, {"innovationTeamTrust", 1}},
                cls::text("notice.study_complete"),
                "你推进了一部分大创方案。项目终于不像只存在于群聊里了。");
            return true;
        }

        runAndMergeHidden(ctx, 60, Attributes{.energy = -8, .san = -4, .academic = 6},
            HiddenMap{{"libraryVisitCount", 1}},
            cls::text("notice.reading_complete"),
            "你在阅读桌前坐了一段时间。图书馆的安静让你更容易进入状态。 ");
        return true;
    }

    if (actionId == "classroom_board") {
        runAndMergeHidden(ctx, 20, Attributes{.energy = -2, .academic = 3},
            HiddenMap{{"teacherTrust", 1}},
            cls::text("notice.board_reviewed"),
            "黑板上还留着上节课的重点。你快速梳理了一遍，之前漏掉的内容清楚了一点。 ");
        return true;
    }

    if (actionId.rfind("classroom_desk_", 0) == 0) {
        const bool inClassTime = ctx.timeSystem.getMinuteOfDay() >= TimeSystem::kClassMinute
            && ctx.timeSystem.getMinuteOfDay() < TimeSystem::kClassEndMinute;
        if (inClassTime) {
            runAndMergeHidden(ctx, 45, Attributes{.energy = -10, .academic = 10},
                HiddenMap{{"classAttendCount", 1}, {"teacherTrust", 2}},
                cls::text("notice.class_complete"),
                "你坐到课桌前，老老实实上完了这一段课程。至少这次老师点名时你人在。 ");
        } else if (hasJoinedInnovation(ctx) && isAfternoonOrLater(ctx.timeSystem)) {
            runAndMergeHidden(ctx, 60, Attributes{.energy = -8, .san = 4, .social = 3},
                HiddenMap{{"innovationProgress", 4}, {"innovationTeamTrust", 3}},
                cls::text("notice.study_complete"),
                "你和队友开了一次大创会议。至少这次大家不只是说'收到'。");
        } else {
            runAndMergeHidden(ctx, 35, Attributes{.energy = -5, .san = -2, .academic = 5}, HiddenMap::object(),
                cls::text("notice.study_complete"),
                "你在教室里翻出笔记复习了一会儿。安静的环境让知识点连了起来。 ");
        }
        return true;
    }

    if (actionId.rfind("gym_treadmill_", 0) == 0) {
        runAndMergeHidden(ctx, 45, Attributes{.energy = -10, .san = -8},
            HiddenMap{{"exerciseCount", 1}, {"healthIndex", 5}},
            cls::text("notice.training_complete"),
            "你在跑步机上跑了一会儿，呼吸变重，脑子反而清爽了一些。 ");
        return true;
    }

    if (actionId.rfind("gym_barbell_", 0) == 0) {
        runAndMergeHidden(ctx, 50, Attributes{.energy = -15, .san = -2},
            HiddenMap{{"exerciseCount", 1}, {"healthIndex", 5}},
            cls::text("notice.training_complete"),
            "你完成了一组力量训练，身体很累，但有种真实的掌控感。 ");
        return true;
    }

    if (actionId == "gym_front_desk") {
        if (!isEveningOrNight(ctx.timeSystem)) {
            ctx.activityNotice.show(cls::text("notice.too_early"),
                "健身房前台兼职通常安排在晚上，现在还没有开始。");
            return true;
        }
        runAndMergeHidden(ctx, 75, Attributes{.energy = -16, .gold = 30, .san = 4},
            HiddenMap{{"partTimeCount", 1}, {"healthIndex", -1}},
            cls::text("notice.training_complete"),
            "你在健身房前台帮忙登记和整理器材，赚到一点兼职费。");
        return true;
    }

    if (actionId == "store_shelf") {
        if (ctx.player.getAttributes().gold < 12) {
            ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
                "你想买点便利店食物，但身上的钱不够。 ");
            return true;
        }
        runAndMergeHidden(ctx, 10, Attributes{.energy = 15, .gold = -12},
            HiddenMap{{"healthIndex", -3}},
            cls::text("notice.meal_complete"),
            "你在便利店买了点能快速入口的食物。它不如食堂健康，但胜在方便。 ");
        return true;
    }

    if (actionId == "store_drink_fridge") {
        if (ctx.player.getAttributes().gold < 8) {
            ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
                "你想买饮料提神，但手头的钱不太够。 ");
            return true;
        }
        runAndMergeHidden(ctx, 5, Attributes{.energy = 10, .gold = -8, .san = -3},
            HiddenMap{{"healthIndex", -5}},
            cls::text("notice.meal_complete"),
            "你买了一瓶提神饮料。短时间内精神回来了，但这不是真正的休息。 ");
        return true;
    }

    if (actionId == "store_hot_water") {
        if (ctx.player.getAttributes().gold < 10) {
            ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
                "你想泡面，但连这点钱都拿不出来。 ");
            return true;
        }
        runAndMergeHidden(ctx, 15, Attributes{.energy = 10, .gold = -10, .san = -10},
            HiddenMap{{"healthIndex", -5}, {"lateNightLevel", 1}},
            cls::text("notice.meal_complete"),
            "你泡了一桶面。热气升起来的瞬间，深夜似乎变得没那么难熬。 ");
        return true;
    }

    if (actionId == "store_counter") {
        if (ctx.player.getAttributes().gold < 6) {
            ctx.activityNotice.show(cls::text("notice.not_enough_gold"),
                "你在收银台前看了看价格，最后还是把东西放了回去。");
            return true;
        }
        runAndMergeHidden(ctx, 8, Attributes{.energy = 5, .gold = -6, .san = -2},
            HiddenMap{{"healthIndex", -1}},
            cls::text("notice.meal_complete"),
            "你在收银台买了点小东西。花费不多，但足够把这一小段空档撑过去。");
        return true;
    }

    if (actionId == "store_night_shift") {
        if (!isEveningOrNight(ctx.timeSystem)) {
            ctx.activityNotice.show(cls::text("notice.too_early"),
                "便利店夜班要到傍晚以后才会开始，现在还太早。 ");
            return true;
        }
        runAndMergeHidden(ctx, 90, Attributes{.energy = -20, .gold = 40, .san = 5},
            HiddenMap{{"partTimeCount", 1}, {"storeNightShiftCount", 1}, {"lateNightLevel", 5}, {"healthIndex", -2}},
            cls::text("notice.training_complete"),
            "你开始便利店兼职，扫码、找零、补货，一套流程下来比想象中更累。 ");
        return true;
    }

    return false;
}

} // namespace RegularInteraction
