#include "ui/TitleScreen.h"
#include "utils/AssetPath.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"

#include <algorithm>
#include <array>
#include <cmath>

TitleScreen::TitleScreen(sf::Font& fontRef, const std::string& backgroundPath)
    : mFont(fontRef)
    , mButtons{{
        Button{sf::FloatRect({330.0f, 360.0f}, {300.0f, 88.0f}), "title.start", TitleAction::Start,
               sf::Color(20, 154, 115, 238), sf::Color(232, 219, 160)},
        Button{sf::FloatRect({772.0f, 92.0f}, {132.0f, 52.0f}), "title.settings", TitleAction::Settings,
               sf::Color(46, 126, 180, 238), sf::Color(214, 227, 255)},
        Button{sf::FloatRect({804.0f, 446.0f}, {100.0f, 50.0f}), "title.help", TitleAction::Help,
               sf::Color(188, 164, 105, 235), sf::Color(246, 232, 178)}
    }}
{
    const std::string resolvedPath = cls::resolveAssetPath(backgroundPath);
    mBgTexture.loadFromFile(resolvedPath);
}

void TitleScreen::attachToGui(TguiContext& ctx) {
    mTguiCtx = &ctx;
    createWidgets();
}

void TitleScreen::createWidgets() {
    if (mWidgetsCreated) return;
    mWidgetsCreated = true;

    using namespace cls::tgui_theme;

    mContainer = tgui::Panel::create({960, 540});
    mContainer->setVisible(false);
    mContainer->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
    mTguiCtx->gui().add(mContainer);

    // 标题和副标题
    mTitleLabel = createLabel(cls::text("title.name"), 58);
    mTitleLabel->setPosition({278, 188});
    mTitleLabel->getRenderer()->setTextColor(tgui::Color(246, 255, 235));
    mContainer->add(mTitleLabel);

    mSubtitleLabel = createLabel(cls::text("title.subtitle"), 18);
    mSubtitleLabel->setPosition({355, 270});
    mSubtitleLabel->getRenderer()->setTextColor(tgui::Color(225, 246, 215));
    mContainer->add(mSubtitleLabel);

    // 三个按钮（透明背景，视觉效果由 SFML render 绘制）
    for (std::size_t i = 0; i < mButtons.size(); ++i) {
        const auto& b = mButtons[i];
        auto btn = tgui::Button::create();
        btn->setSize({b.bounds.size.x, b.bounds.size.y});
        btn->setPosition({b.bounds.position.x, b.bounds.position.y});
        btn->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
        btn->getRenderer()->setBackgroundColorHover(tgui::Color(255, 255, 255, 12));
        btn->getRenderer()->setBorders({0});
        btn->setText("");
        btn->onClick([this, i] {
            mSelectedIndex = i;
            updateSelectedButton();
            if (mOnAction) mOnAction(mButtons[i].action);
        });
        mTguiButtons[i] = btn;
        mContainer->add(btn);
    }

    // 按钮文字（叠加在透明按钮上方）
    for (std::size_t i = 0; i < mButtons.size(); ++i) {
        const auto& b = mButtons[i];
        const unsigned int textSize = b.bounds.size.x >= 280.0f ? 28U : 18U;
        auto label = createLabel(cls::text(b.labelKey), textSize);
        label->setPosition({b.bounds.position.x + b.bounds.size.x / 2.0f,
                            b.bounds.position.y + b.bounds.size.y / 2.0f - 3.0f});
        label->getRenderer()->setTextColor(tgui::Color(248, 241, 214));
        label->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
        label->setVerticalAlignment(tgui::VerticalAlignment::Center);
        // 调整位置以居中
        label->setPosition({b.bounds.position.x, b.bounds.position.y});
        label->setSize({b.bounds.size.x, b.bounds.size.y});
        mContainer->add(label);
    }

    // 导航提示和版本号
    mNavLabel = createLabel(cls::text("title.nav"), 15);
    mNavLabel->setPosition({244, 468});
    mNavLabel->getRenderer()->setTextColor(tgui::Color(235, 244, 218, 220));
    mContainer->add(mNavLabel);

    mVersionLabel = createLabel("v0.2.0", 13);
    mVersionLabel->setPosition({28, 500});
    mVersionLabel->getRenderer()->setTextColor(tgui::Color(230, 244, 218, 210));
    mContainer->add(mVersionLabel);

    updateSelectedButton();
}

void TitleScreen::updateSelectedButton() {
    for (std::size_t i = 0; i < mButtons.size(); ++i) {
        const bool sel = (i == mSelectedIndex);
        // SFML render() 绘制完整按钮视觉效果（发光+底色+底部条），这里只设置 hover 标识
        mTguiButtons[i]->getRenderer()->setBackgroundColorHover(
            sel ? tgui::Color(255, 255, 255, 20) : tgui::Color(255, 255, 255, 12));
    }
}

void TitleScreen::update(float deltaTime) {
    mElapsedTime += deltaTime;
}

