#ifndef CLS_UI_HUD_H
#define CLS_UI_HUD_H

#include "ui/UIComponent.h"
#include "ui/TguiTheme.h"
#include "ui/TguiContext.h"

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

class Player;

class HUD : public UIComponent {
public:
    explicit HUD(sf::Font& font);

    void setPlayer(const Player* player);
    void setPageName(const std::string& pageName);
    void setHelpText(const std::string& primary, const std::string& secondary);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void attachToGui(TguiContext& ctx);
    void setVisible(bool visible);

private:
    void createWidgets();
    void refreshBars();

    sf::Font& mFont;
    const Player* mPlayer = nullptr;
    std::string mPageName;
    std::string mPrimaryHelp;
    std::string mSecondaryHelp;
    TguiContext* mTguiCtx = nullptr;

    tgui::Panel::Ptr mContainer;
    tgui::Label::Ptr mStatsLabel;
    tgui::Label::Ptr mPageLabel;
    struct BarWidgets {
        tgui::Label::Ptr label;
        tgui::ProgressBar::Ptr bar;
        tgui::Label::Ptr valueLabel;
    };
    std::array<BarWidgets, 6> mBars;

    bool mVisible = false;
    bool mWidgetsCreated = false;
};

#endif
