#include "ui/SettingsPanel.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"
#include "utils/AssetPath.h"

#include <algorithm>
#include <filesystem>

namespace {
constexpr int kRowCount = 5;
constexpr int kBgmRow = 0;
constexpr int kSfxRow = 1;
constexpr int kWindowRow = 2;
constexpr int kLanguageRow = 3;
constexpr int kBackRow = 4;
}

SettingsPanel::SettingsPanel(sf::Font& fontRef)
    : mFont(fontRef)
{
}

void SettingsPanel::setSettings(cls::GameSettings* value) {
    mSettings = value;
    if (value) {
        refreshText();
        refreshValues();
    }
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
    mTitleLabel = createLabel("", 30);
    mTitleLabel->setPosition({132, 96});
    mTitleLabel->getRenderer()->setTextColor(tgui::Color::White);
    mContainer->add(mTitleLabel);

    const float rowY = 154.0f;
    const float rowSpacing = 54.0f;
    const float labelX = 160.0f;
    const float sliderX = 380.0f;
    const float sliderW = 280.0f;

    for (int i = 0; i < kRowCount; ++i) {
        auto highlight = tgui::Panel::create({704, 44});
        highlight->setPosition({128.0f, rowY + rowSpacing * static_cast<float>(i)});
        highlight->getRenderer()->setBackgroundColor(tgui::Color(0, 0, 0, 0));
        highlight->getRenderer()->setBorders({0});
        mRowHighlights[i] = highlight;
        mContainer->add(highlight);
    }

    // Row 0: BGM Volume
    {
        mRowLabels[0] = createLabel("", 22);
        mRowLabels[0]->setPosition({labelX, rowY + 12});
        mRowLabels[0]->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(mRowLabels[0]);

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
        mRowLabels[1] = createLabel("", 22);
        mRowLabels[1]->setPosition({labelX, rowY + rowSpacing + 12});
        mRowLabels[1]->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(mRowLabels[1]);

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
        mRowLabels[2] = createLabel("", 22);
        mRowLabels[2]->setPosition({labelX, rowY + rowSpacing * 2 + 12});
        mRowLabels[2]->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(mRowLabels[2]);

        mWindowBtn = createButton("", 20);
        mWindowBtn->setPosition({sliderX, rowY + rowSpacing * 2 + 4});
        mWindowBtn->setSize({sliderW + 50, 36});
        mWindowBtn->onClick([this] {
            mSelectedRow = kWindowRow;
            updateSelectionVisuals();
            const SettingsAction action = cycleWindowSize(1);
            if (mOnAction && action != SettingsAction::None) mOnAction(action);
        });
        mContainer->add(mWindowBtn);
    }

    // Row 3: Language
    {
        mRowLabels[3] = createLabel("", 22);
        mRowLabels[3]->setPosition({labelX, rowY + rowSpacing * 3 + 12});
        mRowLabels[3]->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(mRowLabels[3]);

        mLangBtn = createButton("", 20);
        mLangBtn->setPosition({sliderX, rowY + rowSpacing * 3 + 4});
        mLangBtn->setSize({sliderW + 50, 36});
        mLangBtn->onClick([this] {
            mSelectedRow = kLanguageRow;
            updateSelectionVisuals();
            const SettingsAction action = toggleLanguage();
            if (mOnAction && action != SettingsAction::None) mOnAction(action);
        });
        mContainer->add(mLangBtn);
    }

    // Row 4: Back
    {
        mRowLabels[4] = createLabel("", 22);
        mRowLabels[4]->setPosition({labelX, rowY + rowSpacing * 4 + 12});
        mRowLabels[4]->getRenderer()->setTextColor(tgui::Color(238, 241, 248));
        mContainer->add(mRowLabels[4]);

        mBackBtn = createButton("", 20);
        mBackBtn->setPosition({sliderX, rowY + rowSpacing * 4 + 4});
        mBackBtn->setSize({sliderW + 50, 36});
        mBackBtn->onClick([this] {
            mSelectedRow = kBackRow;
            updateSelectionVisuals();
            if (mOnAction) mOnAction(SettingsAction::Close);
        });
        mContainer->add(mBackBtn);
    }

    // 导航提示
    mHintLabel = createLabel(cls::text("settings.nav"), 15);
    mHintLabel->setPosition({126, 430});
    mHintLabel->getRenderer()->setTextColor(tgui::Color(220, 228, 242));
    mContainer->add(mHintLabel);

    refreshText();
    refreshValues();
    updateSelectionVisuals();
}

void SettingsPanel::refreshText() {
    if (!mTitleLabel) return;

    mTitleLabel->setText(cls::text("settings.title"));

    const std::array<const char*, 5> rowKeys = {
        "settings.bgm",
        "settings.sfx",
        "settings.window",
        "settings.language",
        "settings.back"
    };
    for (std::size_t i = 0; i < rowKeys.size(); ++i) {
        if (mRowLabels[i]) mRowLabels[i]->setText(cls::text(rowKeys[i]));
    }

    if (mBackBtn) mBackBtn->setText(cls::text("settings.back"));
    if (mHintLabel) mHintLabel->setText(cls::text("settings.nav"));
}

void SettingsPanel::refreshValues() {
    if (!mSettings || !mBgmSlider) return;

    mRefreshingValues = true;
    mBgmSlider->setValue(static_cast<float>(mSettings->bgmVolume));
    mSfxSlider->setValue(static_cast<float>(mSettings->sfxVolume));
    mRefreshingValues = false;
    mBgmValueLabel->setText(std::to_string(mSettings->bgmVolume) + "%");
    mSfxValueLabel->setText(std::to_string(mSettings->sfxVolume) + "%");

    const auto& preset = cls::windowScalePresets()[mSettings->windowScaleIndex];
    mWindowBtn->setText(cls::text(preset.labelKey));

    mLangBtn->setText(cls::languageName(mSettings->language));
    updateSelectionVisuals();
}

