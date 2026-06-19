#include "event/EventRunner.h"

#include "core/GameContext.h"
#include "core/TimeSystem.h"
#include "core/CharacterState.h"
#include "core/Localization.h"
#include "entity/Player.h"
#include "ui/ModalBox.h"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

EventRunner::EventRunner()
    : mRng(static_cast<unsigned int>(
          std::chrono::steady_clock::now().time_since_epoch().count())) {
}

// ─── 辅助解析 ───────────────────────────────────────────────────────────────

static EventNodeType parseNodeType(const std::string& s) {
    if (s == "display")       return EventNodeType::DISPLAY;
    if (s == "choice")        return EventNodeType::CHOICE;
    if (s == "random_check")  return EventNodeType::RANDOM_CHECK;
    if (s == "check")         return EventNodeType::CHECK;
    if (s == "outcome")       return EventNodeType::OUTCOME;
    std::cerr << "[Event] Unknown node type: " << s << std::endl;
    return EventNodeType::OUTCOME;
}

static ConditionKind parseConditionKind(const std::string& s) {
    if (s == "location")  return ConditionKind::LOCATION;
    if (s == "stat")      return ConditionKind::STAT;
    if (s == "flag")      return ConditionKind::FLAG;
    if (s == "group")     return ConditionKind::GROUP;
    if (s == "meta")      return ConditionKind::META;
    if (s == "enum")      return ConditionKind::ENUM;
    if (s == "time")      return ConditionKind::TIME;
    std::cerr << "[Event] Unknown condition kind: " << s << std::endl;
    return ConditionKind::LOCATION;
}

static std::string normalizeActionId(const std::string& actionId) {
    if (actionId.rfind("cafeteria_table_", 0) == 0) return "cafeteria_table";
    if (actionId.rfind("classroom_desk_", 0) == 0) return "classroom_desk";
    if (actionId.rfind("gym_treadmill_", 0) == 0) return "gym_treadmill";
    if (actionId.rfind("gym_barbell_", 0) == 0) return "gym_bench";
    if (actionId.rfind("library_shelf_", 0) == 0) return "library_shelf";
    if (actionId == "library_table") return "library_desk";
    return actionId;
}

static std::string eventCountKey(const std::string& eventId) {
    return "_event_" + eventId + "_count";
}

static std::string eventLastDayKey(const std::string& eventId) {
    return "_event_" + eventId + "_last_day";
}

static bool passesTriggerGate(const EventDefinition& def, GameContext& ctx,
                              std::mt19937& rng) {
    const auto& hidden = ctx.player.getHidden();
    const std::string countKey = eventCountKey(def.id);
    const std::string lastDayKey = eventLastDayKey(def.id);
    const int triggerCount = hidden.value(countKey, 0);

    if (def.trigger.once && triggerCount > 0)
        return false;

    if (def.trigger.cooldownDays > 0 && hidden.contains(lastDayKey)) {
        const int lastDay = hidden.value(lastDayKey, -999);
        if (ctx.timeSystem.getDay() - lastDay <= def.trigger.cooldownDays)
            return false;
    }

    if (def.trigger.chance < 100) {
        const int roll = static_cast<int>(rng() % 100);
        if (roll >= def.trigger.chance)
            return false;
    }

    return true;
}

static void markEventTriggered(const std::string& eventId, GameContext& ctx) {
    HiddenMap delta = HiddenMap::object();
    delta[eventCountKey(eventId)] = 1;
    delta[eventLastDayKey(eventId)] = ctx.timeSystem.getDay();
    mergeHidden(ctx.player.getHidden(), delta);
}

static std::string phaseName(TimePhase phase) {
    switch (phase) {
        case TimePhase::EarlyMorning: return "early_morning";
        case TimePhase::Noon:         return "noon";
        case TimePhase::Afternoon:    return "afternoon";
        case TimePhase::Evening:      return "evening";
        case TimePhase::Night:        return "night";
    }
    return "";
}

static Condition parseCondition(const json& c) {
    Condition cond;
    cond.kind = parseConditionKind(c.value("kind", ""));
    cond.place = c.value("place", "");
    cond.stat = c.value("stat", "");
    cond.op = c.value("op", "");
    cond.flag = c.value("flag", "");
    cond.requireMode = c.value("require", "all");
    if (c.contains("value")) {
        cond.valueJson = c["value"];
        if (cond.valueJson.is_number_integer()) {
            cond.value = cond.valueJson.get<int>();
        }
    }
    if (c.contains("conditions") && c["conditions"].is_array()) {
        for (const auto& nested : c["conditions"]) {
            cond.conditions.push_back(parseCondition(nested));
        }
    }
    return cond;
}

