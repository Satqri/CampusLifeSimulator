#include "core/SettlementResolver.h"

#include "entity/Player.h"
#include "core/Localization.h"
#include "core/Types.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

namespace {

int getVisibleStat(const Player& player, const std::string& key, bool& found) {
    found = true;
    const auto& attrs = player.getAttributes();
    if (key == "energy") return attrs.energy;
    if (key == "health") return attrs.health;
    if (key == "gold") return attrs.gold;
    if (key == "academic") return attrs.academic;
    if (key == "social") return attrs.social;
    if (key == "stress") return 100 - attrs.san;
    if (key == "san") return attrs.san;
    found = false;
    return 0;
}

bool evaluateConditionJson(const json& cond, const Player& player);

std::string localizedValue(const std::string& key, const std::string& fallback) {
    if (!key.empty()) return cls::text(key);
    return fallback;
}

bool compareJsonValues(const json& actual, const std::string& op, const json& expected) {
    if (actual.is_number() && expected.is_number()) {
        const int lhs = actual.get<int>();
        const int rhs = expected.get<int>();
        if (op == ">=") return lhs >= rhs;
        if (op == "<=") return lhs <= rhs;
        if (op == ">") return lhs > rhs;
        if (op == "<") return lhs < rhs;
        if (op == "eq") return lhs == rhs;
        if (op == "neq") return lhs != rhs;
        return false;
    }
    if (actual.is_boolean() && expected.is_boolean()) {
        const bool lhs = actual.get<bool>();
        const bool rhs = expected.get<bool>();
        if (op == "eq") return lhs == rhs;
        if (op == "neq") return lhs != rhs;
        return false;
    }
    if (actual.is_string() && expected.is_string()) {
        const std::string lhs = actual.get<std::string>();
        const std::string rhs = expected.get<std::string>();
        if (op == "eq") return lhs == rhs;
        if (op == "neq") return lhs != rhs;
        return false;
    }
    return false;
}

bool evaluateConditionsJson(const json& conditions, const std::string& requireMode, const Player& player) {
    if (!conditions.is_array() || conditions.empty()) return true;
    if (requireMode == "any") {
        for (const auto& cond : conditions) {
            if (evaluateConditionJson(cond, player)) return true;
        }
        return false;
    }
    for (const auto& cond : conditions) {
        if (!evaluateConditionJson(cond, player)) return false;
    }
    return true;
}

bool evaluateConditionJson(const json& cond, const Player& player) {
    const std::string kind = cond.value("kind", "");
    if (kind == "group") {
        return evaluateConditionsJson(cond["conditions"], cond.value("require", "all"), player);
    }
    if (kind == "meta") {
        return false;
    }
    if (kind == "stat" || kind == "enum") {
        const std::string stat = cond.value("stat", "");
        const std::string op = cond.value("op", "eq");
        const auto& hidden = player.getHidden();
        bool found = false;
        int visible = getVisibleStat(player, stat, found);
        if (found) {
            return compareJsonValues(json(visible), op, cond["value"]);
        }
        if (!hidden.contains(stat)) return false;
        return compareJsonValues(hidden.at(stat), op, cond["value"]);
    }
    return false;
}

} // namespace

bool SettlementResolver::load(const std::string& endingsPath, const std::string& titlesPath) {
    mEndings.clear();
    mTitleDefs.clear();

    std::ifstream endingsFile(endingsPath);
    if (!endingsFile.is_open()) return false;
    json endingsJson;
    endingsFile >> endingsJson;
    for (const auto& item : endingsJson["endings"]) {
        EndingDefinition def;
        def.priority = item.value("priority", 0);
        def.id = item.value("id", "");
        def.name = item.value("name", "");
        def.nameKey = item.value("name_key", "");
        def.tagline = item.value("tagline", "");
        def.taglineKey = item.value("tagline_key", "");
        def.description = item.value("description", "");
        def.descriptionKey = item.value("description_key", "");
        def.type = item.value("type", "");
        def.trigger = item.value("trigger", "");
        mEndings.push_back(def);
    }

    std::ifstream titlesFile(titlesPath);
    if (!titlesFile.is_open()) return false;
    json titlesJson;
    titlesFile >> titlesJson;
    for (const auto& group : titlesJson["groups"]) {
        const std::string groupName = group.value("group", "");
        const std::string groupKey = group.value("group_key", "");
        for (const auto& item : group["titles"]) {
            EarnedTitle title;
            title.group = groupName;
            title.groupKey = groupKey;
            title.id = item.value("id", "");
            title.name = item.value("name", "");
            title.nameKey = item.value("name_key", "");
            title.subtitle = item.value("subtitle", "");
            title.subtitleKey = item.value("subtitle_key", "");
            title.text = item.value("text", "");
            title.textKey = item.value("text_key", "");
            mTitleDefs.push_back(title);
        }
    }
    return true;
}

