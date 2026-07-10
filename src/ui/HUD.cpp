#include "ui/HUD.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"
#include "entity/Player.h"

#include <algorithm>
#include <sstream>

HUD::HUD(sf::Font& font)
    : mFont(font)
{
}

void HUD::setPlayer(const Player* p) { mPlayer = p; }
void HUD::setPageName(const std::string& name) { mPageName = name; }
void HUD::setHelpText(const std::string& primary, const std::string& secondary) {
    mPrimaryHelp = primary;
    mSecondaryHelp = secondary;
}

void HUD::attachToGui(TguiContext& ctx) {
    mTguiCtx = &ctx;
    createWidgets();
}

void HUD::createWidgets() {
    if (mWidgetsCreated) return;
    mWidgetsCreated = true;

    using namespace cls::tgui_theme;

    // 顶部面板 (960x42)
    mContainer = tgui::Panel::create({960, 42});
    mContainer->getRenderer()->setBackgroundColor(tgui::Color(20, 20, 30, 230));
    mContainer->setVisible(false);
    mTguiCtx->gui().add(mContainer);

    // 状态文字
    mStatsLabel = createLabel("", 12);
    mStatsLabel->setPosition({8, 6});
    mStatsLabel->getRenderer()->setTextColor(tgui::Color(200, 220, 255));
    mContainer->add(mStatsLabel);

    // 页面名
    mPageLabel = createLabel("", 12);
    mPageLabel->setPosition({820, 6});
    mPageLabel->getRenderer()->setTextColor(tgui::Color(255, 200, 100));
    mContainer->add(mPageLabel);

    // 6 属性条
    constexpr float kSpacing = 152.0f;
    constexpr float kStartX = 8.0f;
    constexpr float kBarY = 25.0f;
    constexpr float kBarW = 96.0f;

    const std::array<std::string, 6> labels = {
        cls::text("hud.energy"), cls::text("hud.health"), cls::text("hud.gold"),
        cls::text("hud.san"), cls::text("hud.academic"), cls::text("hud.social")
    };
    const std::array<tgui::Color, 6> colors = {
        kGreen, kRed, kYellow, kBlue, kYellow, kPurple
    };

    for (int i = 0; i < 6; ++i) {
        float x = kStartX + kSpacing * i;

        // 标签
        auto label = createLabel(labels[i], 10);
        label->setPosition({x, kBarY});
        label->getRenderer()->setTextColor(tgui::Color(210, 215, 225));
        mContainer->add(label);
        mBars[i].label = label;

        // 进度条
        auto bar = tgui::ProgressBar::create();
        bar->setPosition({x + 34, kBarY + 3});
        bar->setSize({kBarW, 8});
        bar->setMinimum(0);
        bar->setMaximum(100);
        bar->getRenderer()->setBackgroundColor(tgui::Color(45, 48, 58));
        bar->getRenderer()->setFillColor(colors[i]);
        bar->getRenderer()->setTextSize(0);  // 隐藏进度条内的文字
        mContainer->add(bar);
        mBars[i].bar = bar;

        // 数值
        auto valLabel = createLabel("0", 10);
        valLabel->setPosition({x + 136, kBarY});
        valLabel->getRenderer()->setTextColor(tgui::Color(230, 235, 245));
        mContainer->add(valLabel);
        mBars[i].valueLabel = valLabel;
    }
}

void HUD::refreshBars() {
    if (!mPlayer) return;

    const auto& a = mPlayer->getAttributes();
    const auto& buffs = mPlayer->getCombatBuffs();

    // 状态文字
    std::ostringstream ss;
    ss << cls::text("hud.san_level") << ':' << mPlayer->getSanLevel()
       << "  |  " << cls::text("hud.buff") << ':'
       << (buffs.nextEventPositive ? cls::text("hud.win") : cls::text("hud.none"))
       << " d" << (buffs.nextRollModifier >= 0 ? "+" : "")
       << buffs.nextRollModifier;
    mStatsLabel->setText(ss.str());

    // 页面名
    mPageLabel->setText(mPageName);

    // 属性值
    const std::array<int, 6> values = {a.energy, a.health, a.gold, a.san, a.academic, a.social};

    for (int i = 0; i < 6; ++i) {
        int clamped = std::max(0, std::min(values[i], 100));
        mBars[i].bar->setValue(static_cast<unsigned int>(clamped));
        mBars[i].valueLabel->setText(std::to_string(clamped));
    }
}

void HUD::update(float deltaTime) {
    (void)deltaTime;
    refreshBars();
}

void HUD::render(sf::RenderWindow& window) {
    (void)window;
}

void HUD::setVisible(bool visible) {
    mVisible = visible;
    if (mContainer) mContainer->setVisible(visible);
}