// ─── JSON 加载 ───────────────────────────────────────────────────────────────


static std::string localizedValue(const std::string& key, const std::string& fallback) {
    if (!key.empty()) return cls::text(key);
    return fallback;
}

bool EventRunner::loadEvents(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Event] Cannot open: " << filepath << std::endl;
        return false;
    }
    json root;
    try { file >> root; } catch (const json::parse_error& e) {
        std::cerr << "[Event] JSON parse error: " << e.what() << std::endl;
        return false;
    }

    for (const auto& ev : root["events"]) {
        EventDefinition def;
        def.id       = ev.value("id", "");
        def.rootNode = ev.value("root", "");

        // 解析 trigger
        const auto& tr = ev["trigger"];
        std::string trigType = tr.value("type", "");
        if (trigType == "time_schedule") {
            def.trigger.type = EventTrigger::TIME_SCHEDULE;
            def.trigger.method = tr.value("method", "");
        } else if (trigType == "interaction") {
            def.trigger.type = EventTrigger::INTERACTION;
            def.trigger.actionId = tr.value("action_id", "");
        }
        def.trigger.requireMode = tr.value("require", "all");
        def.trigger.chance = std::clamp(tr.value("chance", 100), 0, 100);
        def.trigger.once = tr.value("once", false);
        def.trigger.cooldownDays = std::max(0, tr.value("cooldown_days", 0));
        def.trigger.fallbackToRegular = tr.value("fallback_to_regular", false);
        if (tr.contains("conditions") && tr["conditions"].is_array()) {
            for (const auto& c : tr["conditions"]) {
                def.trigger.conditions.push_back(parseCondition(c));
            }
        }

        // 解析 steps
        const auto& steps = ev["steps"];
        for (auto it = steps.begin(); it != steps.end(); ++it) {
            EventNode node;
            const auto& n = it.value();
            node.type    = parseNodeType(n.value("type", ""));
            node.title   = n.value("title", "");
            node.titleKey = n.value("title_key", "");
            node.body    = n.value("body", "");
            node.bodyKey = n.value("body_key", "");
            node.footer  = n.value("footer", "");
            node.footerKey = n.value("footer_key", "");

            switch (node.type) {
            case EventNodeType::CHOICE:
                for (const auto& opt : n["options"]) {
                    node.options.push_back({
                        opt.value("text", ""),
                        opt.value("text_key", ""),
                        opt.value("then", "")
                    });
                }
                break;

            case EventNodeType::DISPLAY:
                node.thenNode = n.value("then", "");
                break;

            case EventNodeType::RANDOM_CHECK:
                node.probability        = n.value("probability", 50);
                node.successNode        = n.value("success", "");
                node.failureNode        = n.value("failure", "");
                node.timeAdvanceMinutes = n.value("time_advance", 0);
                node.flashText          = n.value("flash", "");
                node.flashKey           = n.value("flash_key", "");
                if (n.contains("delta")) {
                    const auto& d = n["delta"];
                    if (!d.is_null()) {
                        node.delta = Attributes{
                            .energy   = d.value("energy", 0),
                            .health   = d.value("health", 0),
                            .gold     = d.value("gold", 0),
                            .san      = d.value("san", 0),
                            .academic = d.value("academic", 0),
                            .social   = d.value("social", 0)
                        };
                    }
                }
                if (n.contains("hidden_delta"))
                    node.hiddenDelta = n["hidden_delta"];
                break;

            case EventNodeType::CHECK:
                node.requireMode = n.value("require", "all");
                for (const auto& c : n["conditions"]) {
                    node.conditions.push_back(parseCondition(c));
                }
                node.thenNode = n.value("then", "");
                node.elseNode = n.value("else", "");
                node.timeAdvanceMinutes = n.value("time_advance", 0);
                node.flashText          = n.value("flash", "");
                node.flashKey           = n.value("flash_key", "");
                if (n.contains("delta")) {
                    const auto& d = n["delta"];
                    if (!d.is_null()) {
                        node.delta = Attributes{
                            .energy   = d.value("energy", 0),
                            .health   = d.value("health", 0),
                            .gold     = d.value("gold", 0),
                            .san      = d.value("san", 0),
                            .academic = d.value("academic", 0),
                            .social   = d.value("social", 0)
                        };
                    }
                }
                if (n.contains("hidden_delta"))
                    node.hiddenDelta = n["hidden_delta"];
                break;

            case EventNodeType::OUTCOME: {
                if (n.contains("delta")) {
                    const auto& d = n["delta"];
                    if (!d.is_null()) {
                        node.delta = Attributes{
                            .energy   = d.value("energy", 0),
                            .health   = d.value("health", 0),
                            .gold     = d.value("gold", 0),
                            .san      = d.value("san", 0),
                            .academic = d.value("academic", 0),
                            .social   = d.value("social", 0)
                        };
                    }
                }
                node.timeAdvanceMinutes = n.value("time_advance", 0);
                node.flashText          = n.value("flash", "");
                node.flashKey           = n.value("flash_key", "");
                if (n.contains("hidden_delta"))
                    node.hiddenDelta = n["hidden_delta"];
                break;
            }
            }

            def.steps[it.key()] = node;
        }

        if (def.id.empty() || def.rootNode.empty()) {
            std::cerr << "[Event] Skipping event with missing id or root" << std::endl;
            continue;
        }
        mEvents[def.id] = def;
        mEventOrder.push_back(def.id);
        std::cout << "[Event] Loaded: " << def.id << " (" << def.steps.size()
                  << " steps)" << std::endl;
    }
    return true;
}