SettlementResult SettlementResolver::resolveImmediate(const Player& player) const {
    SettlementResult result;
    for (const auto& ending : mEndings) {
        if (ending.trigger != "immediate") continue;
        const auto& attrs = player.getAttributes();
        const auto& hidden = player.getHidden();
        if (ending.id == "hospitalized" && hidden.value("healthIndex", 100) <= 0) {
            result.resolved = true;
            result.gameOver = true;
            result.ending = ending;
            result.summary = buildSummary(player, result);
            return result;
        }
        if (ending.id == "breakdown" && ((100 - attrs.san) >= 100 || attrs.energy <= 0)) {
            result.resolved = true;
            result.gameOver = true;
            result.ending = ending;
            result.summary = buildSummary(player, result);
            return result;
        }
    }
    return result;
}

SettlementResult SettlementResolver::resolveFinal(const Player& player) const {
    SettlementResult result;
    const auto& attrs = player.getAttributes();
    const auto& hidden = player.getHidden();

    auto setEnding = [&](const std::string& id) {
        for (const auto& ending : mEndings) {
            if (ending.id == id) {
                result.ending = ending;
                result.resolved = true;
                result.gameOver = ending.type == "game_over";
                return;
            }
        }
    };

    if (hidden.value("healthIndex", 100) <= 0) setEnding("hospitalized");
    else if ((100 - attrs.san) >= 100 || attrs.energy <= 0) setEnding("breakdown");
    else if (attrs.academic < 40 && hidden.value("skipClassCount", 0) >= 6) setEnding("retention");
    else if (hidden.value("innovationResult", std::string("none")) == "excellent"
          || (hidden.value("innovationProgress", 0) >= 80 && hidden.value("innovationTeamTrust", 0) >= 35 && hidden.value("innovationDefenseScore", 0) >= 60)) setEnding("innovation_star");
    else if (attrs.academic >= 85 && hidden.value("teacherTrust", 0) >= 25 && hidden.value("skipClassCount", 0) <= 1) setEnding("top_student");
    else if (attrs.social >= 85 && attrs.academic >= 50) setEnding("social_butterfly");
    else if (attrs.gold >= 250 && hidden.value("partTimeCount", 0) >= 6) setEnding("workaholic");
    else if (hidden.value("healthIndex", 100) >= 110 && hidden.value("exerciseCount", 0) >= 7) setEnding("fitness_guru");
    else if (hidden.value("friendBond", 0) >= 10 && attrs.social >= 80) setEnding("network_king");
    else if (hidden.value("gameAddiction", 0) >= 30 && (100 - attrs.san) <= 50) setEnding("gamer_legend");
    else if (attrs.academic >= 70 && hidden.value("lateNightLevel", 0) >= 25) setEnding("night_grinder");
    else if ((100 - attrs.san) <= 30 && attrs.academic < 50) setEnding("happy_slacker");
    else setEnding("normal_graduate");

    auto addTitle = [&](const std::string& id) {
        for (const auto& def : mTitleDefs) {
            if (def.id == id) result.titles.push_back(def);
        }
    };

    if (hidden.value("classAttendCount", 0) >= 8 && hidden.value("skipClassCount", 0) <= 1 && hidden.value("teacherTrust", 0) >= 20) addTitle("class_stable_student");
    if (hidden.value("returnClassCount", 0) >= 2) addTitle("class_honest_repair");
    if (hidden.value("skipClassCount", 0) >= 2 && hidden.value("rollCallSavedCount", 0) >= 2) addTitle("class_roll_call_survivor");
    if (hidden.value("skipClassCount", 0) >= 5 && hidden.value("absencePenaltyCount", 0) >= 3 && hidden.value("teacherTrust", 0) <= 10) addTitle("class_absence_warning");

    if (hidden.value("friendBond", 0) >= 6 && hidden.value("friendHelpCount", 0) >= 3) addTitle("friend_reliable_partner");
    if (hidden.value("friendRollCallHelpCount", 0) >= 1) addTitle("friend_roll_call_buddy");
    if (hidden.value("sharedNotes", false) && hidden.value("friendReviewCount", 0) >= 1) addTitle("friend_exam_ally");
    if (hidden.value("friendBond", 0) >= 0 && hidden.value("friendBond", 0) <= 2 && hidden.value("friendHelpCount", 0) <= 1) addTitle("friend_nodding_terms");
    if (hidden.value("friendRefuseCount", 0) >= 2 || hidden.value("friendBond", 0) < 0) addTitle("friend_read_ignored");

    if (hidden.value("clubType", std::string("none")) != "none" && hidden.value("clubContribution", 0) >= 6 && hidden.value("clubShowcaseSuccess", false)) addTitle("club_backbone");
    if (hidden.value("clubRelation", 0) >= 5) addTitle("club_mediator");
    if (hidden.value("clubShowcaseScore", 0) >= 18 && (hidden.value("expressionUnlocked", false) || attrs.social >= 50)) addTitle("club_showcase_star");
    if (hidden.value("clubActivityCount", 0) >= 2 && hidden.value("clubContribution", 0) >= 2 && hidden.value("clubContribution", 0) <= 5) addTitle("club_steady_member");
    if (hidden.value("clubType", std::string("none")) != "none" && (hidden.value("clubActivityCount", 0) <= 1 || hidden.value("clubContribution", 0) < 0)) addTitle("club_ghost_member");
    if (hidden.value("clubType", std::string("none")) == "none") addTitle("club_free_spirit");

    if (hidden.value("innovationResult", std::string("none")) == "excellent") addTitle("innovation_campus_star");
    if (hidden.value("innovationTeamTrust", 0) >= 40 && hidden.value("innovationResult", std::string("none")) != "failed") addTitle("innovation_team_harmony");
    if (hidden.value("innovationResult", std::string("none")) == "firefighter" || (hidden.value("innovationCrisisCount", 0) >= 2 && hidden.value("innovationProgress", 0) >= 60)) addTitle("innovation_firefighter");
    if (hidden.value("innovationResult", std::string("none")) == "completed") addTitle("innovation_completed");
    if (hidden.value("innovationResult", std::string("none")) == "pass" && (hidden.value("innovationDemoMode", std::string("none")) == "safe" || hidden.value("innovationDefenseScore", 0) >= 25)) addTitle("innovation_ppt_master");
    if (hidden.value("innovationResult", std::string("none")) == "failed" || hidden.value("innovationTeamTrust", 0) <= 0) addTitle("innovation_silent");
    if (hidden.value("innovationResult", std::string("none")) == "bystander" || !hidden.value("innovationJoined", false)) addTitle("innovation_bystander");

    if (hidden.value("lateCount", 0) >= 5) addTitle("daily_morning_runner");
    if (hidden.value("libraryVisitCount", 0) >= 10) addTitle("daily_library_dweller");
    if (hidden.value("mealCount", 0) >= 20) addTitle("daily_cafeteria_resident");
    if (hidden.value("storeNightShiftCount", 0) >= 5) addTitle("daily_night_shift_champion");

    result.summary = buildSummary(player, result);
    return result;
}

std::string SettlementResolver::buildSummary(const Player& player, const SettlementResult& result) const {
    std::ostringstream summary;
    const std::string endingName = localizedValue(result.ending.nameKey, result.ending.name);
    const std::string endingTagline = localizedValue(result.ending.taglineKey, result.ending.tagline);
    const std::string endingDescription = localizedValue(result.ending.descriptionKey, result.ending.description);

    summary << endingName;
    if (!endingTagline.empty()) summary << " - " << endingTagline;
    summary << "\n\n" << endingDescription;
    summary << "\n\n" << cls::text("settlement.final_stats") << ": "
            << cls::text("hud.energy") << " " << player.getAttributes().energy
            << " / " << cls::text("hud.gold") << " " << player.getAttributes().gold
            << " / " << cls::text("hud.san") << " " << player.getAttributes().san
            << " / " << cls::text("hud.academic") << " " << player.getAttributes().academic
            << " / " << cls::text("hud.social") << " " << player.getAttributes().social;
    return summary.str();
}
