#ifndef CLS_UI_MODALBOX_H
#define CLS_UI_MODALBOX_H

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <memory>
#include <string>
#include "ui/UIComponent.h"

class TguiContext;

class ModalBox : public UIComponent {
public:
    explicit ModalBox(sf::Font& font);

    void setContent(const std::string& title, const std::string& body,
                    const std::string& footer);
    void setFullscreenTexture(const sf::Texture* texture);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void attachToGui(TguiContext& ctx);
    void setVisible(bool visible);

private:
    void createWidgets();
    void updateWidgetText();

    sf::Font& mFont;
    std::string mTitle;
    std::string mBody;
    std::string mFooter;

    const sf::Texture* mFullscreenTexture = nullptr;
    TguiContext* mTguiCtx = nullptr;

    tgui::Panel::Ptr mContainer;
    tgui::Panel::Ptr mShade;
    tgui::Panel::Ptr mPanel;
    tgui::Label::Ptr mTitleLabel;
    tgui::Label::Ptr mBodyLabel;
    tgui::Label::Ptr mFooterLabel;

    bool mVisible = false;
    bool mWidgetsCreated = false;
};

#endif