// ─── 启动 / 清除 ────────────────────────────────────────────────────────────

bool EventRunner::startEvent(const std::string& eventId, GameContext& ctx) {
    auto it = mEvents.find(eventId);
    if (it == mEvents.end()) return false;

    mCurrentEvent    = &it->second;
    mCurrentEventId  = eventId;
    mActive          = true;
    mWaitingForEnter = false;

    std::cout << "[Event] Starting: " << eventId << std::endl;
    transitionTo(mCurrentEvent->rootNode, ctx);
    return true;
}

void EventRunner::clear() {
    mActive          = false;
    mWaitingForEnter = false;
    mCurrentEvent    = nullptr;
    mCurrentEventId.clear();
    mCurrentNodeId.clear();
}

// ─── 节点跳转 ───────────────────────────────────────────────────────────────

void EventRunner::transitionTo(const std::string& nodeId, GameContext& ctx) {
    if (!mCurrentEvent) return;
    auto it = mCurrentEvent->steps.find(nodeId);
    if (it == mCurrentEvent->steps.end()) {
        std::cerr << "[Event] Missing step: " << nodeId << std::endl;
        clear();
        return;
    }

    mCurrentNodeId = nodeId;
    const EventNode& node = it->second;

    // 带 title 的 RANDOM_CHECK/CHECK：先展示文字，等待 Enter 后再解析
    if ((!node.title.empty() || !node.titleKey.empty()) &&
        (node.type == EventNodeType::RANDOM_CHECK ||
         node.type == EventNodeType::CHECK)) {
        mWaitingForEnter = true;
        return;
    }

    // 自动解析
    switch (node.type) {
    case EventNodeType::DISPLAY:
        mWaitingForEnter = true;
        return;

    case EventNodeType::CHOICE:
        return;

    case EventNodeType::RANDOM_CHECK: {
        applyEffects(node, ctx);
        const int roll = static_cast<int>(mRng() % 100);
        const bool success = roll < node.probability;
        std::cout << "[Event] Random: roll=" << roll
                  << " p=" << node.probability
                  << " -> " << (success ? "success" : "failure") << std::endl;
        transitionTo(success ? node.successNode : node.failureNode, ctx);
        return;
    }
    case EventNodeType::CHECK: {
        applyEffects(node, ctx);
        const bool passed = evaluateConditions(
            node.conditions, node.requireMode, ctx);
        const std::string& next = passed ? node.thenNode : node.elseNode;
        if (next.empty()) {
            std::cerr << "[Event] CHECK node has no " << (passed ? "then" : "else") << std::endl;
            clear();
            return;
        }
        transitionTo(next, ctx);
        return;
    }
    case EventNodeType::OUTCOME:
        applyEffects(node, ctx);
        markEventTriggered(mCurrentEventId, ctx);
        const std::string title = localizedValue(node.titleKey, node.title);
        const std::string bodyText = localizedValue(node.bodyKey, node.body);
        if (!title.empty() || !bodyText.empty()) {
            std::ostringstream body;
            body << bodyText;
            body << "\n" << cls::text("time.current") << ": " << ctx.timeSystem.clockText();
            ctx.activityNotice.show(title, body.str());
        }
        clear();
        return;
    }
}

