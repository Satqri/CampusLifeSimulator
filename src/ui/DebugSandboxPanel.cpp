#include "ui/DebugSandboxPanel.h"

#include "core/GameContext.h"
#include "core/TextUtils.h"
#include "event/EventRunner.h"
#include "entity/Player.h"
#include "map/BuildingInterior.h"
#include "map/MapPortal.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace {
constexpr std::size_t kMaxLogs = 28;
constexpr int kEventPageSize = 10;
constexpr int kHiddenPreviewCount = 10;

std::string boolText(bool value) {
    return value ? "Y" : "N";
}

std::string pageName(DebugSandboxPanel::Page page) {
    switch (page) {
        case DebugSandboxPanel::Page::Events: return "Events";
        case DebugSandboxPanel::Page::Attributes: return "Stats";
        case DebugSandboxPanel::Page::Log: return "Log";
    }
    return "Events";
}

std::string phaseName(TimePhase phase) {
    switch (phase) {
        case TimePhase::EarlyMorning: return "early_morning";
        case TimePhase::Noon: return "noon";
        case TimePhase::Afternoon: return "afternoon";
        case TimePhase::Evening: return "evening";
        case TimePhase::Night: return "night";
    }
    return "unknown";
}

std::string attrName(int index) {
    switch (index) {
        case 0: return "energy";
        case 1: return "health";
        case 2: return "gold";
        case 3: return "san";
        case 4: return "academic";
        case 5: return "social";
    }
    return "";
}

int attrValue(const Attributes& attributes, int index) {
    switch (index) {
        case 0: return attributes.energy;
        case 1: return attributes.health;
        case 2: return attributes.gold;
        case 3: return attributes.san;
        case 4: return attributes.academic;
        case 5: return attributes.social;
    }
    return 0;
}

Attributes attrDelta(int index, int delta) {
    Attributes value{};
    value.energy = 0;
    value.health = 0;
    value.gold = 0;
    value.san = 0;
    value.academic = 0;
    value.social = 0;

    switch (index) {
        case 0: value.energy = delta; break;
        case 1: value.health = delta; break;
        case 2: value.gold = delta; break;
        case 3: value.san = delta; break;
        case 4: value.academic = delta; break;
        case 5: value.social = delta; break;
    }
    return value;
}

std::string compactJsonValue(const nlohmann::json& value) {
    if (value.is_string()) return value.get<std::string>();
    if (value.is_boolean()) return value.get<bool>() ? "true" : "false";
    if (value.is_number_integer()) return std::to_string(value.get<int>());
    return value.dump();
}
}

DebugSandboxPanel::DebugSandboxPanel(sf::Font& fontRef)
    : font(fontRef) {
    addLog("Sandbox ready. F1 toggles, F2 opens panel.");
}

void DebugSandboxPanel::setEnabled(bool value) {
    enabled = value;
    if (!enabled) expanded = false;
}

void DebugSandboxPanel::toggleEnabled() {
    enabled = !enabled;
    if (!enabled) expanded = false;
    addLog(enabled ? "Debug sandbox enabled." : "Debug sandbox disabled.");
}

void DebugSandboxPanel::addLog(const std::string& message) {
    logs.push_back(message);
    if (logs.size() > kMaxLogs)
        logs.erase(logs.begin(), logs.begin() + static_cast<long long>(logs.size() - kMaxLogs));
}

void DebugSandboxPanel::update(float deltaTime) {
    (void)deltaTime;
}

void DebugSandboxPanel::refresh(GameContext& ctx, EventRunner& eventRunner) {
    if (!enabled) return;
    refreshSnapshot(ctx, eventRunner);
}

