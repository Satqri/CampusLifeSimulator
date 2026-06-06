#include "ui/QuestManagerDemoPage.h"
#include "quest/QuestManager.h"
#include "quest/MainQuest.h"
#include "core/AssetPath.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>

QuestManagerDemoPage::QuestManagerDemoPage(sf::Font& font) : mFont(font) {}

void QuestManagerDemoPage::setQuestManager(const QuestManager* qm) { mQuestManager = qm; }
void QuestManagerDemoPage::update(float) {}

void QuestManagerDemoPage::render(sf::RenderWindow& window) {
    if (!mQuestManager) return;

    sf::RectangleShape bg({960.0f, 540.0f});
    bg.setFillColor(sf::Color(15, 15, 25, 220));
    window.draw(bg);

    sf::Text title(mFont, "QuestManager -- JSON Factory + Quest Chain", 26);
    title.setFillColor(sf::Color::White);
    title.setPosition({40.0f, 30.0f});
    window.draw(title);

    // 进度信息
    std::ostringstream info;
    info << "Events Completed: " << mQuestManager->getCompletedEventCount()
         << "  |  Current Quest: " << mQuestManager->getCurrentQuestIndex()
         << "/" << mQuestManager->getTotalQuests()
         << "  |  Semester Progress: " << std::fixed << std::setprecision(1)
         << (mQuestManager->getSemesterProgress() * 100.0f) << "%"
         << "  |  Next Threshold: " << (mQuestManager->getNextThreshold() >= 0
                ? std::to_string(mQuestManager->getNextThreshold()) : "None");
    sf::Text infoText(mFont, info.str(), 14);
    infoText.setFillColor(sf::Color(180, 200, 230));
    infoText.setPosition({40.0f, 75.0f});
    window.draw(infoText);

    // 当前活跃任务
    auto* curr = mQuestManager->getCurrentQuest();
    if (curr) {
        sf::RectangleShape activeBg({880.0f, 100.0f});
        activeBg.setFillColor(sf::Color(30, 60, 30, 200));
        activeBg.setPosition({40.0f, 110.0f});
        window.draw(activeBg);

        sf::Text activeLabel(mFont, "> Active Quest", 16);
        activeLabel.setFillColor(sf::Color(100, 255, 100));
        activeLabel.setPosition({50.0f, 115.0f});
        window.draw(activeLabel);

        sf::Text activeName(mFont, curr->getQuestName(), 20);
        activeName.setFillColor(sf::Color::White);
        activeName.setPosition({50.0f, 140.0f});
        window.draw(activeName);

        sf::Text activeDesc(mFont, curr->getDescription(), 14);
        activeDesc.setFillColor(sf::Color(200, 200, 200));
        activeDesc.setPosition({50.0f, 170.0f});
        window.draw(activeDesc);
    }

    // 任务链列表
    sf::Text chainTitle(mFont, "Quest Chain (triggered by threshold):", 16);
    chainTitle.setFillColor(sf::Color(200, 200, 200));
    chainTitle.setPosition({40.0f, 230.0f});
    window.draw(chainTitle);

    // 手动读取 JSON 展示原始数据
    using json = nlohmann::json;
    std::ifstream f(cls::resolveAssetPath("assets/config/quests.json"));
    if (f.is_open()) {
        json data = json::parse(f);
        int idx = 0;
        for (const auto& q : data["quests"]) {
            float y = 260.0f + idx * 30.0f;
            std::string typeStr = q.value("type", "?");

            sf::Color rowColor = (idx == mQuestManager->getCurrentQuestIndex())
                ? sf::Color(255, 200, 100)
                : sf::Color(150, 150, 150);

            std::ostringstream line;
            line << "#" << idx << "  [" << q.value("threshold", 0) << " events trigger]  "
                 << q.value("name", "???") << "  ("
                 << (typeStr == "midterm_exam" || typeStr == "final_exam"
                    ? "ExamQuest subclass" : "SimpleQuest")
                 << ")";
            sf::Text row(mFont, line.str(), 13);
            row.setFillColor(rowColor);
            row.setPosition({60.0f, y});
            window.draw(row);
            idx++;
        }
    }

    // 操作提示
    sf::Text hint(mFont,
        "[Enter] Create next quest (factory method)  |  [S] Simulate trigger check\n"
        "[E] Simulate completing a random event (+1 count)  |  [C] Reset demo",
        13);
    hint.setFillColor(sf::Color(130, 130, 150));
    hint.setPosition({40.0f, 490.0f});
    window.draw(hint);
}