// ─── 键盘输入 ───────────────────────────────────────────────────────────────

void EventRunner::handleInput(sf::Keyboard::Key key, GameContext& ctx) {
    if (!mActive || !mCurrentEvent) return;
    auto it = mCurrentEvent->steps.find(mCurrentNodeId);
    if (it == mCurrentEvent->steps.end()) return;

    const EventNode& node = it->second;
    const bool isEnter  = (key == sf::Keyboard::Key::Enter);
    const bool isEsc    = (key == sf::Keyboard::Key::Escape);
    const bool isNum1   = (key == sf::Keyboard::Key::Num1 || key == sf::Keyboard::Key::Numpad1);
    const bool isNum2   = (key == sf::Keyboard::Key::Num2 || key == sf::Keyboard::Key::Numpad2);
    const bool isNum3   = (key == sf::Keyboard::Key::Num3 || key == sf::Keyboard::Key::Numpad3);
    const bool isNum4   = (key == sf::Keyboard::Key::Num4 || key == sf::Keyboard::Key::Numpad4);

    // ── 等待 Enter 确认（带 title 的 RANDOM_CHECK/CHECK，或 DISPLAY）──
    if (mWaitingForEnter) {
        if (!isEnter && !isEsc) return;

        mWaitingForEnter = false;

        if (node.type == EventNodeType::RANDOM_CHECK) {
            applyEffects(node, ctx);
            const int roll = static_cast<int>(mRng() % 100);
            const bool success = roll < node.probability;
            std::cout << "[Event] Random (on Enter): roll=" << roll
                      << " -> " << (success ? "success" : "failure") << std::endl;
            transitionTo(success ? node.successNode : node.failureNode, ctx);
            return;
        }
        if (node.type == EventNodeType::CHECK) {
            applyEffects(node, ctx);
            const bool passed = evaluateConditions(
                node.conditions, node.requireMode, ctx);
            const std::string& next = passed ? node.thenNode : node.elseNode;
            if (next.empty()) { clear(); return; }
            transitionTo(next, ctx);
            return;
        }
        if (node.type == EventNodeType::DISPLAY) {
            transitionTo(node.thenNode, ctx);
            return;
        }
        return;
    }

    // ── CHOICE ──────────────────────────────────────────────────────
    if (node.type == EventNodeType::CHOICE) {
        if (isEsc) return;
        int idx = -1;
        if (isNum1) idx = 0;
        else if (isNum2) idx = 1;
        else if (isNum3) idx = 2;
        else if (isNum4) idx = 3;
        // 在 2 选项时 Enter = 选 1
        if (idx == -1 && isEnter && node.options.size() <= 2) idx = 0;

        if (idx >= 0 && idx < static_cast<int>(node.options.size()))
            transitionTo(node.options[idx].nextNode, ctx);
        return;
    }

    // ── DISPLAY（兜底，通常走 mWaitingForEnter 分支）────────────────
    if (node.type == EventNodeType::DISPLAY) {
        if (isEnter || isEsc)
            transitionTo(node.thenNode, ctx);
    }
}

// ─── 渲染 ───────────────────────────────────────────────────────────────────

void EventRunner::render(sf::RenderWindow& window, ModalBox& modalBox) {
    if (!mActive || !mCurrentEvent) return;
    auto it = mCurrentEvent->steps.find(mCurrentNodeId);
    if (it == mCurrentEvent->steps.end()) return;

    const EventNode& node = it->second;
    std::string title   = localizedValue(node.titleKey, node.title);
    std::string body    = localizedValue(node.bodyKey, node.body);
    std::string footer  = localizedValue(node.footerKey, node.footer);

    if (title.empty() && body.empty()) return;

    switch (node.type) {
    case EventNodeType::CHOICE: {
        std::ostringstream oss;
        if (!body.empty()) oss << body << "\n\n";
        for (size_t i = 0; i < node.options.size(); ++i) {
            oss << "[" << (i + 1) << "] "
                << localizedValue(node.options[i].textKey, node.options[i].text);
            if (i + 1 < node.options.size()) oss << '\n';
        }
        body = oss.str();
        if (footer.empty()) {
            if (node.options.size() >= 4) footer = cls::text("prompt.choice1234");
            else footer = (node.options.size() >= 3 ? cls::text("prompt.choice123") : cls::text("prompt.choice12"));
        }
        break;
    }
    case EventNodeType::DISPLAY:
        if (footer.empty()) footer = cls::text("ui.press_enter_continue");
        break;
    default:
        if (mWaitingForEnter && footer.empty())
            footer = cls::text("ui.press_enter_continue");
        break;
    }

    modalBox.setContent(title, body, footer);
    modalBox.render(window);
}

