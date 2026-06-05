#ifndef CLS_UI_QUESTPANEL_H
#define CLS_UI_QUESTPANEL_H

#include "ui/UIComponent.h"

#include <SFML/Graphics.hpp>
#include <string>

class MainQuest;
struct Attributes;

/**
 * @class QuestPanel
 * @brief Reusable panel that renders main quest descriptions, choices, and exam results.
 */
class QuestPanel : public UIComponent {
public:
    explicit QuestPanel(sf::Font& font);

    void setQuest(MainQuest* quest);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    std::string buildExamStatus() const;
    std::string buildChoiceLine(int index) const;
    std::string buildPrompt() const;
    std::string formatDelta(const Attributes& delta) const;

    sf::Font& font;
    MainQuest* quest;
    sf::RectangleShape overlay;
    sf::RectangleShape panel;
};

#endif // CLS_UI_QUESTPANEL_H