DebugSandboxResult DebugSandboxPanel::handleKey(sf::Keyboard::Key key, GameContext& ctx,
                                                EventRunner& eventRunner) {
    if (key == sf::Keyboard::Key::F1) {
        toggleEnabled();
        return {DebugSandboxAction::None, "", seedValue,
                enabled ? "Debug sandbox enabled" : "Debug sandbox disabled"};
    }

    if (!enabled) return {};

    if (key == sf::Keyboard::Key::F2) {
        expanded = !expanded;
        addLog(expanded ? "Panel opened." : "Panel collapsed.");
        return {};
    }

    if (!expanded) return {};

    refreshSnapshot(ctx, eventRunner);

    if (key == sf::Keyboard::Key::Tab) {
        if (page == Page::Events) page = Page::Attributes;
        else if (page == Page::Attributes) page = Page::Log;
        else page = Page::Events;
        selectedIndex = 0;
        return {};
    }
    if (key == sf::Keyboard::Key::Escape) {
        expanded = false;
        return {};
    }
    if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::W) {
        moveSelection(-1, ctx, eventRunner);
        return {};
    }
    if (key == sf::Keyboard::Key::Down || key == sf::Keyboard::Key::S) {
        moveSelection(1, ctx, eventRunner);
        return {};
    }
    if (key == sf::Keyboard::Key::Left || key == sf::Keyboard::Key::A) {
        if (page == Page::Attributes) return adjustCurrentAttribute(-5, ctx);
        if (page == Page::Events) {
            if (seedValue > 1) --seedValue;
            return {};
        }
    }
    if (key == sf::Keyboard::Key::Right || key == sf::Keyboard::Key::D) {
        if (page == Page::Attributes) return adjustCurrentAttribute(5, ctx);
        if (page == Page::Events) {
            ++seedValue;
            return {};
        }
    }
    if (key == sf::Keyboard::Key::Enter) {
        if (page == Page::Events) return triggerSelected(ctx, eventRunner, false);
        if (page == Page::Attributes) return adjustCurrentAttribute(10, ctx);
    }
    if (key == sf::Keyboard::Key::Space && page == Page::Events) {
        return triggerSelected(ctx, eventRunner, true);
    }
    if (key == sf::Keyboard::Key::R && page == Page::Events) {
        return setSeed(eventRunner);
    }
    if (key == sf::Keyboard::Key::E && page == Page::Attributes) {
        return applyExtremeState(ctx);
    }
    if (key == sf::Keyboard::Key::Delete && page == Page::Log) {
        logs.clear();
        eventRunner.clearDebugHistory();
        return {DebugSandboxAction::ClearLog, "", seedValue, "Debug log cleared"};
    }

    return {};
}

void DebugSandboxPanel::moveSelection(int delta, GameContext& ctx, EventRunner& eventRunner) {
    refreshSnapshot(ctx, eventRunner);
    int count = 0;
    if (page == Page::Events) count = static_cast<int>(eventIds.size());
    else if (page == Page::Attributes) count = 6;
    else count = static_cast<int>(logs.size());

    if (count <= 0) {
        selectedIndex = 0;
        return;
    }

    selectedIndex += delta;
    while (selectedIndex < 0) selectedIndex += count;
    selectedIndex %= count;
}

DebugSandboxResult DebugSandboxPanel::adjustCurrentAttribute(int delta, GameContext& ctx) {
    if (selectedIndex < 0 || selectedIndex >= 6) return {};

    const auto before = ctx.player.getAttributes();
    ctx.player.modifyAttributes(attrDelta(selectedIndex, delta));
    const auto after = ctx.player.getAttributes();

    std::ostringstream ss;
    ss << attrName(selectedIndex) << " " << attrValue(before, selectedIndex)
       << " -> " << attrValue(after, selectedIndex);
    const std::string message = ss.str();
    addLog(message);
    return {DebugSandboxAction::AttributeChanged, "", seedValue, message};
}

DebugSandboxResult DebugSandboxPanel::applyExtremeState(GameContext& ctx) {
    const auto& a = ctx.player.getAttributes();
    Attributes delta{};
    delta.energy = -a.energy;
    delta.health = -a.health;
    delta.gold = -a.gold;
    delta.san = -a.san;
    delta.academic = -a.academic;
    delta.social = -a.social;
    ctx.player.modifyAttributes(delta);

    const std::string message = "Extreme low state applied.";
    addLog(message);
    return {DebugSandboxAction::ExtremeState, "", seedValue, message};
}

DebugSandboxResult DebugSandboxPanel::setSeed(EventRunner& eventRunner) {
    eventRunner.setRandomSeed(seedValue);
    std::ostringstream ss;
    ss << "Random seed set to " << seedValue;
    const std::string message = ss.str();
    addLog(message);
    return {DebugSandboxAction::SetSeed, "", seedValue, message};
}

DebugSandboxResult DebugSandboxPanel::triggerSelected(GameContext& ctx, EventRunner& eventRunner,
                                                      bool eligibleOnly) {
    refreshSnapshot(ctx, eventRunner);
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(eventIds.size())) return {};
    if (eligibleOnly && !eventEligible[selectedIndex]) {
        const std::string message = "Selected event is not eligible in current context.";
        addLog(message);
        return {DebugSandboxAction::None, "", seedValue, message};
    }

    const std::string id = eventIds[selectedIndex];
    if (!eventRunner.startEvent(id, ctx)) {
        const std::string message = "Failed to start event: " + id;
        addLog(message);
        return {DebugSandboxAction::None, id, seedValue, message};
    }

    const std::string message = "Forced event: " + id;
    addLog(message);
    expanded = false;
    return {eligibleOnly ? DebugSandboxAction::TriggerEligibleEvent : DebugSandboxAction::TriggerEvent,
            id, seedValue, message};
}

