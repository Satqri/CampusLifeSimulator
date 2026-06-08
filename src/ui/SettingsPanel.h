#ifndef CLS_UI_SETTINGS_PANEL_H
#define CLS_UI_SETTINGS_PANEL_H

#include "core/GameSettings.h"
#include "ui/UIComponent.h"

#include <SFML/Graphics.hpp>
#include <string>

/**
 * @enum SettingsAction
 * @brief 设置面板输出动作
 */
enum class SettingsAction {
    None,
    Changed,
    Close
};

/**
 * @class SettingsPanel
 * @brief 游戏设置界面，支持键盘与鼠标调节。
 */
class SettingsPanel : public UIComponent {
public:
    explicit SettingsPanel(sf::Font& font);

    void setSettings(const cls::GameSettings* settings);
    void setOverlayMode(bool overlayMode);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void moveSelection(int delta);
    SettingsAction adjustCurrent(cls::GameSettings& settings, int delta);
    SettingsAction handleClick(sf::Vector2f mousePosition, cls::GameSettings& settings);
    SettingsAction confirmCurrent(cls::GameSettings& settings);
    bool isEditing() const;
    void setEditing(bool value);

private:
    enum class Row {
        Bgm,
        Sfx,
        WindowSize,
        Language,
        Back
    };

    sf::Font& font;
    const cls::GameSettings* settings = nullptr;
    sf::RectangleShape background;
    sf::RectangleShape panel;
    int selectedIndex = 0;
    bool overlayMode = false;
    bool editing = false;

    std::string valueText(Row row) const;
    sf::FloatRect rowBounds(int index) const;
    bool contains(const sf::FloatRect& bounds, sf::Vector2f point) const;
};

#endif // CLS_UI_SETTINGS_PANEL_H