void TitleScreen::render(sf::RenderWindow& window) {
    // 背景图
    const auto size = mBgTexture.getSize();
    if (size.x > 0 && size.y > 0) {
        sf::Sprite bgSprite(mBgTexture);
        bgSprite.setScale({
            960.0f / static_cast<float>(size.x),
            540.0f / static_cast<float>(size.y)
        });
        window.draw(bgSprite);
    } else {
        sf::RectangleShape fallback({960.0f, 540.0f});
        fallback.setFillColor(sf::Color(12, 92, 76));
        window.draw(fallback);
    }

    // 暗色遮罩
    sf::RectangleShape shade({960.0f, 540.0f});
    shade.setFillColor(sf::Color(0, 20, 18, 70));
    window.draw(shade);

    // 氛围粒子
    drawAmbientEffects(window);

    // 按钮发光效果（在 TGUI 按钮下方绘制）
    for (std::size_t i = 0; i < mButtons.size(); ++i) {
        drawButtonGlow(window, mButtons[i]);
    }

    // 按钮底色（在 TGUI 按钮下方绘制，TGUI 按钮透明所以可见）
    for (std::size_t i = 0; i < mButtons.size(); ++i) {
        const auto& b = mButtons[i];
        const bool sel = (i == mSelectedIndex);

        sf::RectangleShape base(b.bounds.size);
        base.setPosition(b.bounds.position);
        base.setFillColor(b.fill);
        base.setOutlineColor(b.outline);
        base.setOutlineThickness(sel ? 4.0f : 2.0f);
        window.draw(base);

        // 底部色条
        sf::RectangleShape footer({b.bounds.size.x, std::min(18.0f, b.bounds.size.y * 0.28f)});
        footer.setPosition({b.bounds.position.x, b.bounds.position.y + b.bounds.size.y - footer.getSize().y});
        footer.setFillColor(sel ? sf::Color(255, 224, 130, 235) : sf::Color(182, 152, 98, 210));
        window.draw(footer);
    }
    // 按钮文字由 TGUI labels 绘制
}

void TitleScreen::drawButtonGlow(sf::RenderWindow& window, const Button& button) const {
    const bool selected = (&button == &mButtons[mSelectedIndex]);
    if (!selected) return;

    const float pulse = (std::sin(mElapsedTime * 3.0f) + 1.0f) * 0.5f;
    for (int i = 4; i >= 1; --i) {
        const float expand = static_cast<float>(i) * (3.8f + pulse * 2.1f);
        sf::RectangleShape glow({button.bounds.size.x + expand * 2.0f,
                                 button.bounds.size.y + expand * 2.0f});
        glow.setPosition({button.bounds.position.x - expand, button.bounds.position.y - expand});
        glow.setFillColor(sf::Color(255, 247, 160,
                                    static_cast<std::uint8_t>(16 + i * 10 + pulse * 30)));
        window.draw(glow);
    }
}

void TitleScreen::drawAmbientEffects(sf::RenderWindow& window) const {
    const std::array<sf::Vector2f, 11> seeds = {{
        {58.0f, 294.0f}, {162.0f, 122.0f}, {312.0f, 90.0f},
        {486.0f, 178.0f}, {604.0f, 76.0f}, {708.0f, 150.0f},
        {826.0f, 296.0f}, {886.0f, 96.0f}, {244.0f, 410.0f},
        {520.0f, 360.0f}, {752.0f, 414.0f}
    }};

    for (std::size_t i = 0; i < seeds.size(); ++i) {
        const float phase = mElapsedTime * (0.55f + static_cast<float>(i % 4) * 0.08f)
            + static_cast<float>(i) * 1.37f;
        const float driftX = std::sin(phase) * (4.0f + static_cast<float>(i % 3) * 2.0f);
        const float driftY = std::cos(phase * 0.8f) * (3.0f + static_cast<float>(i % 2) * 2.0f);
        const float p = (std::sin(phase * 1.35f) + 1.0f) * 0.5f;
        const float radius = 3.0f + static_cast<float>(i % 4) * 1.5f + p * 2.0f;

        sf::CircleShape glow(radius * 3.0f);
        glow.setOrigin({radius * 3.0f, radius * 3.0f});
        glow.setPosition({seeds[i].x + driftX, seeds[i].y + driftY});
        glow.setFillColor(sf::Color(88, 255, 195, static_cast<std::uint8_t>(18 + p * 18)));
        window.draw(glow);

        sf::CircleShape core(radius);
        core.setOrigin({radius, radius});
        core.setPosition({seeds[i].x + driftX, seeds[i].y + driftY});
        core.setFillColor(sf::Color(157, 255, 210, static_cast<std::uint8_t>(80 + p * 70)));
        window.draw(core);
    }

    const float breathe = (std::sin(mElapsedTime * 0.9f) + 1.0f) * 0.5f;
    sf::RectangleShape vignette({960.0f, 540.0f});
    vignette.setFillColor(sf::Color(0, 18, 18, static_cast<std::uint8_t>(18 + breathe * 16)));
    window.draw(vignette);
}

void TitleScreen::setVisible(bool visible) {
    mVisible = visible;
    if (mContainer) mContainer->setVisible(visible);
}

void TitleScreen::setOnAction(std::function<void(TitleAction)> callback) {
    mOnAction = std::move(callback);
}

void TitleScreen::moveSelection(int delta) {
    if (mButtons.empty()) return;
    const int total = static_cast<int>(mButtons.size());
    int next = static_cast<int>(mSelectedIndex) + delta;
    while (next < 0) next += total;
    next %= total;
    mSelectedIndex = static_cast<std::size_t>(next);
    updateSelectedButton();
}

void TitleScreen::setSelection(std::size_t index) {
    if (index < mButtons.size()) {
        mSelectedIndex = index;
        updateSelectedButton();
    }
}

TitleAction TitleScreen::confirmSelection() const {
    return mButtons[mSelectedIndex].action;
}
