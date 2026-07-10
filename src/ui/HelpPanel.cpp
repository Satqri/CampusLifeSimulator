#include "ui/HelpPanel.h"
#include "core/Localization.h"

HelpPanel::HelpPanel(sf::Font& font)
    : mFont(font)
{
}

void HelpPanel::attachToGui(TguiContext& ctx) {
    mTguiCtx = &ctx;
    createWidgets();
}

void HelpPanel::createWidgets() {
    if (mWidgetsCreated) return;
    mWidgetsCreated = true;

    using namespace cls::tgui_theme;

    // 全屏背景
    mContainer = tgui::Panel::create({960, 540});
    mContainer->getRenderer()->setBackgroundColor(tgui::Color(16, 18, 24));
    mContainer->setVisible(false);

    // 内容面板
    mContentPanel = tgui::Panel::create({820, 410});
    mContentPanel->setPosition({70, 70});
    mContentPanel->getRenderer()->setBackgroundColor(tgui::Color(24, 27, 38, 235));
    mContentPanel->getRenderer()->setBorders({1.0f});
    mContentPanel->getRenderer()->setBorderColor(tgui::Color(100, 110, 145));
    mContainer->add(mContentPanel);

    // 帮助文本行
    struct LineInfo { std::string key; float x; float y; unsigned int size; tgui::Color color; };
    const std::vector<LineInfo> lines = {
        {"help.title",                  100,  95,  30, tgui::Color::White},
        {"help.section.pages",          105, 150,  18, kTextGold},
        {"help.pages",                  125, 180,  15, kTextLight},
        {"help.section.explore",        105, 230,  18, kTextGold},
        {"help.move",                   125, 260,  15, kTextLight},
        {"help.click_move",             125, 285,  15, kTextLight},
        {"help.stress",                 125, 310,  15, kTextLight},
        {"help.combat",                 125, 335,  15, kTextLight},
        {"help.section.quest",          105, 380,  18, kTextGold},
        {"help.quest_enter",            125, 410,  15, kTextLight},
        {"help.quest_ud",               125, 435,  15, kTextLight},
        {"help.quest_chain",            125, 460,  15, kTextLight},
        {"help.global",                 125, 485,  15, tgui::Color(255, 238, 188)},
        {"notice.standard_route_title", 520, 150,  18, kTextGold},
        {"notice.standard_route_body",  520, 180,  14, kTextLight},
        {"help.close",                  125, 508,  14, kTextLight},
    };

    for (const auto& li : lines) {
        auto label = createLabel(cls::text(li.key), li.size);
        label->setPosition({li.x, li.y});
        label->getRenderer()->setTextColor(li.color);
        mContainer->add(label);
        mLabels.push_back(label);
    }

    // 点击任意位置关闭
    mContainer->onClick([this](tgui::Vector2f) {
        if (mOnClose) mOnClose();
    });

    mTguiCtx->gui().add(mContainer);
}

void HelpPanel::update(float deltaTime) {
    (void)deltaTime;
}

void HelpPanel::render(sf::RenderWindow& window) {
    (void)window;
}

void HelpPanel::setVisible(bool visible) {
    mVisible = visible;
    if (mContainer) mContainer->setVisible(visible);
}

void HelpPanel::setOverlayMode(bool value) {
    mOverlayMode = value;
    if (mContainer) {
        mContainer->getRenderer()->setBackgroundColor(
            tgui::Color(16, 18, 24, static_cast<std::uint8_t>(value ? 190 : 255)));
    }
}
