/**
 * @file TguiTheme.h
 * @brief TGUI 主题工厂 — 创建统一样式的 widget
 */

#ifndef CLS_UI_TGUITHEME_H
#define CLS_UI_TGUITHEME_H

#include <TGUI/TGUI.hpp>

namespace cls::tgui_theme {

// ── 项目主色调 ──────────────────────────────────────────
inline constexpr tgui::Color kPanelBg{14, 24, 31};
inline constexpr tgui::Color kPanelOutline{230, 210, 148};
inline constexpr tgui::Color kTextLight{210, 210, 210};
inline constexpr tgui::Color kTextGold{230, 210, 148};
inline constexpr tgui::Color kTextDim{140, 140, 140};
inline constexpr tgui::Color kButtonBg{18, 30, 38};
inline constexpr tgui::Color kButtonHover{30, 50, 65};
inline constexpr tgui::Color kShade{0, 0, 0};
inline constexpr tgui::Color kButtonOutline{140, 130, 100};
inline constexpr tgui::Color kRed{220, 80, 80};
inline constexpr tgui::Color kGreen{80, 200, 80};
inline constexpr tgui::Color kYellow{220, 200, 80};
inline constexpr tgui::Color kBlue{80, 140, 220};
inline constexpr tgui::Color kPurple{160, 100, 200};

/// @brief 从文件路径加载全局字体
inline void loadGlobalFont(const tgui::String& path) {
    tgui::Font font(path);
    tgui::Font::setGlobalFont(font);
}

/// @brief 创建项目风格的 Button
inline tgui::Button::Ptr createButton(const tgui::String& text, unsigned int textSize = 16) {
    auto btn = tgui::Button::create(text);
    btn->getRenderer()->setTextSize(textSize);
    btn->getRenderer()->setTextColor(kTextLight);
    btn->getRenderer()->setBackgroundColor(kButtonBg);
    btn->getRenderer()->setBackgroundColorHover(kButtonHover);
    btn->getRenderer()->setBorderColor(kButtonOutline);
    btn->getRenderer()->setBorders({1.5f});
    btn->getRenderer()->setRoundedBorderRadius(4.0f);
    return btn;
}

/// @brief 创建项目风格的 Label
inline tgui::Label::Ptr createLabel(const tgui::String& text, unsigned int textSize = 14) {
    auto label = tgui::Label::create(text);
    label->getRenderer()->setTextSize(textSize);
    label->getRenderer()->setTextColor(kTextLight);
    return label;
}

/// @brief 创建半透明遮罩 Panel
inline tgui::Panel::Ptr createShade(float w = 960.0f, float h = 540.0f,
                                    tgui::Color color = kShade, uint8_t alpha = 105) {
    auto panel = tgui::Panel::create({w, h});
    panel->getRenderer()->setBackgroundColor(tgui::Color(color.getRed(), color.getGreen(), color.getBlue(), alpha));
    return panel;
}

} // namespace cls::tgui_theme

#endif
