#ifndef CLS_UI_DIFFICULTY_PANEL_H
#define CLS_UI_DIFFICULTY_PANEL_H

#include "ui/UIComponent.h"
#include "ui/TguiTheme.h"
#include "ui/TguiContext.h"

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>

#include <array>
#include <functional>
#include <string>

enum class Difficulty { Easy, Normal, Hard };
enum class DifficultyActionType { None, Back, Select };

struct DifficultyAction {
    DifficultyActionType type = DifficultyActionType::None;
    Difficulty difficulty = Difficulty::Normal;
};

class DifficultyPanel : public UIComponent {
public:
    explicit DifficultyPanel(sf::Font& font);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    void attachToGui(TguiContext& ctx);
    void setVisible(bool visible);
    void setOnAction(std::function<void(DifficultyAction)> callback);

    void moveSelection(int delta);
    DifficultyAction confirmSelection() const;

private:
    struct Card {
        sf::FloatRect bounds;
        std::string title;
        std::string description;
        Difficulty difficulty;
        sf::Color accent;
    };

    void createWidgets();
    void updateCardVisuals();
    void drawDecorative(sf::RenderWindow& window) const;

    sf::Font& mFont;
    TguiContext* mTguiCtx = nullptr;
    tgui::Panel::Ptr mContainer;
    tgui::Button::Ptr mBackButton;
    std::array<tgui::Button::Ptr, 3> mCardButtons;
    tgui::Label::Ptr mTitleLabel;
    tgui::Label::Ptr mSubtitleLabel;
    tgui::Label::Ptr mNoteLabel;
    std::array<tgui::Label::Ptr, 3> mCardTitleLabels;
    std::array<tgui::Label::Ptr, 3> mCardDescLabels;

    std::array<Card, 3> mCards;
    int mSelectedIndex = 1;
    bool mVisible = false;
    bool mWidgetsCreated = false;
    std::function<void(DifficultyAction)> mOnAction;
};

#endif
