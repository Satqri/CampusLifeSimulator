#include "ui/SettingsPanel.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"
#include "utils/AssetPath.h"

#include <algorithm>
#include <filesystem>

SettingsPanel::SettingsPanel(sf::Font& fontRef)
    : mFont(fontRef)
{
}

void SettingsPanel::setSettings(cls::GameSettings* value) {
    mSettings = value;
    if (value) refreshValues();
}

void SettingsPanel::setOverlayMode(bool value) {
    mOverlayMode = value;
    if (mContainer) {
        uint8_t alpha = value ? 190 : 255;
        mContainer->getRenderer()->setBackgroundColor(tgui::Color(14, 18, 28, alpha));
    }
}

void SettingsPanel::attachToGui(TguiContext& ctx) {
    mTguiCtx = &ctx;
    createWidgets();
}

void SettingsPanel::createWidgets() {
    if (mWidgetsCreated) return;
    mWidgetsCreated = true;

    using namespace cls::tgui_theme;

    // 全屏背景
    mContainer = tgui::Panel::create({960, 540});
    mContainer->getRenderer()->setBackgroundColor(tgui::Color(14, 18, 28));
    mContainer->setVisible(false);
    mTguiCtx->gui().add(mContainer);

    // 内容面板（尝试加载贴图）
    tgui::Panel::Ptr panelBg = tgui::Panel::create({760, 390});
    panelBg->setPosition({100, 78});
    panelBg->getRenderer()->setBackgroundColor(tgui::Color(20, 26, 36, 238));
    panelBg->getRenderer()->setBorders({2.0f});
    panelBg->getRenderer()->setBorderColor(tgui::Color(114, 138, 168));
    mContainer->add(panelBg);

    // 标题
    auto titleLabel = createLabel(cls::text("settings.title"), 30);
    titleLabel->setPosition({132, 96});
    titleLabel->getRenderer()->setTextColor(tgui::Color::White);
    mContainer->add(titleLabel);

    const float rowY = 154.0f;
    const float rowSpacing = 54.0f;
    const float labelX = 160.0f;
    const float sliderX = 380.0f;
    const float sliderW = 280.0f;

    // Row 0: BGM Volume
    {
        auto label = createLabel(cls::text("settings.bgm"), 22);
        label->setPosition({labelX, rowY + 12});
        label->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(label);

        mBgmSlider = tgui::Slider::create(0.0f, 100.0f);
        mBgmSlider->setPosition({sliderX, rowY + 6});
        mBgmSlider->setSize({sliderW, 24});
        mBgmSlider->setStep(5.0f);
        mBgmSlider->onValueChange([this](float v) { onBgmChanged(v); });
        mContainer->add(mBgmSlider);

        mBgmValueLabel = createLabel("50%", 20);
        mBgmValueLabel->setPosition({sliderX + sliderW + 10, rowY + 10});
        mBgmValueLabel->getRenderer()->setTextColor(tgui::Color(208, 223, 240));
        mContainer->add(mBgmValueLabel);
    }

    // Row 1: SFX Volume
    {
        auto label = createLabel(cls::text("settings.sfx"), 22);
        label->setPosition({labelX, rowY + rowSpacing + 12});
        label->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(label);

        mSfxSlider = tgui::Slider::create(0.0f, 100.0f);
        mSfxSlider->setPosition({sliderX, rowY + rowSpacing + 6});
        mSfxSlider->setSize({sliderW, 24});
        mSfxSlider->setStep(5.0f);
        mSfxSlider->onValueChange([this](float v) { onSfxChanged(v); });
        mContainer->add(mSfxSlider);

        mSfxValueLabel = createLabel("50%", 20);
        mSfxValueLabel->setPosition({sliderX + sliderW + 10, rowY + rowSpacing + 10});
        mSfxValueLabel->getRenderer()->setTextColor(tgui::Color(208, 223, 240));
        mContainer->add(mSfxValueLabel);
    }

    // Row 2: Window Size
    {
        auto label = createLabel(cls::text("settings.window"), 22);
        label->setPosition({labelX, rowY + rowSpacing * 2 + 12});
        label->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(label);

        mWindowBtn = createButton("", 20);
        mWindowBtn->setPosition({sliderX, rowY + rowSpacing * 2 + 4});
        mWindowBtn->setSize({sliderW + 50, 36});
        mWindowBtn->onClick([this] {
            if (!mSettings) return;
            mSettings->windowScaleIndex = (mSettings->windowScaleIndex + 1)
                % static_cast<int>(cls::windowScalePresets().size());
            refreshValues();
            if (mOnAction) mOnAction(SettingsAction::Changed);
        });
        mContainer->add(mWindowBtn);
    }

    // Row 3: Language
    {
        auto label = createLabel(cls::text("settings.language"), 22);
        label->setPosition({labelX, rowY + rowSpacing * 3 + 12});
        label->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(label);

        mLangBtn = createButton("", 20);
        mLangBtn->setPosition({sliderX, rowY + rowSpacing * 3 + 4});
        mLangBtn->setSize({sliderW + 50, 36});
        mLangBtn->onClick([this] {
            if (!mSettings) return;
            mSettings->language = (mSettings->language == cls::Language::Chinese)
                ? cls::Language::English : cls::Language::Chinese;
            refreshValues();
            if (mOnAction) mOnAction(SettingsAction::Changed);
        });
        mContainer->add(mLangBtn);
    }

    // Row 4: Back
    {
        auto label = createLabel(cls::text("settings.back"), 22);
        label->setPosition({labelX, rowY + rowSpacing * 4 + 12});
        label->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(label);

        mBackBtn = createButton("Back", 20);
        mBackBtn->setPosition({sliderX, rowY + rowSpacing * 4 + 4});
        mBackBtn->setSize({sliderW + 50, 36});
        mBackBtn->onClick([this] {
            if (mOnAction) mOnAction(SettingsAction::Close);
        });
        mContainer->add(mBackBtn);
    }

    // 导航提示
    mHintLabel = createLabel(cls::text("settings.nav"), 15);
    mHintLabel->setPosition({126, 430});
    mHintLabel->getRenderer()->setTextColor(tgui::Color(220, 228, 242));
    mContainer->add(mHintLabel);
}

