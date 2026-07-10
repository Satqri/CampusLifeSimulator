#ifndef CLS_UI_HELPPANEL_H
#define CLS_UI_HELPPANEL_H

#include "ui/UIComponent.h"
#include "ui/TguiTheme.h"
#include "ui/TguiContext.h"

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>

#include <functional>
#include <vector>

/**
 * @class HelpPanel
 * @brief 帮助界面组件，使用 TGUI Label 渲染。
 */
class HelpPanel : public UIComponent {
public:
    explicit HelpPanel(sf::Font& font);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    /// @brief 将 TGUI widget 添加到 gui（在 TguiContext 创建后调用）
    void attachToGui(TguiContext& ctx);

    /// @brief 显示/隐藏面板
    void setVisible(bool visible);

    /// @brief 关闭回调（由 main.cpp 注入）
    void setOnClose(std::function<void()> callback) { mOnClose = std::move(callback); }

    void setOverlayMode(bool overlayMode);

private:
    void createWidgets();

    sf::Font& mFont;
    TguiContext* mTguiCtx = nullptr;
    tgui::Panel::Ptr mContainer;
    tgui::Panel::Ptr mContentPanel;
    std::vector<tgui::Label::Ptr> mLabels;
    bool mOverlayMode = false;
    bool mVisible = false;
    bool mWidgetsCreated = false;
    std::function<void()> mOnClose;
};

#endif
