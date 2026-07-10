#ifndef CLS_UI_SETTINGS_PANEL_H
#define CLS_UI_SETTINGS_PANEL_H

#include "core/GameSettings.h"
#include "ui/UIComponent.h"
#include "ui/TguiTheme.h"
#include "ui/TguiContext.h"

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>

enum class SettingsAction { None, Changed, Close };

class SettingsPanel : public UIComponent {
public:
    explicit SettingsPanel(sf::Font& font);

    void setSettings(cls::GameSettings* settings);
    void setOverlayMode(bool overlayMode);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void attachToGui(TguiContext& ctx);
    void setVisible(bool visible);
    void setOnAction(std::function<void(SettingsAction)> callback);

    bool isEditing() const { return false; }
    void setEditing(bool) {}

private:
    void createWidgets();
    void refreshValues();
    void onBgmChanged(float value);
    void onSfxChanged(float value);

    sf::Font& mFont;
    cls::GameSettings* mSettings = nullptr;
    TguiContext* mTguiCtx = nullptr;

    tgui::Panel::Ptr mContainer;
    tgui::Slider::Ptr mBgmSlider;
    tgui::Slider::Ptr mSfxSlider;
    tgui::Button::Ptr mWindowBtn;
    tgui::Button::Ptr mLangBtn;
    tgui::Button::Ptr mBackBtn;
    tgui::Label::Ptr mBgmValueLabel;
    tgui::Label::Ptr mSfxValueLabel;
    tgui::Label::Ptr mHintLabel;

    bool mOverlayMode = false;
    bool mVisible = false;
    bool mWidgetsCreated = false;
    std::function<void(SettingsAction)> mOnAction;
};

#endif