void DebugSandboxPanel::render(sf::RenderWindow& window) {
    if (!enabled) return;
    if (!expanded) {
        renderBadge(window);
        return;
    }
    renderPanel(window);
}

void DebugSandboxPanel::renderBadge(sf::RenderWindow& window) const {
    sf::RectangleShape badge({210.0f, 24.0f});
    badge.setPosition({742.0f, 48.0f});
    badge.setFillColor(sf::Color(24, 36, 42, 220));
    badge.setOutlineColor(sf::Color(248, 214, 110, 210));
    badge.setOutlineThickness(1.5f);
    window.draw(badge);

    drawText(window, "DEBUG SANDBOX ON  F2", {752.0f, 53.0f}, 11,
             sf::Color(255, 232, 150));
}

void DebugSandboxPanel::renderPanel(sf::RenderWindow& window) {
    sf::RectangleShape shade({960.0f, 540.0f});
    shade.setFillColor(sf::Color(0, 0, 0, 78));
    window.draw(shade);

    sf::RectangleShape panel({382.0f, 476.0f});
    panel.setPosition({562.0f, 50.0f});
    panel.setFillColor(sf::Color(13, 20, 26, 240));
    panel.setOutlineColor(sf::Color(238, 209, 128, 210));
    panel.setOutlineThickness(2.0f);
    window.draw(panel);

    std::ostringstream title;
    title << "Debug Sandbox / " << pageName(page);
    drawText(window, title.str(), {582.0f, 68.0f}, 18, sf::Color(255, 238, 190));
    drawText(window, contextLine, {582.0f, 96.0f}, 11, sf::Color(196, 220, 224));
    drawText(window, statusLine, {582.0f, 114.0f}, 11, sf::Color(208, 205, 180));

    if (page == Page::Events) renderEventsPage(window);
    else if (page == Page::Attributes) renderAttributesPage(window);
    else renderLogPage(window);

    drawText(window,
             "Tab page  Up/Down select  Esc close  F1 off",
             {582.0f, 500.0f}, 10, sf::Color(158, 175, 178));
}

void DebugSandboxPanel::renderEventsPage(sf::RenderWindow& window) {
    int start = 0;
    if (selectedIndex >= kEventPageSize)
        start = selectedIndex - kEventPageSize + 1;

    drawText(window, "Enter force  Space eligible-only  R seed  Left/Right seed",
             {582.0f, 138.0f}, 10, sf::Color(176, 198, 204));

    std::ostringstream seed;
    seed << "seed=" << seedValue;
    drawText(window, seed.str(), {812.0f, 138.0f}, 10, sf::Color(255, 226, 146));

    const int end = std::min(static_cast<int>(eventLines.size()), start + kEventPageSize);
    for (int i = start; i < end; ++i) {
        const float y = 162.0f + static_cast<float>(i - start) * 30.0f;
        const bool selected = i == selectedIndex;
        if (selected) {
            sf::RectangleShape row({340.0f, 25.0f});
            row.setPosition({582.0f, y - 3.0f});
            row.setFillColor(sf::Color(51, 75, 82, 220));
            row.setOutlineColor(sf::Color(255, 232, 146, 210));
            row.setOutlineThickness(1.0f);
            window.draw(row);
        }
        drawText(window, eventLines[i], {590.0f, y}, 10,
                 eventEligible[i] ? sf::Color(217, 244, 214) : sf::Color(206, 195, 178));
    }

    if (eventLines.empty()) {
        drawText(window, "No loaded events.", {590.0f, 164.0f}, 12,
                 sf::Color(224, 210, 190));
    }
}

void DebugSandboxPanel::renderAttributesPage(sf::RenderWindow& window) {
    drawText(window, "Left/Right +/-5  Enter +10  E extreme-low",
             {582.0f, 138.0f}, 10, sf::Color(176, 198, 204));
    for (int i = 0; i < static_cast<int>(attributeLines.size()); ++i) {
        const float y = 166.0f + static_cast<float>(i) * 28.0f;
        if (i == selectedIndex) {
            sf::RectangleShape row({214.0f, 24.0f});
            row.setPosition({582.0f, y - 3.0f});
            row.setFillColor(sf::Color(51, 75, 82, 220));
            row.setOutlineColor(sf::Color(255, 232, 146, 210));
            row.setOutlineThickness(1.0f);
            window.draw(row);
        }
        drawText(window, attributeLines[i], {590.0f, y}, 12, sf::Color(226, 235, 230));
    }

    drawText(window, "Hidden preview", {582.0f, 348.0f}, 12, sf::Color(255, 238, 190));
    for (int i = 0; i < static_cast<int>(hiddenLines.size()); ++i) {
        const float y = 374.0f + static_cast<float>(i) * 19.0f;
        drawText(window, hiddenLines[i], {590.0f, y}, 9, sf::Color(196, 216, 210));
    }
}

