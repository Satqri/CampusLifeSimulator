#include "ui/DifficultyPanel.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"

DifficultyPanel::DifficultyPanel(sf::Font& fontRef)
    : mFont(fontRef)
    , mCards{{
        Card{sf::FloatRect({118.0f, 144.0f}, {170.0f, 218.0f}),
            "difficulty.easy.title", "difficulty.easy.desc",
            Difficulty::Easy, sf::Color(34, 170, 128)},
        Card{sf::FloatRect({395.0f, 144.0f}, {170.0f, 218.0f}),
            "difficulty.normal.title", "difficulty.normal.desc",
            Difficulty::Normal, sf::Color(24, 154, 115)},
        Card{sf::FloatRect({672.0f, 144.0f}, {170.0f, 218.0f}),
            "difficulty.hard.title", "difficulty.hard.desc",
            Difficulty::Hard, sf::Color(206, 120, 80)}
    }}
{
}

void DifficultyPanel::attachToGui(TguiContext& ctx) {
    mTguiCtx = &ctx;
    createWidgets();
}

void DifficultyPanel::createWidgets() {
    if (mWidgetsCreated) return;
    mWidgetsCreated = true;

    using namespace cls::tgui_theme;

    mContainer = tgui::Panel::create({960, 540});
    mContainer->setVisible(false);
    mContainer->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0)); // transparent, SFML draws bg
    mTguiCtx->gui().add(mContainer);

    // 返回按钮
    mBackButton = createButton("<", 28);
    mBackButton->setSize({54, 54});
    mBackButton->setPosition({34, 34});
    mBackButton->getRenderer()->setBackgroundColor(tgui::Color(21, 158, 119));
    mBackButton->getRenderer()->setTextColor(kTextGold);
    mBackButton->onClick([this] {
        mSelectedIndex = -1;
        updateCardVisuals();
        if (mOnAction) mOnAction({DifficultyActionType::Back, Difficulty::Normal});
    });
    mContainer->add(mBackButton);

    // 标题和副标题
    mTitleLabel = createLabel(cls::text("difficulty.title"), 28);
    mTitleLabel->setPosition({320, 42});
    mTitleLabel->getRenderer()->setTextColor(tgui::Color(245, 235, 205));
    mContainer->add(mTitleLabel);

    mSubtitleLabel = createLabel(cls::text("difficulty.subtitle"), 18);
    mSubtitleLabel->setPosition({320, 78});
    mSubtitleLabel->getRenderer()->setTextColor(tgui::Color(224, 213, 188));
    mContainer->add(mSubtitleLabel);

    // 三张卡片（透明 button 用于点击检测，视觉效果由 render() 中 SFML 绘制）
    for (int i = 0; i < 3; ++i) {
        const auto& card = mCards[i];

        auto btn = tgui::Button::create();
        btn->setSize({card.bounds.size.x, card.bounds.size.y});
        btn->setPosition({card.bounds.position.x, card.bounds.position.y});
        btn->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
        btn->getRenderer()->setBackgroundColorHover(tgui::Color(255, 255, 255, 15));
        btn->getRenderer()->setBorders({0});
        btn->setText("");
        btn->onClick([this, i] {
            mSelectedIndex = i;
            updateCardVisuals();
            if (mOnAction) mOnAction({DifficultyActionType::Select, mCards[i].difficulty});
        });
        mCardButtons[i] = btn;
        mContainer->add(btn);

        // 卡片标题
        auto titleLabel = createLabel(cls::text(card.title), 18);
        titleLabel->setPosition({card.bounds.position.x + 14.0f, card.bounds.position.y + 110.0f});
        titleLabel->getRenderer()->setTextColor(tgui::Color(245, 237, 208));
        mCardTitleLabels[i] = titleLabel;
        mContainer->add(titleLabel);

        // 卡片描述
        auto descLabel = createLabel(cls::text(card.description), 13);
        descLabel->setPosition({card.bounds.position.x + 14.0f, card.bounds.position.y + 150.0f});
        descLabel->getRenderer()->setTextColor(tgui::Color(241, 233, 210));
        mCardDescLabels[i] = descLabel;
        mContainer->add(descLabel);
    }

    // 底部提示
    mNoteLabel = createLabel(cls::text("difficulty.note"), 16);
    mNoteLabel->setPosition({258, 470});
    mNoteLabel->getRenderer()->setTextColor(tgui::Color(225, 217, 198));
    mContainer->add(mNoteLabel);
}

