#ifndef CLS_UI_QUESTMANAGERDEMOPAGE_H
#define CLS_UI_QUESTMANAGERDEMOPAGE_H

#include <SFML/Graphics.hpp>
#include "ui/UIComponent.h"

class QuestManager;

/**
 * @class QuestManagerDemoPage
 * @brief QuestManager 演示页面 — 任务链列表、进度信息、JSON 工厂状态
 */
class QuestManagerDemoPage : public UIComponent {
public:
    explicit QuestManagerDemoPage(sf::Font& font);

    void setQuestManager(const QuestManager* qm);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    sf::Font& mFont;
    const QuestManager* mQuestManager = nullptr;
};

#endif
