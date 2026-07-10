#ifndef CLS_UI_TIMEPANEL_H
#define CLS_UI_TIMEPANEL_H

#include <SFML/Graphics.hpp>
#include "ui/UIComponent.h"
#include "ui/TguiTheme.h"
#include "ui/TguiContext.h"

#include <TGUI/TGUI.hpp>

class TimeSystem;

class TimePanel : public UIComponent {
public:
    explicit TimePanel(sf::Font& font);
    void setTimeSystem(const TimeSystem* time);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void attachToGui(TguiContext& ctx);
    void setVisible(bool visible);

private:
    void createWidgets();
    void refreshDisplay();
    void toggle();

    sf::Font& mFont;
    const TimeSystem* mTimeSystem = nullptr;
    bool mExpanded = false;
    TguiContext* mTguiCtx = nullptr;

    tgui::Panel::Ptr mContainer;
    tgui::Panel::Ptr mCollapsedPanel;
    tgui::Label::Ptr mToggleLabel;
    tgui::Panel::Ptr mExpandedPanel;
    tgui::Label::Ptr mClockLabel;
    tgui::Label::Ptr mDayLabel;

    bool mVisible = false;
    bool mWidgetsCreated = false;
};

#endif
