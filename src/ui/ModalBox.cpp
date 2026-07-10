#include "ui/ModalBox.h"
#include "ui/TguiContext.h"
#include "ui/TguiTheme.h"
#include "utils/TextUtils.h"
#include "map/MapPortal.h"

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
    // TGUI 不支持直接使用 sf::Texture，简化处理：只用遮罩
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

    // 半透明遮罩
    mShade = createShade(960, 540, kShade, 105);
    mContainer->add(mShade);

    // 居中弹窗面板
    mPanel = tgui::Panel::create({620, 178});
    mPanel->setPosition({190, 174});
    mPanel->getRenderer()->setBackgroundColor(tgui::Color(14, 24, 31, 235));
    mPanel->getRenderer()->setBorders({2.0f});
    mPanel->getRenderer()->setBorderColor(tgui::Color(230, 210, 148, 180));
    mContainer->add(mPanel);

    // 标题
    mTitleLabel = createLabel("", 22);
    mTitleLabel->setPosition({218, 196});
    mTitleLabel->getRenderer()->setTextColor(tgui::Color(250, 238, 200));
    mTitleLabel->setMaximumTextWidth(580.0f);
    mContainer->add(mTitleLabel);

    // 正文
    mBodyLabel = createLabel("", 15);
    mBodyLabel->setPosition({218, 238});
    mBodyLabel->getRenderer()->setTextColor(tgui::Color(218, 230, 220));
    mBodyLabel->setMaximumTextWidth(580.0f);
    mContainer->add(mBodyLabel);

    // 页脚
    mFooterLabel = createLabel("", 12);
    mFooterLabel->setPosition({218, 320});
    mFooterLabel->getRenderer()->setTextColor(tgui::Color(172, 184, 178));
    mFooterLabel->setMaximumTextWidth(580.0f);
    mContainer->add(mFooterLabel);
}

void ModalBox::updateWidgetText() {
    if (!mWidgetsCreated) return;
    mTitleLabel->setText(mTitle);
    mBodyLabel->setText(mBody);
    mFooterLabel->setText(mFooter);
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