// ─── 触发器轮询 ─────────────────────────────────────────────────────────────

bool EventRunner::checkTriggers(GameContext& ctx, int previousMinute) {
    const CampusPlace previousPlace = ctx.currentPlace;
    bool classAttendanceQueued = false;
    std::string classAttendanceId;

    for (const auto& id : mEventOrder) {
        auto it = mEvents.find(id);
        if (it == mEvents.end()) continue;
        auto& def = it->second;
        const auto& tr = def.trigger;
        if (tr.type == EventTrigger::TIME_SCHEDULE) {
            if (tr.method == "crossed_class_time") {
                if (!ctx.timeSystem.crossedClassTime(previousMinute)) continue;
                if (!tr.conditions.empty() && !evaluateConditions(tr.conditions, tr.requireMode, ctx)) continue;

                if (id == "class_attendance") {
                    classAttendanceQueued = true;
                    classAttendanceId = id;
                    continue;
                }

                if (!passesTriggerGate(def, ctx, mRng))
                    continue;

                ctx.timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
                ctx.timeSystem.markClassPrompted();
                ctx.activityNotice.clear();
                startEvent(id, ctx);
                return true;
            }
        }
    }

    if (classAttendanceQueued) {
        auto it = mEvents.find(classAttendanceId);
        if (it != mEvents.end()) {
            auto& def = it->second;
            if (passesTriggerGate(def, ctx, mRng)) {
                if (previousPlace == CampusPlace::Classroom) {
                    ctx.timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
                    ctx.timeSystem.markClassPrompted();
                    ctx.currentPlace = CampusPlace::Classroom;
                    ctx.currentMap = ctx.classroomMap;
                    ctx.player.setPosition(480.0f, 276.0f);
                    ctx.player.stopMovement();
                    ctx.activityNotice.clear();
                    startEvent(classAttendanceId, ctx);
                    return true;
                }

                ctx.timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
                ctx.timeSystem.markClassPrompted();
                ctx.activityNotice.clear();
                startEvent(classAttendanceId, ctx);
                return true;
            }
        }
    }
    return false;
}

// ─── 交互触发 ───────────────────────────────────────────────────────────────

bool EventRunner::triggerByAction(const std::string& actionId, GameContext& ctx) {
    const std::string normalized = normalizeActionId(actionId);
    for (const auto& id : mEventOrder) {
        auto it = mEvents.find(id);
        if (it == mEvents.end()) continue;
        auto& def = it->second;
        if (def.trigger.type != EventTrigger::INTERACTION) continue;
        if (def.trigger.actionId != actionId && def.trigger.actionId != normalized) continue;
        if (!def.trigger.conditions.empty() && !evaluateConditions(def.trigger.conditions, def.trigger.requireMode, ctx)) {
            continue;
        }
        if (!passesTriggerGate(def, ctx, mRng)) {
            if (def.trigger.fallbackToRegular)
                std::cout << "[Event] Skipped optional trigger: " << id << std::endl;
            continue;
        }
        std::cout << "[Event] Triggered by interaction: " << id
                  << " (action=" << actionId << ", normalized=" << normalized << ")" << std::endl;
        startEvent(id, ctx);
        return true;
    }
    return false;
}

// ─── 条件求值 ───────────────────────────────────────────────────────────────

bool EventRunner::evaluateConditions(const std::vector<Condition>& conditions,
                                      const std::string& requireMode,
                                      GameContext& ctx) {
    if (conditions.empty()) return true;

    if (requireMode == "any") {
        for (const auto& c : conditions)
            if (evaluateCondition(c, ctx)) return true;
        return false;
    }
    // 默认 "all" (AND)
    for (const auto& c : conditions)
        if (!evaluateCondition(c, ctx)) return false;
    return true;
}

