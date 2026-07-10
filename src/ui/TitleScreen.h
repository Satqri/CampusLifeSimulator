#ifndef CLS_UI_TITLE_SCREEN_H
#define CLS_UI_TITLE_SCREEN_H

#include "ui/UIComponent.h"
#include "ui/TguiTheme.h"
#include "ui/TguiContext.h"

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>

#include <array>
#include <functional>
#include <string>

enum class TitleAction { None, Start, Settings, Help };

class TitleScreen : public UIComponent {
public:
    TitleScreen(sf::Font& font, const std::string& backgroundPath);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void attachToGui(TguiContext& ctx);
    void setVisible(bool visible);
    void setOnAction(std::function<void(TitleAction)> callback);

    void moveSelection(int delta);
    void setSelection(std::size_t index);
    TitleAction confirmSelection() const;

private:
    struct Button {
        sf::FloatRect bounds;
        std::string labelKey;
        TitleAction action;
        sf::Color fill;
        sf::Color outline;
    };

    void createWidgets();
    void updateSelectedButton();
    void drawAmbientEffects(sf::RenderWindow& window) const;
    void drawButtonGlow(sf::RenderWindow& window, const Button& button) const;

    sf::Font& mFont;
    TguiContext* mTguiCtx = nullptr;

    // SFML 装饰
    sf::Texture mBgTexture;
    float mElapsedTime = 0.0f;
    std::array<Button, 3> mButtons;
    std::size_t mSelectedIndex = 0;

    // TGUI widgets
    tgui::Panel::Ptr mContainer;
    std::array<tgui::Button::Ptr, 3> mTguiButtons;
    tgui::Label::Ptr mTitleLabel;
    tgui::Label::Ptr mSubtitleLabel;
    tgui::Label::Ptr mNavLabel;
    tgui::Label::Ptr mVersionLabel;

    bool mVisible = false;
    bool mWidgetsCreated = false;
    std::function<void(TitleAction)> mOnAction;
};

#endif
