#ifndef CLS_UI_SETTINGS_PANEL_H
#define CLS_UI_SETTINGS_PANEL_H

#include "core/GameSettings.h"
#include "ui/UIComponent.h"
#include "ui/TguiTheme.h"
#include "ui/TguiContext.h"

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>
#include <array>
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

    void moveSelection(int delta);
    SettingsAction adjustCurrent(int delta);
    SettingsAction confirmCurrent();

    bool isEditing() const { return false; }
    void setEditing(bool) {}

private:
    void createWidgets();
    void refreshText();
    void refreshValues();
    void updateSelectionVisuals();
    SettingsAction cycleWindowSize(int delta);
    SettingsAction toggleLanguage();
    void onBgmChanged(float value);
    void onSfxChanged(float value);

    sf::Font& mFont;
    cls::GameSettings* mSettings = nullptr;
    TguiContext* mTguiCtx = nullptr;

    tgui::Panel::Ptr mContainer;
    tgui::Label::Ptr mTitleLabel;
    std::array<tgui::Panel::Ptr, 5> mRowHighlights;
    std::array<tgui::Label::Ptr, 5> mRowLabels;
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
    bool mRefreshingValues = false;
    int mSelectedRow = 0;
    std::function<void(SettingsAction)> mOnAction;
};

#endif
