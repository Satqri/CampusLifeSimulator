#include "ui/ModalBox.h"
#include "ui/TguiContext.h"
#include "ui/TguiTheme.h"
#include "utils/TextUtils.h"
#include "map/MapPortal.h"

namespace {
constexpr float kPanelW = 620.0f;
constexpr float kPanelX = 190.0f;          // (960 - 620) / 2
constexpr float kPadX = 28.0f;             // 文字左边距（相对于面板）
constexpr float kPadTop = 22.0f;           // 标题顶部边距
constexpr float kPadBottom = 22.0f;        // 底部边距
constexpr float kLineGap = 8.0f;           // 元素间距
constexpr float kTextW = kPanelW - kPadX * 2;  // 文字最大宽度 = 564
}

ModalBox::ModalBox(sf::Font& font) : mFont(font) {}

void ModalBox::setContent(const std::string& title, const std::string& body,
                          const std::string& footer) {
    mTitle = title;
    mBody = body;
    mFooter = footer;
    updateWidgetText();
    setVisible(!mTitle.empty() || !mBody.empty());
}

void ModalBox::setFullscreenTexture(const sf::Texture* texture) {
    mFullscreenTexture = texture;
    (void)mFullscreenTexture;
}

void ModalBox::attachToGui(TguiContext& ctx) {
    mTguiCtx = &ctx;
    createWidgets();
}

void ModalBox::createWidgets() {
    if (mWidgetsCreated) return;
    mWidgetsCreated = true;

    using namespace cls::tgui_theme;

    mContainer = tgui::Panel::create({960, 540});
    mContainer->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
    mContainer->setVisible(false);
    mTguiCtx->gui().add(mContainer);

    mShade = createShade(960, 540, kShade, 105);
    mContainer->add(mShade);

    // 面板 — 初始大小占位，后续 updateWidgetText 里动态调整
    mPanel = tgui::Panel::create({kPanelW, 120});
    mPanel->setPosition({kPanelX, 180});
    mPanel->getRenderer()->setBackgroundColor(tgui::Color(14, 24, 31, 235));
    mPanel->getRenderer()->setBorders({2.0f});
    mPanel->getRenderer()->setBorderColor(tgui::Color(230, 210, 148, 180));
    mContainer->add(mPanel);

    // 标题
    mTitleLabel = createLabel("", 22);
    mTitleLabel->getRenderer()->setTextColor(tgui::Color(250, 238, 200));
    mTitleLabel->setMaximumTextWidth(kTextW);
    mContainer->add(mTitleLabel);

    // 正文 — 开 auto-size 让高度随文字增长
    mBodyLabel = createLabel("", 15);
    mBodyLabel->getRenderer()->setTextColor(tgui::Color(218, 230, 220));
    mBodyLabel->setMaximumTextWidth(kTextW);
    mBodyLabel->setAutoSize(true);
    mContainer->add(mBodyLabel);

    // 页脚
    mFooterLabel = createLabel("", 12);
    mFooterLabel->getRenderer()->setTextColor(tgui::Color(172, 184, 178));
    mFooterLabel->setMaximumTextWidth(kTextW);
    mContainer->add(mFooterLabel);
}

void ModalBox::updateWidgetText() {
    if (!mWidgetsCreated) return;

    mTitleLabel->setText(mTitle);
    mBodyLabel->setText(mBody);
    mFooterLabel->setText(mFooter);

    // 动态计算面板高度
    const float titleH = mTitle.empty() ? 0.0f : mTitleLabel->getSize().y;
    const float bodyH = mBody.empty() ? 0.0f : mBodyLabel->getSize().y;
    const float footerH = mFooter.empty() ? 0.0f : mFooterLabel->getSize().y;

    float curY = kPadTop;
    if (!mTitle.empty()) curY += titleH + kLineGap;
    if (!mBody.empty()) curY += bodyH + kLineGap;
    if (!mFooter.empty()) curY += footerH;
    const float panelH = curY + kPadBottom;

    // 面板调整大小并垂直居中（最高不超过 500）
    const float clampedH = std::min(panelH, 500.0f);
    mPanel->setSize({kPanelW, clampedH});
    mPanel->setPosition({kPanelX, (540.0f - clampedH) / 2.0f});

    // 逐行定位
    curY = kPadTop;
    if (!mTitle.empty()) {
        mTitleLabel->setPosition({kPanelX + kPadX, mPanel->getPosition().y + curY});
        curY += titleH + kLineGap;
    }
    if (!mBody.empty()) {
        mBodyLabel->setPosition({kPanelX + kPadX, mPanel->getPosition().y + curY});
        curY += bodyH + kLineGap;
    }
    if (!mFooter.empty()) {
        mFooterLabel->setPosition({kPanelX + kPadX, mPanel->getPosition().y + curY});
    }
}

void ModalBox::update(float deltaTime) {
    (void)deltaTime;
}

void ModalBox::render(sf::RenderWindow& window) {
    (void)window;
}

void ModalBox::setVisible(bool visible) {
    mVisible = visible;
    if (mContainer) mContainer->setVisible(visible);
}
