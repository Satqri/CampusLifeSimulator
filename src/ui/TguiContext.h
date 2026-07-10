/**
 * @file TguiContext.h
 * @brief TGUI 适配层 — 封装 tgui::Gui 实例，统一管理 SFML 事件转发
 */

#ifndef CLS_UI_TGUICONTEXT_H
#define CLS_UI_TGUICONTEXT_H

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics.hpp>

class TguiContext {
public:
    explicit TguiContext(sf::RenderWindow& window);

    /// @brief 将 SFML 事件转发给 TGUI，返回 true 表示 TGUI 已消费该事件
    bool handleEvent(const sf::Event& event);

    /// @brief 渲染所有 TGUI widget（在 window.clear 之后、window.display 之前调用）
    void draw();

    /// @brief 获取 TGUI 根容器引用
    tgui::Gui& gui() { return mGui; }
    const tgui::Gui& gui() const { return mGui; }

private:
    tgui::Gui mGui;
};

#endif
