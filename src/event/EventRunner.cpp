#include "event/EventRunner.h"

#include "core/GameContext.h"
#include "core/TimeSystem.h"
#include "entity/Player.h"
#include "ui/ModalBox.h"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <chrono>
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
    std::cerr << "[Event] Unknown condition kind: " << s << std::endl;
    return ConditionKind::LOCATION;
}

// ─── JSON 加载 ───────────────────────────────────────────────────────────────

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

        // 解析 steps
        const auto& steps = ev["steps"];
        for (auto it = steps.begin(); it != steps.end(); ++it) {
            EventNode node;
            const auto& n = it.value();
            node.type    = parseNodeType(n.value("type", ""));
            node.title   = n.value("title", "");
            node.body    = n.value("body", "");
            node.footer  = n.value("footer", "");

            switch (node.type) {
            case EventNodeType::CHOICE:
                for (const auto& opt : n["options"]) {
                    node.options.push_back({
                        opt.value("text", ""), opt.value("then", "")
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
                    Condition cond;
                    cond.kind  = parseConditionKind(c.value("kind", ""));
                    cond.place = c.value("place", "");
                    cond.stat  = c.value("stat", "");
                    cond.op    = c.value("op", "");
                    cond.value = c.value("value", 0);
                    cond.flag  = c.value("flag", "");
                    node.conditions.push_back(cond);
                }
                node.thenNode = n.value("then", "");
                node.elseNode = n.value("else", "");
                node.timeAdvanceMinutes = n.value("time_advance", 0);
                node.flashText          = n.value("flash", "");
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
    if (!node.title.empty() &&
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
        if (!node.title.empty() || !node.body.empty()) {
            std::ostringstream body;
            body << node.body;
            body << "\nCurrent time: " << ctx.timeSystem.clockText();
            ctx.activityNotice.show(node.title, body.str());
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

    // ── 等待 Enter 确认（带 title 的 RANDOM_CHECK/CHECK，或 DISPLAY）──
    if (mWaitingForEnter) {
        if (isEsc) {
            clear();
            return;
        }
        if (!isEnter) return;

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
        if (isEsc) {
            clear();
            return;
        }
        int idx = -1;
        if (isNum1) idx = 0;
        else if (isNum2) idx = 1;
        else if (isNum3) idx = 2;
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
    if (node.title.empty() && node.body.empty()) return;

    std::string title   = node.title;
    std::string body    = node.body;
    std::string footer  = node.footer;

    switch (node.type) {
    case EventNodeType::CHOICE: {
        std::ostringstream oss;
        if (!node.body.empty()) oss << node.body << "\n\n";
        for (size_t i = 0; i < node.options.size(); ++i) {
            oss << "[" << (i + 1) << "] " << node.options[i].text;
            if (i + 1 < node.options.size()) oss << '\n';
        }
        body = oss.str();
        if (footer.empty())
            footer = (node.options.size() >= 3 ? "Press 1, 2, or 3" : "Press 1 or 2");
        break;
    }
    case EventNodeType::DISPLAY:
        if (footer.empty()) footer = "Press Enter to continue";
        break;
    default:
        if (mWaitingForEnter && footer.empty())
            footer = "Press Enter to continue";
        break;
    }

    modalBox.setContent(title, body, footer);
    modalBox.render(window);
}

// ─── 触发器轮询 ─────────────────────────────────────────────────────────────

bool EventRunner::checkTriggers(GameContext& ctx, int previousMinute) {
    for (auto& [id, def] : mEvents) {
        const auto& tr = def.trigger;
        if (tr.type == EventTrigger::TIME_SCHEDULE) {
            if (tr.method == "crossed_class_time") {
                if (ctx.timeSystem.crossedClassTime(previousMinute)) {
                    ctx.timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
                    ctx.timeSystem.markClassPrompted();
                    ctx.currentPlace = CampusPlace::Classroom;
                    ctx.currentMap = ctx.classroomMap;
                    ctx.player.setPosition(480.0f, 276.0f);
                    ctx.player.stopMovement();
                    ctx.activityNotice.clear();
                    startEvent(id, ctx);
                    return true;
                }
            }
        }
    }
    return false;
}

// ─── 交互触发 ───────────────────────────────────────────────────────────────

bool EventRunner::triggerByAction(const std::string& actionId, GameContext& ctx) {
    for (auto& [id, def] : mEvents) {
        if (def.trigger.type == EventTrigger::INTERACTION
            && def.trigger.actionId == actionId) {
            std::cout << "[Event] Triggered by interaction: " << id
                      << " (action=" << actionId << ")" << std::endl;
            startEvent(id, ctx);
            return true;
        }
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
    switch (cond.kind) {
    case ConditionKind::LOCATION:
        if (cond.place == "classroom") return ctx.currentPlace == CampusPlace::Classroom;
        if (cond.place == "dormitory") return ctx.currentPlace == CampusPlace::Dormitory;
        if (cond.place == "cafeteria") return ctx.currentPlace == CampusPlace::Cafeteria;
        if (cond.place == "gym")       return ctx.currentPlace == CampusPlace::Gym;
        if (cond.place == "library")   return ctx.currentPlace == CampusPlace::Library;
        if (cond.place == "campus")    return ctx.currentPlace == CampusPlace::Campus;
        return false;

    case ConditionKind::STAT: {
        int val = 0;
        if (cond.stat == "energy")      val = ctx.player.getAttributes().energy;
        else if (cond.stat == "health") val = ctx.player.getAttributes().health;
        else if (cond.stat == "gold")   val = ctx.player.getAttributes().gold;
        else if (cond.stat == "san")    val = ctx.player.getAttributes().san;
        else if (cond.stat == "academic") val = ctx.player.getAttributes().academic;
        else if (cond.stat == "social") val = ctx.player.getAttributes().social;
        else return false;
        if (cond.op == ">=") return val >= cond.value;
        if (cond.op == "<")  return val < cond.value;
        return false;
    }
    case ConditionKind::FLAG:
        if (cond.flag == "is_midterm_day")
            return ctx.timeSystem.isMidtermDay();
        return false;
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
        auto& hidden = ctx.player.getHidden();
        for (auto it = node.hiddenDelta.begin(); it != node.hiddenDelta.end(); ++it) {
            if (it.value().is_number_integer()) {
                int cur = hidden.value(it.key(), 0);
                int add = it.value();
                hidden[it.key()] = cur + add;
            } else {
                hidden[it.key()] = it.value();
            }
        }
    }
    if (!node.flashText.empty())
        ctx.timeSkipFlash.start(node.flashText);
}
