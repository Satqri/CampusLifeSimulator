#ifndef CLS_UI_DEBUGSANDBOXPANEL_H
#define CLS_UI_DEBUGSANDBOXPANEL_H

#include "ui/UIComponent.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class EventRunner;
struct GameContext;

enum class DebugSandboxAction {
    None,
    TriggerEvent,
    TriggerEligibleEvent,
    SetSeed,
    AttributeChanged,
    ExtremeState,
    ClearLog
};

struct DebugSandboxResult {
    DebugSandboxAction action = DebugSandboxAction::None;
    std::string eventId;
    unsigned int seed = 0;
    std::string message;
};

class DebugSandboxPanel : public UIComponent {
public:
    explicit DebugSandboxPanel(sf::Font& font);

    bool isEnabled() const { return enabled; }
    bool isExpanded() const { return expanded; }
    void setEnabled(bool value);
    void toggleEnabled();

    void addLog(const std::string& message);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    DebugSandboxResult handleKey(sf::Keyboard::Key key, GameContext& ctx,
                                 EventRunner& eventRunner);
    void refresh(GameContext& ctx, EventRunner& eventRunner);

    enum class Page {
        Events,
        Attributes,
        Log
    };

private:
    void moveSelection(int delta, GameContext& ctx, EventRunner& eventRunner);
    DebugSandboxResult adjustCurrentAttribute(int delta, GameContext& ctx);
    DebugSandboxResult applyExtremeState(GameContext& ctx);
    DebugSandboxResult setSeed(EventRunner& eventRunner);
    DebugSandboxResult triggerSelected(GameContext& ctx, EventRunner& eventRunner,
                                       bool eligibleOnly);

    void renderBadge(sf::RenderWindow& window) const;
    void renderPanel(sf::RenderWindow& window);
    void renderEventsPage(sf::RenderWindow& window);
    void renderAttributesPage(sf::RenderWindow& window);
    void renderLogPage(sf::RenderWindow& window);
    void drawText(sf::RenderWindow& window, const std::string& text,
                  sf::Vector2f position, unsigned int size,
                  sf::Color color) const;
    void refreshSnapshot(GameContext& ctx, EventRunner& eventRunner);

    sf::Font& font;
    bool enabled = false;
    bool expanded = false;
    Page page = Page::Events;
    int selectedIndex = 0;
    unsigned int seedValue = 1;
    int seedStep = 0;
    std::vector<std::string> logs;

    std::vector<std::string> eventLines;
    std::vector<std::string> eventIds;
    std::vector<bool> eventEligible;
    std::vector<std::string> attributeLines;
    std::vector<std::string> hiddenLines;
    std::vector<std::string> logLines;
    std::string statusLine;
    std::string contextLine;
};

#endif
