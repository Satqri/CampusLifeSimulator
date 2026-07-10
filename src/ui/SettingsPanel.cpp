#include "ui/SettingsPanel.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"
#include "utils/AssetPath.h"

#include <algorithm>
#include <array>
#include <filesystem>

SettingsPanel::SettingsPanel(sf::Font& fontRef)
    : font(fontRef)
    , background({960.0f, 540.0f})
    , panel({760.0f, 390.0f})
{
    panel.setPosition({100.0f, 78.0f});
    panel.setFillColor(sf::Color(20, 26, 36, 238));
    panel.setOutlineColor(sf::Color(114, 138, 168));
    panel.setOutlineThickness(2.0f);

    const std::string resolved = cls::resolveAssetPath("assets/image/ui/settings/settings_popup.png");
    if (std::filesystem::exists(resolved) && mPanelTexture.loadFromFile(resolved)) {
        mPanelSprite = std::make_unique<sf::Sprite>(mPanelTexture);
    }
}

void SettingsPanel::setSettings(const cls::GameSettings* value) {
    settings = value;
}

void SettingsPanel::setOverlayMode(bool value) {
    overlayMode = value;
}

void SettingsPanel::update(float deltaTime) {
    (void)deltaTime;
}

void SettingsPanel::render(sf::RenderWindow& window) {
    if (!settings) return;

    background.setFillColor(overlayMode ? sf::Color(6, 8, 14, 190) : sf::Color(14, 18, 28));
    window.draw(background);

    if (mPanelSprite) {
        const auto size = mPanelTexture.getSize();
        const float scale = 390.0f / static_cast<float>(size.y);
        mPanelSprite->setScale({scale, scale});
        const float scaledW = size.x * scale;
        mPanelSprite->setPosition({100.0f + (760.0f - scaledW) / 2.0f, 78.0f});
        window.draw(*mPanelSprite);
    } else {
        window.draw(panel);
    }

    sf::Text title = cls::makeText(font, cls::text("settings.title"), 30);
    title.setFillColor(sf::Color::White);
    title.setPosition({132.0f, 96.0f});
    window.draw(title);

    const std::array<std::pair<Row, std::string>, 5> rows = {{
        {Row::Bgm, "settings.bgm"},
        {Row::Sfx, "settings.sfx"},
        {Row::WindowSize, "settings.window"},
        {Row::Language, "settings.language"},
        {Row::Back, "settings.back"}
    }};

    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
        const sf::FloatRect bounds = rowBounds(i);
        sf::RectangleShape box(bounds.size);
        box.setPosition(bounds.position);
        const bool selected = i == selectedIndex;
        const bool activeEdit = selected && editing && i != 4;
        box.setFillColor(activeEdit ? sf::Color(58, 92, 132, 245)
                                    : selected ? sf::Color(41, 75, 108, 235)
                                               : sf::Color(29, 36, 50, 215));
        box.setOutlineColor(activeEdit ? sf::Color(255, 245, 180)
                                       : selected ? sf::Color(245, 225, 144)
                                                  : sf::Color(101, 116, 138));
        box.setOutlineThickness(activeEdit ? 4.0f : selected ? 3.0f : 1.5f);
        window.draw(box);

        sf::Text label = cls::makeText(font, cls::text(rows[i].second), 22);
        label.setFillColor(sf::Color(238, 241, 248));
        label.setPosition({bounds.position.x + 24.0f, bounds.position.y + 14.0f});
        window.draw(label);

        sf::Text value = cls::makeText(font, valueText(rows[i].first), 20);
        value.setFillColor(activeEdit ? sf::Color(255, 252, 210)
                                      : selected ? sf::Color(255, 244, 185)
                                                 : sf::Color(208, 223, 240));
        const auto vb = value.getLocalBounds();
        value.setOrigin({vb.position.x + vb.size.x, vb.position.y});
        value.setPosition({bounds.position.x + bounds.size.x - 26.0f, bounds.position.y + 16.0f});
        window.draw(value);
    }

    sf::Text hint = cls::makeText(font, cls::text("settings.nav"), 15);
    hint.setFillColor(sf::Color(220, 228, 242));
    hint.setPosition({126.0f, 430.0f});
    window.draw(hint);
}

