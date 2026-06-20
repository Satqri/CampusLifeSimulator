#include "quest/SimpleQuest.h"
#include "entity/Player.h"
#include <SFML/Graphics.hpp>

SimpleQuest::SimpleQuest(const std::string& id, const std::string& name,
                         const std::string& desc,
                         const std::vector<std::pair<std::string, Attributes>>& choices,
                         const Attributes& reward)
    : MainQuest(id, name, desc, reward, MainQuestType::ORIENTATION)
    , hoveredChoiceIndex(0)
{
    for (const auto& c : choices) {
        choiceTexts.push_back(c.first);
        choiceOutcomes.push_back(c.second);
        outcomeTexts.push_back("");
    }
    currentPhase = QuestPhase::ANNOUNCEMENT;
}

void SimpleQuest::execute(Player& player) {
    if (selectedChoiceIndex >= 0 && selectedChoiceIndex < static_cast<int>(choiceOutcomes.size())) {
        player.modifyAttributes(choiceOutcomes[selectedChoiceIndex]);
    }
    player.modifyAttributes(completionReward);
    syncVisibleHealthFromHidden(player.getAttributes(), player.getHidden());
    completed = true;
}

void SimpleQuest::render(sf::RenderWindow& window) {
    (void)window;
    // 渲染由 MainQuestState 统一管理
}

bool SimpleQuest::handleInput(const sf::Event& event, Player& player, int& choiceMade) {
    (void)player;

    const auto* key = event.getIf<sf::Event::KeyPressed>();
    if (!key) return false;

    using sf::Keyboard::Key;
    auto code = key->code;

    switch (currentPhase) {
        case QuestPhase::ANNOUNCEMENT:
            if (code == Key::Enter) {
                advancePhase(); // → CHOICE
                return true;
            }
            break;

        case QuestPhase::CHOICE:
            if (code == Key::Up) {
                if (hoveredChoiceIndex > 0) hoveredChoiceIndex--;
                else hoveredChoiceIndex = static_cast<int>(choiceTexts.size()) - 1;
                return true;
            }
            if (code == Key::Down) {
                if (hoveredChoiceIndex < static_cast<int>(choiceTexts.size()) - 1)
                    hoveredChoiceIndex++;
                else hoveredChoiceIndex = 0;
                return true;
            }
            if (code == Key::Enter) {
                selectedChoiceIndex = hoveredChoiceIndex;
                choiceMade = selectedChoiceIndex;
                advancePhase(); // → FINAL_RESULT
                return true;
            }
            break;

        case QuestPhase::FINAL_RESULT:
            if (code == Key::Enter) {
                advancePhase(); // → COMPLETED
                return true;
            }
            break;

        default:
            break;
    }
    return false;
}