bool EventRunner::evaluateCondition(const Condition& cond, GameContext& ctx) {
    const auto compareJson = [&](const json& actual) -> bool {
        const json expected = cond.valueJson.is_null() ? json(cond.value) : cond.valueJson;

        if (actual.is_number() && expected.is_number()) {
            const int lhs = actual.get<int>();
            const int rhs = expected.get<int>();
            if (cond.op == ">=") return lhs >= rhs;
            if (cond.op == "<=") return lhs <= rhs;
            if (cond.op == ">")  return lhs > rhs;
            if (cond.op == "<")  return lhs < rhs;
            if (cond.op == "eq") return lhs == rhs;
            if (cond.op == "neq") return lhs != rhs;
            return false;
        }

        if (actual.is_boolean() && expected.is_boolean()) {
            const bool lhs = actual.get<bool>();
            const bool rhs = expected.get<bool>();
            if (cond.op == "eq") return lhs == rhs;
            if (cond.op == "neq") return lhs != rhs;
            return false;
        }

        if (actual.is_string() && expected.is_string()) {
            const std::string lhs = actual.get<std::string>();
            const std::string rhs = expected.get<std::string>();
            if (cond.op == "eq") return lhs == rhs;
            if (cond.op == "neq") return lhs != rhs;
            return false;
        }

        return false;
    };

    switch (cond.kind) {
    case ConditionKind::LOCATION:
    {
        bool matches = false;
        if (cond.place == "classroom") matches = ctx.currentPlace == CampusPlace::Classroom;
        else if (cond.place == "dormitory") matches = ctx.currentPlace == CampusPlace::Dormitory;
        else if (cond.place == "cafeteria") matches = ctx.currentPlace == CampusPlace::Cafeteria;
        else if (cond.place == "gym")       matches = ctx.currentPlace == CampusPlace::Gym;
        else if (cond.place == "library")   matches = ctx.currentPlace == CampusPlace::Library;
        else if (cond.place == "campus")    matches = ctx.currentPlace == CampusPlace::Campus;
        else if (cond.place == "store")     matches = ctx.currentPlace == CampusPlace::Store;
        else return false;
        return cond.op == "neq" ? !matches : matches;
    }

    case ConditionKind::STAT:
    case ConditionKind::ENUM: {
        json actual;
        if (cond.stat == "energy") actual = ctx.player.getAttributes().energy;
        else if (cond.stat == "health") actual = ctx.player.getAttributes().health;
        else if (cond.stat == "gold") actual = ctx.player.getAttributes().gold;
        else if (cond.stat == "san") actual = ctx.player.getAttributes().san;
        else if (cond.stat == "academic") actual = ctx.player.getAttributes().academic;
        else if (cond.stat == "social") actual = ctx.player.getAttributes().social;
        else {
            const auto& hidden = ctx.player.getHidden();
            if (!hidden.contains(cond.stat)) return false;
            actual = hidden.at(cond.stat);
        }
        return compareJson(actual);
    }
    case ConditionKind::FLAG:
        if (cond.flag == "is_midterm_day")
            return ctx.timeSystem.isMidtermDay();
        return false;
    case ConditionKind::TIME: {
        if (cond.stat == "phase") {
            return compareJson(json(phaseName(ctx.timeSystem.currentPhase())));
        }
        if (cond.stat == "minute") {
            return compareJson(json(ctx.timeSystem.getMinuteOfDay() % (24 * 60)));
        }
        if (cond.stat == "day") {
            return compareJson(json(ctx.timeSystem.getDay()));
        }
        if (cond.stat == "meal_time") {
            return compareJson(json(ctx.timeSystem.isMealTime()));
        }
        return false;
    }
    case ConditionKind::GROUP:
        return evaluateConditions(cond.conditions, cond.requireMode.empty() ? "all" : cond.requireMode, ctx);
    case ConditionKind::META:
        return true;
    }
    return false;
}

// ─── 附属效果 ───────────────────────────────────────────────────────────────

void EventRunner::applyEffects(const EventNode& node, GameContext& ctx) {
    if (node.timeAdvanceMinutes > 0)
        ctx.timeSystem.advanceMinutes(node.timeAdvanceMinutes);
    const auto& d = node.delta;
    if (d.energy != 0 || d.health != 0 || d.gold != 0 ||
        d.san != 0 || d.academic != 0 || d.social != 0)
        ctx.player.modifyAttributes(node.delta);
    if (!node.hiddenDelta.is_null()) {
        mergeHidden(ctx.player.getHidden(), node.hiddenDelta);
    }
    const std::string flash = localizedValue(node.flashKey, node.flashText);
    if (!flash.empty())
        ctx.timeSkipFlash.start(flash);
}