void SettingsPanel::refreshValues() {
    if (!mSettings || !mBgmSlider) return;

    mBgmSlider->setValue(static_cast<float>(mSettings->bgmVolume));
    mSfxSlider->setValue(static_cast<float>(mSettings->sfxVolume));
    mBgmValueLabel->setText(std::to_string(mSettings->bgmVolume) + "%");
    mSfxValueLabel->setText(std::to_string(mSettings->sfxVolume) + "%");

    const auto& preset = cls::windowScalePresets()[mSettings->windowScaleIndex];
    mWindowBtn->setText(cls::text(preset.labelKey));

    mLangBtn->setText(cls::languageName(mSettings->language));
}

void SettingsPanel::onBgmChanged(float value) {
    if (!mSettings) return;
    mSettings->bgmVolume = static_cast<int>(value);
    mBgmValueLabel->setText(std::to_string(mSettings->bgmVolume) + "%");
    if (mOnAction) mOnAction(SettingsAction::Changed);
}

void SettingsPanel::onSfxChanged(float value) {
    if (!mSettings) return;
    mSettings->sfxVolume = static_cast<int>(value);
    mSfxValueLabel->setText(std::to_string(mSettings->sfxVolume) + "%");
    if (mOnAction) mOnAction(SettingsAction::Changed);
}

void SettingsPanel::update(float deltaTime) {
    (void)deltaTime;
}

void SettingsPanel::render(sf::RenderWindow& window) {
    (void)window;
}

void SettingsPanel::setVisible(bool visible) {
    mVisible = visible;
    if (mContainer) mContainer->setVisible(visible);
}

void SettingsPanel::setOnAction(std::function<void(SettingsAction)> callback) {
    mOnAction = std::move(callback);
}