void SettingsPanel::updateSelectionVisuals() {
    const tgui::Color selectedBg(50, 67, 94, 218);
    const tgui::Color selectedBorder(255, 232, 145, 235);
    const tgui::Color idleText(238, 241, 248);
    const tgui::Color selectedText(255, 238, 176);
    const tgui::Color valueIdle(208, 223, 240);
    const tgui::Color buttonIdleBorder(140, 130, 100);
    const tgui::Color buttonIdleText(210, 210, 210);

    for (int i = 0; i < kRowCount; ++i) {
        const bool selected = i == mSelectedRow;
        if (mRowHighlights[i]) {
            mRowHighlights[i]->getRenderer()->setBackgroundColor(
                selected ? selectedBg : tgui::Color(0, 0, 0, 0));
            mRowHighlights[i]->getRenderer()->setBorders({selected ? 2.0f : 0.0f});
            mRowHighlights[i]->getRenderer()->setBorderColor(selectedBorder);
        }
        if (mRowLabels[i]) {
            mRowLabels[i]->getRenderer()->setTextColor(selected ? selectedText : idleText);
        }
    }

    if (mBgmValueLabel) {
        mBgmValueLabel->getRenderer()->setTextColor(
            mSelectedRow == kBgmRow ? selectedText : valueIdle);
    }
    if (mSfxValueLabel) {
        mSfxValueLabel->getRenderer()->setTextColor(
            mSelectedRow == kSfxRow ? selectedText : valueIdle);
    }

    auto setButtonVisual = [&](const tgui::Button::Ptr& button, int row) {
        if (!button) return;
        const bool selected = mSelectedRow == row;
        button->getRenderer()->setBorders({selected ? 2.5f : 1.5f});
        button->getRenderer()->setBorderColor(selected ? selectedBorder : buttonIdleBorder);
        button->getRenderer()->setTextColor(selected ? selectedText : buttonIdleText);
    };
    setButtonVisual(mWindowBtn, kWindowRow);
    setButtonVisual(mLangBtn, kLanguageRow);
    setButtonVisual(mBackBtn, kBackRow);
}

SettingsAction SettingsPanel::cycleWindowSize(int delta) {
    if (!mSettings) return SettingsAction::None;
    const int count = static_cast<int>(cls::windowScalePresets().size());
    if (count <= 0) return SettingsAction::None;

    int next = mSettings->windowScaleIndex + delta;
    while (next < 0) next += count;
    next %= count;
    if (next == mSettings->windowScaleIndex) return SettingsAction::None;

    mSettings->windowScaleIndex = next;
    refreshValues();
    return SettingsAction::Changed;
}

SettingsAction SettingsPanel::toggleLanguage() {
    if (!mSettings) return SettingsAction::None;
    mSettings->language = (mSettings->language == cls::Language::Chinese)
        ? cls::Language::English
        : cls::Language::Chinese;
    cls::setLanguage(mSettings->language);
    refreshText();
    refreshValues();
    return SettingsAction::Changed;
}

void SettingsPanel::onBgmChanged(float value) {
    if (!mSettings) return;
    mSettings->bgmVolume = static_cast<int>(value);
    mBgmValueLabel->setText(std::to_string(mSettings->bgmVolume) + "%");
    if (mRefreshingValues) return;
    mSelectedRow = kBgmRow;
    updateSelectionVisuals();
    if (mOnAction) mOnAction(SettingsAction::Changed);
}

void SettingsPanel::onSfxChanged(float value) {
    if (!mSettings) return;
    mSettings->sfxVolume = static_cast<int>(value);
    mSfxValueLabel->setText(std::to_string(mSettings->sfxVolume) + "%");
    if (mRefreshingValues) return;
    mSelectedRow = kSfxRow;
    updateSelectionVisuals();
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
    if (visible) {
        refreshText();
        refreshValues();
        updateSelectionVisuals();
    }
    if (mContainer) mContainer->setVisible(visible);
}

void SettingsPanel::setOnAction(std::function<void(SettingsAction)> callback) {
    mOnAction = std::move(callback);
}

void SettingsPanel::moveSelection(int delta) {
    mSelectedRow += delta;
    while (mSelectedRow < 0) mSelectedRow += kRowCount;
    mSelectedRow %= kRowCount;
    updateSelectionVisuals();
}

SettingsAction SettingsPanel::adjustCurrent(int delta) {
    if (!mSettings) return SettingsAction::None;

    if (mSelectedRow == kBgmRow) {
        const int next = std::clamp(mSettings->bgmVolume + delta * 5, 0, 100);
        if (next == mSettings->bgmVolume) return SettingsAction::None;
        mSettings->bgmVolume = next;
        refreshValues();
        return SettingsAction::Changed;
    }

    if (mSelectedRow == kSfxRow) {
        const int next = std::clamp(mSettings->sfxVolume + delta * 5, 0, 100);
        if (next == mSettings->sfxVolume) return SettingsAction::None;
        mSettings->sfxVolume = next;
        refreshValues();
        return SettingsAction::Changed;
    }

    if (mSelectedRow == kWindowRow) return cycleWindowSize(delta);
    if (mSelectedRow == kLanguageRow) return toggleLanguage();
    return SettingsAction::None;
}

SettingsAction SettingsPanel::confirmCurrent() {
    if (mSelectedRow == kWindowRow) return cycleWindowSize(1);
    if (mSelectedRow == kLanguageRow) return toggleLanguage();
    if (mSelectedRow == kBackRow) return SettingsAction::Close;
    return SettingsAction::None;
}
