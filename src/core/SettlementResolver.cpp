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

void appendEndingLine(std::ostringstream& out, const EndingDefinition& ending) {
    const std::string endingName = localizedValue(ending.nameKey, ending.name);
    const std::string endingTagline = localizedValue(ending.taglineKey, ending.tagline);
    out << endingName;
    if (!endingTagline.empty()) out << " - " << endingTagline;
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
            if (cond.value("kind", "") == "meta") continue;
            if (evaluateConditionJson(cond, player)) return true;
        }
        return false;
    }
    for (const auto& cond : conditions) {
        if (cond.value("kind", "") == "meta") continue;
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
        return true;
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
        def.requireMode = item.value("require", "all");
        def.conditions = item.value("conditions", json::array());
        mEndings.push_back(def);
    }
    std::sort(mEndings.begin(), mEndings.end(), [](const EndingDefinition& lhs, const EndingDefinition& rhs) {
        return lhs.priority < rhs.priority;
    });

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
            title.requireMode = item.value("require", "all");
            title.conditions = item.value("conditions", json::array());
            mTitleDefs.push_back(title);
        }
    }
    return true;
}

SettlementResult SettlementResolver::resolveImmediate(const Player& player) const {
    SettlementResult result;
    for (const auto& ending : mEndings) {
        if (ending.trigger != "immediate") continue;
        if (evaluateConditionsJson(ending.conditions, ending.requireMode, player)) {
            result.resolved = true;
            result.gameOver = ending.type == "game_over";
            result.ending = ending;
            result.achievedEndings.push_back(ending);
            result.summary = buildSummary(player, result);
            return result;
        }
    }
    return result;
}

SettlementResult SettlementResolver::resolveFinal(const Player& player) const {
    SettlementResult result;
    const EndingDefinition* defaultEnding = nullptr;

    for (const auto& ending : mEndings) {
        if (ending.trigger == "immediate") continue;
        if (ending.type == "default") {
            defaultEnding = &ending;
            continue;
        }
        if (evaluateConditionsJson(ending.conditions, ending.requireMode, player)) {
            result.achievedEndings.push_back(ending);
        }
    }

    if (result.achievedEndings.empty() && defaultEnding) {
        result.achievedEndings.push_back(*defaultEnding);
    }
    if (!result.achievedEndings.empty()) {
        result.ending = result.achievedEndings.front();
        result.resolved = true;
        result.gameOver = std::any_of(result.achievedEndings.begin(), result.achievedEndings.end(),
            [](const EndingDefinition& ending) {
                return ending.type == "game_over";
            });
    }

    for (const auto& title : mTitleDefs) {
        if (evaluateConditionsJson(title.conditions, title.requireMode, player)) {
            result.titles.push_back(title);
        }
    }

    result.summary = buildSummary(player, result);
    return result;
}

std::string SettlementResolver::buildSummary(const Player& player, const SettlementResult& result) const {
    std::ostringstream summary;
    const auto& endings = result.achievedEndings;
    const EndingDefinition* primaryEnding = nullptr;
    if (!endings.empty()) primaryEnding = &endings.front();
    else if (!result.ending.id.empty()) primaryEnding = &result.ending;

    if (primaryEnding) {
        if (endings.size() > 1) {
            summary << cls::text("settlement.primary_ending") << ": ";
        }
        appendEndingLine(summary, *primaryEnding);
        const std::string endingDescription = localizedValue(
            primaryEnding->descriptionKey, primaryEnding->description);
        summary << "\n\n" << endingDescription;

        if (endings.size() > 1) {
            summary << "\n\n" << cls::text("settlement.also_achieved") << ":\n";
            for (std::size_t i = 1; i < endings.size(); ++i) {
                summary << "- ";
                appendEndingLine(summary, endings[i]);
                summary << "\n";
            }
        }
    }
    summary << "\n\n" << cls::text("settlement.final_stats") << ": "
            << cls::text("hud.energy") << " " << player.getAttributes().energy
            << " / " << cls::text("hud.gold") << " " << player.getAttributes().gold
            << " / " << cls::text("hud.san") << " " << player.getAttributes().san
            << " / " << cls::text("hud.academic") << " " << player.getAttributes().academic
            << " / " << cls::text("hud.social") << " " << player.getAttributes().social;
    return summary.str();
}