void DebugSandboxPanel::renderLogPage(sf::RenderWindow& window) {
    drawText(window, "Delete clear log", {582.0f, 138.0f}, 10,
             sf::Color(176, 198, 204));
    const int visible = 12;
    int start = std::max(0, static_cast<int>(logLines.size()) - visible);
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(logLines.size())) {
        if (selectedIndex < start) start = selectedIndex;
        if (selectedIndex >= start + visible) start = selectedIndex - visible + 1;
    }
    const int end = std::min(static_cast<int>(logLines.size()), start + visible);
    for (int i = start; i < end; ++i) {
        const float y = 164.0f + static_cast<float>(i - start) * 26.0f;
        drawText(window, logLines[i], {590.0f, y}, 10, sf::Color(220, 228, 222));
    }
    if (logLines.empty()) {
        drawText(window, "Log is empty.", {590.0f, 164.0f}, 12,
                 sf::Color(224, 210, 190));
    }
}

void DebugSandboxPanel::drawText(sf::RenderWindow& window, const std::string& text,
                                 sf::Vector2f position, unsigned int size,
                                 sf::Color color) const {
    sf::Text value = cls::makeText(font, text, size);
    value.setFillColor(color);
    value.setPosition(position);
    window.draw(value);
}

void DebugSandboxPanel::refreshSnapshot(GameContext& ctx, EventRunner& eventRunner) {
    eventLines.clear();
    eventIds.clear();
    eventEligible.clear();
    attributeLines.clear();
    hiddenLines.clear();
    logLines.clear();

    const auto& attributes = ctx.player.getAttributes();
    for (int i = 0; i < 6; ++i) {
        std::ostringstream ss;
        ss << std::left << std::setw(10) << attrName(i)
           << " " << std::setw(4) << attrValue(attributes, i);
        attributeLines.push_back(ss.str());
    }

    const auto events = eventRunner.debugEvents(ctx);
    for (const auto& info : events) {
        std::ostringstream ss;
        ss << (info.eligible ? "[OK] " : "[--] ")
           << info.id;
        if (ss.str().size() < 28)
            ss << std::string(28 - ss.str().size(), ' ');
        ss << " " << info.triggerLabel
           << " c" << info.chance
           << " cond:" << boolText(info.conditionsPass)
           << " gate:" << boolText(info.gatePass);
        if (!info.reason.empty())
            ss << " " << info.reason;
        eventLines.push_back(ss.str());
        eventIds.push_back(info.id);
        eventEligible.push_back(info.eligible);
    }

    for (const auto& line : logs) {
        logLines.push_back("sandbox: " + line);
    }
    for (const auto& line : eventRunner.debugHistory()) {
        logLines.push_back("event: " + line);
    }

    int selectionCount = 0;
    if (page == Page::Events) selectionCount = static_cast<int>(eventIds.size());
    else if (page == Page::Attributes) selectionCount = 6;
    else selectionCount = static_cast<int>(logLines.size());

    if (selectionCount > 0)
        selectedIndex = std::clamp(selectedIndex, 0, selectionCount - 1);
    else
        selectedIndex = 0;

    std::ostringstream context;
    context << placeName(ctx.currentPlace)
            << "  " << ctx.timeSystem.clockText()
            << "  phase=" << phaseName(ctx.timeSystem.currentPhase());
    contextLine = context.str();

    std::ostringstream status;
    status << "pos=(" << static_cast<int>(ctx.player.getPosition().x)
           << "," << static_cast<int>(ctx.player.getPosition().y) << ")"
           << " enemies=" << ctx.activeEnemies.size()
           << " currentEvent="
           << (eventRunner.isActive() ? eventRunner.currentEventId() : "-")
           << " node="
           << (eventRunner.isActive() ? eventRunner.currentNodeId() : "-");
    statusLine = status.str();

    if (page != Page::Attributes) return;

    const auto& hidden = ctx.player.getHidden();
    int printed = 0;
    for (auto it = hidden.begin(); it != hidden.end() && printed < kHiddenPreviewCount; ++it) {
        std::ostringstream ss;
        ss << it.key() << "=" << compactJsonValue(it.value());
        hiddenLines.push_back(ss.str());
        ++printed;
    }
}