void DifficultyPanel::updateCardVisuals() {
    for (int i = 0; i < 3; ++i) {
        tgui::Color borderColor = (i == mSelectedIndex) ? tgui::Color(255, 241, 170) : tgui::Color(224, 209, 160);
        float thickness = (i == mSelectedIndex) ? 4.0f : 2.0f;
        mCardButtons[i]->getRenderer()->setBorders({thickness});
        mCardButtons[i]->getRenderer()->setBorderColor(borderColor);
    }
    // 返回按钮选中状态
    tgui::Color backBorder = (mSelectedIndex == -1) ? tgui::Color(255, 236, 150) : tgui::Color(244, 239, 208);
    float backThick = (mSelectedIndex == -1) ? 3.0f : 1.0f;
    mBackButton->getRenderer()->setBorders({backThick});
    mBackButton->getRenderer()->setBorderColor(backBorder);
}

void DifficultyPanel::drawDecorative(sf::RenderWindow& window) const {
    // 背景
    sf::RectangleShape background({960.0f, 540.0f});
    background.setFillColor(sf::Color(58, 56, 43));
    window.draw(background);

    // 装饰性斑点
    for (int i = 0; i < 120; ++i) {
        sf::CircleShape speck(1.0f + static_cast<float>(i % 3) * 0.45f);
        speck.setPosition({
            static_cast<float>((i * 67) % 940),
            330.0f + static_cast<float>((i * 31) % 180)
        });
        speck.setFillColor(sf::Color(25, 31, 23, 130));
        window.draw(speck);
    }

    // 分隔线
    sf::RectangleShape leftRule({270.0f, 2.0f});
    leftRule.setPosition({118.0f, 116.0f});
    leftRule.setFillColor(sf::Color(160, 151, 124));
    window.draw(leftRule);

    sf::RectangleShape rightRule({270.0f, 2.0f});
    rightRule.setPosition({572.0f, 116.0f});
    rightRule.setFillColor(sf::Color(160, 151, 124));
    window.draw(rightRule);

    // 卡片装饰（强调色块 + 火柴人）
    for (int i = 0; i < static_cast<int>(mCards.size()); ++i) {
        const auto& card = mCards[i];
        const bool selected = (i == mSelectedIndex);

        // 选中发光效果
        if (selected) {
            for (int j = 3; j >= 1; --j) {
                const float expand = static_cast<float>(j) * 4.0f;
                sf::RectangleShape glow({card.bounds.size.x + expand * 2.0f, card.bounds.size.y + expand * 2.0f});
                glow.setPosition({card.bounds.position.x - expand, card.bounds.position.y - expand});
                glow.setFillColor(sf::Color(255, 238, 138, static_cast<std::uint8_t>(18 + j * 14)));
                window.draw(glow);
            }
        }

        // 强调色块
        sf::RectangleShape art({card.bounds.size.x, 104.0f});
        art.setPosition(card.bounds.position);
        art.setFillColor(card.accent);
        window.draw(art);

        // 标题条
        sf::RectangleShape nameStrip({card.bounds.size.x, 32.0f});
        nameStrip.setPosition({card.bounds.position.x, card.bounds.position.y + 104.0f});
        nameStrip.setFillColor(selected ? sf::Color(220, 188, 112) : sf::Color(188, 164, 105));
        window.draw(nameStrip);

        // 火柴人头部
        sf::CircleShape head(22.0f);
        head.setPosition({card.bounds.position.x + 64.0f, card.bounds.position.y + 22.0f});
        head.setFillColor(sf::Color(245, 232, 188));
        window.draw(head);

        // 火柴人身体
        sf::RectangleShape body({42.0f, 34.0f});
        body.setPosition({card.bounds.position.x + 65.0f, card.bounds.position.y + 62.0f});
        body.setFillColor(sf::Color(89, 72, 48));
        window.draw(body);
    }
}

void DifficultyPanel::update(float deltaTime) {
    (void)deltaTime;
}

void DifficultyPanel::render(sf::RenderWindow& window) {
    drawDecorative(window);
    // TGUI widgets 由 tguiCtx.draw() 绘制（window.display() 前调用）
}

void DifficultyPanel::setVisible(bool visible) {
    mVisible = visible;
    if (mContainer) mContainer->setVisible(visible);
}

void DifficultyPanel::setOnAction(std::function<void(DifficultyAction)> callback) {
    mOnAction = std::move(callback);
}

void DifficultyPanel::moveSelection(int delta) {
    if (mSelectedIndex == -1) {
        mSelectedIndex = delta > 0 ? 0 : static_cast<int>(mCards.size()) - 1;
    } else {
        mSelectedIndex += delta;
        if (mSelectedIndex < -1) mSelectedIndex = static_cast<int>(mCards.size()) - 1;
        else if (mSelectedIndex >= static_cast<int>(mCards.size())) mSelectedIndex = -1;
    }
    updateCardVisuals();
}

DifficultyAction DifficultyPanel::confirmSelection() const {
    if (mSelectedIndex == -1)
        return {DifficultyActionType::Back, Difficulty::Normal};
    return {DifficultyActionType::Select, mCards[mSelectedIndex].difficulty};
}