void SettingsPanel::moveSelection(int delta) {
    if (editing) return;
    constexpr int kRowCount = 5;
    selectedIndex += delta;
    while (selectedIndex < 0) selectedIndex += kRowCount;
    selectedIndex %= kRowCount;
}

SettingsAction SettingsPanel::adjustCurrent(cls::GameSettings& value, int delta) {
    if (!editing) return SettingsAction::None;

    switch (static_cast<Row>(selectedIndex)) {
        case Row::Bgm:
            value.bgmVolume = std::clamp(value.bgmVolume + delta * 5, 0, 100);
            return SettingsAction::Changed;
        case Row::Sfx:
            value.sfxVolume = std::clamp(value.sfxVolume + delta * 5, 0, 100);
            return SettingsAction::Changed;
        case Row::WindowSize:
            value.windowScaleIndex = std::clamp(value.windowScaleIndex + delta, 0,
                static_cast<int>(cls::windowScalePresets().size()) - 1);
            return SettingsAction::Changed;
        case Row::Language:
            if (delta > 0) {
                value.language = cls::Language::Chinese;
            } else if (delta < 0) {
                value.language = cls::Language::English;
            }
            return SettingsAction::Changed;
        case Row::Back:
            return SettingsAction::Close;
    }
    return SettingsAction::None;
}

SettingsAction SettingsPanel::handleClick(sf::Vector2f mousePosition, cls::GameSettings& value) {
    for (int i = 0; i < 5; ++i) {
        if (contains(rowBounds(i), mousePosition)) {
            selectedIndex = i;
            if (i == 4) {
                editing = false;
                return SettingsAction::Close;
            }

            if (!editing) {
                editing = true;
                return SettingsAction::None;
            }

            if (mousePosition.x > rowBounds(i).position.x + rowBounds(i).size.x * 0.62f) {
                return adjustCurrent(value, 1);
            }
            if (mousePosition.x < rowBounds(i).position.x + rowBounds(i).size.x * 0.38f) {
                return adjustCurrent(value, -1);
            }
            return SettingsAction::None;
        }
    }
    return SettingsAction::None;
}

SettingsAction SettingsPanel::confirmCurrent(cls::GameSettings& value) {
    (void)value;
    if (static_cast<Row>(selectedIndex) == Row::Back) {
        editing = false;
        return SettingsAction::Close;
    }
    if (!editing) {
        editing = true;
        return SettingsAction::None;
    }

    editing = false;
    return SettingsAction::None;
}

bool SettingsPanel::isEditing() const {
    return editing;
}

void SettingsPanel::setEditing(bool value) {
    editing = value;
}

std::string SettingsPanel::valueText(Row row) const {
    if (!settings) return "";
    switch (row) {
        case Row::Bgm:
            return std::to_string(settings->bgmVolume) + "%";
        case Row::Sfx:
            return std::to_string(settings->sfxVolume) + "%";
        case Row::WindowSize:
            return cls::text(cls::windowScalePresets()[settings->windowScaleIndex].labelKey);
        case Row::Language:
            return cls::languageName(settings->language);
        case Row::Back:
            return overlayMode ? "Esc / S" : "Enter";
    }
    return "";
}

sf::FloatRect SettingsPanel::rowBounds(int index) const {
    return sf::FloatRect({136.0f, 154.0f + index * 54.0f}, {688.0f, 44.0f});
}

bool SettingsPanel::contains(const sf::FloatRect& bounds, sf::Vector2f point) const {
    return point.x >= bounds.position.x
        && point.x <= bounds.position.x + bounds.size.x
        && point.y >= bounds.position.y
        && point.y <= bounds.position.y + bounds.size.y;
}
