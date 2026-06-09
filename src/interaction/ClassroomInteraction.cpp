#include "interaction/ClassroomInteraction.h"
#include "core/GameContext.h"
#include "map/MapPortal.h"
#include <cstdlib>
#include <sstream>

namespace ClassroomInteraction {

void forceMorningClass(GameContext& ctx) {
    ctx.timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
    ctx.timeSystem.markClassPrompted();
    ctx.currentPlace = CampusPlace::Classroom;
    ctx.currentMap = ctx.classroomMap;

    std::vector<const InteractionPoint*> desks;
    for (const auto& ip : ctx.classroomMap->getInteractionPoints()) {
        if (ip.label == "Sit at Desk") desks.push_back(&ip);
    }
    if (!desks.empty()) {
        const InteractionPoint* desk = desks[std::rand() % desks.size()];
        ctx.player.setPosition(desk->area.position.x + desk->area.size.x * 0.5f,
                               desk->area.position.y + desk->area.size.y * 0.5f);
    } else {
        ctx.player.setPosition(480.0f, 276.0f);
    }
    ctx.player.stopMovement();

    ctx.classChoicePrompt.show(
        ctx.timeSystem.isMidtermDay() ? "Midterm Morning" : "Morning Class",
        ctx.timeSystem.isMidtermDay()
            ? "It is Day 7. The midterm starts from this classroom seat."
            : "The bell rings at 08:50. Choose how to handle this class.",
        ctx.timeSystem.isMidtermDay() ? "Take the midterm seriously" : "Attend class carefully",
        "Skip class"
    );
}

void checkClassSchedule(GameContext& ctx, int previousMinute) {
    if (ctx.timeSystem.crossedClassTime(previousMinute) || ctx.timeSystem.shouldForceClass()) {
        ctx.activityNotice.clear();
        forceMorningClass(ctx);
    }
}

void resolveClassChoice(GameContext& ctx, bool attend) {
    ctx.classChoicePrompt.clear();
    ctx.timeSystem.markClassResolved();

    if (attend) {
        ctx.timeSystem.setTimeAbsolute(TimeSystem::kClassEndMinute);
        std::ostringstream body;
        if (ctx.timeSystem.isMidtermDay()) {
            const int roll = (std::rand() % 20) + 1;
            const int academicBonus = (ctx.player.getAttributes().academic - 50) / 10;
            const int total = roll + academicBonus;
            const bool passed = total >= 12;
            ctx.player.modifyAttributes(passed
                ? Attributes{.energy = -16, .san = -10, .academic = 12}
                : Attributes{.energy = -18, .san = -16, .academic = 4});
            body << "Midterm finished. Roll " << roll
                 << " + Academic Bonus " << academicBonus
                 << " = " << total << (passed ? " (pass)." : " (struggle).");
        } else {
            ctx.player.modifyAttributes(Attributes{.energy = -12, .san = -8, .academic = 8});
            body << "You focused through the morning lecture. Academic +8, SAN -8, Energy -12.";
        }
        ctx.timeSkipFlash.start("Class time passes...");
        ctx.showTimedResult(ctx.timeSystem.isMidtermDay() ? "Midterm Complete" : "Class Complete", body.str());
    } else {
        ctx.timeSystem.setTimeAbsolute(TimeSystem::kRollCallMinute);
        const bool called = (std::rand() % 100) < (ctx.timeSystem.isMidtermDay() ? 80 : 45);
        std::ostringstream body;
        if (called) {
            ctx.player.modifyAttributes(ctx.timeSystem.isMidtermDay()
                ? Attributes{.energy = -4, .san = -18, .academic = -18, .social = -12}
                : Attributes{.energy = -2, .san = -10, .academic = -10, .social = -8});
            body << "At 10:20 the teacher calls attendance. You are absent and take a penalty.";
        } else {
            ctx.player.modifyAttributes(Attributes{.energy = -2, .san = 3, .academic = -2});
            body << "At 10:20 there is no roll call. You avoid the immediate penalty, but lose study momentum.";
        }
        ctx.timeSkipFlash.start("Skipping class...");
        ctx.showTimedResult("Roll Call Notice", body.str());
    }
}

bool handle(GameContext& ctx, const InteractionPoint& ip) {
    const std::string& id = ip.actionId;

    if (id == "classroom_board") {
        ctx.activityNotice.show("Board Reviewed",
            "The blackboard is covered with today's lecture notes. Reviewing them might give you an edge.");
        return true;
    }

    if (id.rfind("classroom_desk_", 0) == 0) {
        ctx.activityNotice.show("Morning Class",
            "Morning class at 08:50 is handled by the class schedule event. To study on your own, visit the library.");
        return true;
    }

    return false;
}

} // namespace ClassroomInteraction
