/**
 * @file main.cpp
 * @brief 类演示程序 — 展示 Entity/Quest/State/Manager 各继承体系的所有类
 *
 * 操作说明:
 *   WASD/方向键 — 移动玩家
 *   1 — Entity 演示（探索 + SAN 阈值触发战斗）
 *   2 — SimpleQuest 演示（新生报到 3 段式任务）
 *   3 — MidtermExamQuest 演示（期中考试 d20 检定）
 *   4 — FinalExamQuest 演示（期末考试 d20 检定）
 *   5 — QuestManager 演示（JSON 加载 + 工厂创建 + 任务链）
 *   0/6 — Help 帮助/设置页面
 *   C — 降低 SAN / 清除 buff
 *   F — 与附近敌人战斗
 *   Enter — 场景切换 / 任务模式下操作
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "core/AssetPath.h"
#include "core/TimeSystem.h"
#include "core/Types.h"
#include "entity/Player.h"
#include "entity/Enemy.h"
#include "quest/MainQuest.h"
#include "quest/SimpleQuest.h"
#include "quest/ExamQuest.h"
#include "quest/MidtermExamQuest.h"
#include "quest/FinalExamQuest.h"
#include "quest/QuestManager.h"
#include "map/MapPortal.h"
#include "map/BuildingInterior.h"
#include "map/CampusMap.h"
#include "map/DormitoryInterior.h"
#include "map/GymInterior.h"
#include "map/LibraryInterior.h"
#include "map/ClassroomInterior.h"
#include "map/CafeteriaInterior.h"
#include "ui/HUD.h"
#include "ui/HelpPanel.h"
#include "ui/QuestPanel.h"
#include "ui/TitleScreen.h"
#include "ui/DifficultyPanel.h"
#include "ui/SceneBackground.h"

#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

// ──────────────────────────────────────────────────────────────
// 最小 Game 类 — MainQuestState 的依赖（当前未使用 Game* 成员）
// ──────────────────────────────────────────────────────────────
class Game {
public:
    sf::RenderWindow* window = nullptr;
};

// ──────────────────────────────────────────────────────────────
// 演示模式
// ──────────────────────────────────────────────────────────────
enum class DemoPage {
    ENTITY,          // Entity 继承体系演示
    SIMPLE_QUEST,    // SimpleQuest 演示
    MIDTERM_EXAM,    // MidtermExamQuest 演示
    FINAL_EXAM,      // FinalExamQuest 演示
    QUEST_MANAGER,   // QuestManager demo
    HELP             // Help and settings page
};

enum class GameScreen {
    TITLE,
    DIFFICULTY,
    GAME
};

// ──────────────────────────────────────────────────────────────
// 战斗结果
// ──────────────────────────────────────────────────────────────
struct CombatResult {
    bool active = false;
    bool victory = false;
    float timer = 0.0f;
    std::string enemyName;
    int d20Roll = 0;
    int modifier = 0;
    int total = 0;
    int dc = 0;

    void show(bool win, const std::string& name, int d20, int mod, int tot, int d) {
        active = true; victory = win; timer = 3.0f;
        enemyName = name; d20Roll = d20; modifier = mod; total = tot; dc = d;
    }
    void update(float dt) {
        if (active) { timer -= dt; if (timer <= 0.0f) active = false; }
    }
    void clear() { active = false; timer = 0.0f; }
};

struct SceneTransition {
    bool active = false;
    float timer = 0.0f;
    SceneBackgroundType background = SceneBackgroundType::Dormitory;
    std::string title;
    std::string subtitle;

    void start(SceneBackgroundType bg, const std::string& heading, const std::string& line) {
        active = true;
        timer = 0.0f;
        background = bg;
        title = heading;
        subtitle = line;
    }

    void update(float dt) {
        if (!active) return;
        timer += dt;
    }

    bool canContinue() const {
        return timer >= 0.45f;
    }

    void skip() {
        active = false;
        timer = 0.0f;
    }
};

struct ActivityNotice {
    bool active = false;
    std::string title;
    std::string body;

    void show(const std::string& heading, const std::string& message) {
        active = true;
        title = heading;
        body = message;
    }

    void clear() { active = false; title.clear(); body.clear(); }
};

struct ChoicePrompt {
    bool active = false;
    std::string title;
    std::string body;
    std::string first;
    std::string second;

    void show(const std::string& heading, const std::string& message,
              const std::string& optionA, const std::string& optionB) {
        active = true;
        title = heading;
        body = message;
        first = optionA;
        second = optionB;
    }

    void clear() { active = false; title.clear(); body.clear(); first.clear(); second.clear(); }
};

struct TimeSkipFlash {
    bool active = false;
    float timer = 0.0f;
    std::string text;

    void start(const std::string& message) {
        active = true;
        timer = 0.58f;
        text = message;
    }

    void update(float dt) {
        if (!active) return;
        timer -= dt;
        if (timer <= 0.0f) {
            active = false;
            timer = 0.0f;
        }
    }
};

// ──────────────────────────────────────────────────────────────
// 根据情绪类型获取对应玩家属性值（用于战斗检定）
// ──────────────────────────────────────────────────────────────
static int statForEmotion(const Player& player, EmotionType type) {
    const auto& a = player.getAttributes();
    switch (type) {
        case EmotionType::ANXIETY:    return a.academic;
        case EmotionType::ANGER:      return a.energy;
        case EmotionType::LONELINESS: return a.social;
        case EmotionType::DEPRESSION: // fallthrough
        case EmotionType::FEAR:       return a.san;
    }
    return a.san;
}

static const char* actionNameForEmotion(EmotionType type) {
    switch (type) {
        case EmotionType::ANXIETY:    return "Rational Analysis";
        case EmotionType::DEPRESSION: return "Will Stand";
        case EmotionType::ANGER:      return "Vent / Release";
        case EmotionType::FEAR:       return "Will Stand";
        case EmotionType::LONELINESS: return "Confide / Seek Help";
    }
    return "Unknown";
}

// ──────────────────────────────────────────────────────────────
// 渲染当前属性面板（所有模式下都在顶部显示）
// ──────────────────────────────────────────────────────────────
void renderStatsPanel(sf::RenderWindow& window, sf::Font& font,
                      const Player& player, DemoPage page) {
    const char* pageNames[] = {
        "Entity", "SimpleQuest", "MidtermExam",
        "FinalExam", "QuestManager", "Help / Settings"
    };

    HUD hud(font);
    hud.setPlayer(&player);
    hud.setPageName(pageNames[static_cast<int>(page)]);
    hud.render(window);
}

void renderTimePanel(sf::RenderWindow& window, sf::Font& font, const TimeSystem& time) {
    sf::RectangleShape panel({182.0f, 34.0f});
    panel.setPosition({655.0f, 4.0f});
    panel.setFillColor(sf::Color(10, 18, 26, 230));
    panel.setOutlineColor(time.isMidtermDay() ? sf::Color(255, 190, 90) : sf::Color(80, 96, 118));
    panel.setOutlineThickness(1.0f);
    window.draw(panel);

    sf::Text clock(font, time.clockText(), 12);
    clock.setFillColor(sf::Color(235, 238, 220));
    clock.setPosition({665.0f, 8.0f});
    window.draw(clock);

    sf::Text label(font, time.dayLabel(), 10);
    label.setFillColor(time.isMidtermDay() ? sf::Color(255, 210, 120) : sf::Color(155, 180, 205));
    label.setPosition({665.0f, 23.0f});
    window.draw(label);
}

void renderModalBox(sf::RenderWindow& window, sf::Font& font,
                    const std::string& title, const std::string& body,
                    const std::string& footer) {
    sf::RectangleShape shade({kRenderWidth, kRenderHeight});
    shade.setFillColor(sf::Color(0, 0, 0, 105));
    window.draw(shade);

    sf::RectangleShape box({620.0f, 178.0f});
    box.setPosition({190.0f, 174.0f});
    box.setFillColor(sf::Color(14, 24, 31, 235));
    box.setOutlineColor(sf::Color(230, 210, 148, 180));
    box.setOutlineThickness(2.0f);
    window.draw(box);

    sf::Text heading(font, title, 22);
    heading.setFillColor(sf::Color(250, 238, 200));
    heading.setPosition({218.0f, 196.0f});
    window.draw(heading);

    sf::Text message(font, body, 15);
    message.setFillColor(sf::Color(218, 230, 220));
    message.setPosition({218.0f, 238.0f});
    window.draw(message);

    sf::Text hint(font, footer, 12);
    hint.setFillColor(sf::Color(172, 184, 178));
    hint.setPosition({218.0f, 320.0f});
    window.draw(hint);
}

void renderNotice(sf::RenderWindow& window, sf::Font& font, const ActivityNotice& notice) {
    if (!notice.active) return;
    renderModalBox(window, font, notice.title, notice.body, "Press Enter to continue");
}

void renderChoicePrompt(sf::RenderWindow& window, sf::Font& font, const ChoicePrompt& prompt) {
    if (!prompt.active) return;
    std::ostringstream body;
    body << prompt.body << "\n\n"
         << "[1] " << prompt.first << "\n"
         << "[2] " << prompt.second;
    renderModalBox(window, font, prompt.title, body.str(), "Press 1 or 2");
}

void renderTimeSkipFlash(sf::RenderWindow& window, sf::Font& font, const TimeSkipFlash& flash) {
    if (!flash.active) return;
    sf::RectangleShape blackout({kRenderWidth, kRenderHeight});
    blackout.setFillColor(sf::Color(0, 0, 0, 245));
    window.draw(blackout);

    sf::Text text(font, flash.text, 20);
    text.setFillColor(sf::Color(230, 230, 220));
    text.setPosition({360.0f, 252.0f});
    window.draw(text);
}

void applyDifficulty(Player& player, Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:
            player.modifyAttributes(Attributes(20, 15, 5, 5, 0));
            break;
        case Difficulty::Normal:
            break;
        case Difficulty::Hard:
            player.modifyAttributes(Attributes(-20, -10, 0, -5, 0));
            break;
    }
}

void renderSceneTransition(sf::RenderWindow& window, sf::Font& font,
                           SceneBackground& background, const SceneTransition& transition) {
    background.render(window, transition.background, sf::Color(0, 0, 0, 70));

    sf::RectangleShape plate({640.0f, 132.0f});
    plate.setPosition({160.0f, 332.0f});
    plate.setFillColor(sf::Color(9, 22, 28, 176));
    plate.setOutlineColor(sf::Color(230, 212, 148, 170));
    plate.setOutlineThickness(2.0f);
    window.draw(plate);

    sf::Text title(font, transition.title, 34);
    title.setFillColor(sf::Color(250, 240, 205));
    title.setOutlineColor(sf::Color(18, 42, 45));
    title.setOutlineThickness(2.0f);
    title.setPosition({190.0f, 352.0f});
    window.draw(title);

    sf::Text subtitle(font, transition.subtitle, 18);
    subtitle.setFillColor(sf::Color(224, 238, 220));
    subtitle.setPosition({190.0f, 407.0f});
    window.draw(subtitle);

    const bool ready = transition.canContinue();
    sf::Text hint(font, ready ? "Press Enter to enter" : "Entering...", 13);
    hint.setFillColor(ready ? sf::Color(210, 210, 190, 180) : sf::Color(210, 210, 190, 110));
    hint.setPosition({190.0f, 444.0f});
    window.draw(hint);
}

SceneBackgroundType backgroundForPage(DemoPage page) {
    switch (page) {
        case DemoPage::ENTITY: return SceneBackgroundType::Dormitory;
        case DemoPage::SIMPLE_QUEST: return SceneBackgroundType::Cafeteria;
        case DemoPage::MIDTERM_EXAM:
        case DemoPage::FINAL_EXAM: return SceneBackgroundType::Classroom;
        case DemoPage::QUEST_MANAGER: return SceneBackgroundType::Library;
        case DemoPage::HELP: return SceneBackgroundType::Library;
    }
    return SceneBackgroundType::Dormitory;
}

std::pair<std::string, std::string> transitionTextForPage(DemoPage page) {
    switch (page) {
        case DemoPage::ENTITY:
            return {"Dormitory", "A quiet room gives you a moment to gather yourself before the day begins."};
        case DemoPage::SIMPLE_QUEST:
            return {"Cafeteria Break", "Warm lights, crowded tables, and one small choice after another."};
        case DemoPage::MIDTERM_EXAM:
            return {"Classroom Bell", "The midterm is close; every page reviewed now may matter later."};
        case DemoPage::FINAL_EXAM:
            return {"Final Classroom", "The semester narrows to one room, one paper, and one last deep breath."};
        case DemoPage::QUEST_MANAGER:
            return {"Library Route", "Plans, deadlines, and future quests gather between the shelves."};
        case DemoPage::HELP:
            return {"Campus Guide", "Before moving on, check the rules and controls of campus life."};
    }
    return {"Campus", "The next part of the day is about to begin."};
}

// Entity 演示: 探索地图 + SAN 阈值触发敌人出现
// ──────────────────────────────────────────────────────────────
void runEntityDemo(sf::RenderWindow& window, sf::Font& font,
                   Player& player, std::vector<std::unique_ptr<Enemy>>& activeEnemies,
                   const CombatResult& combatResult) {
    // --- 地图背景 ---
    sf::RectangleShape mapBg({960.0f, 540.0f});
    mapBg.setFillColor(sf::Color(30, 40, 30));
    window.draw(mapBg);

    // 地面网格
    for (int x = 0; x < 960; x += 32) {
        for (int y = 0; y < 540; y += 32) {
            sf::RectangleShape tile({31.0f, 31.0f});
            tile.setPosition({static_cast<float>(x), static_cast<float>(y)});
            tile.setFillColor(sf::Color(40, 50, 40));
            window.draw(tile);
        }
    }

    // --- 探索点位标记（事件触发点）---
    const float markers[4][2] = {{400.f, 160.f}, {600.f, 320.f}, {240.f, 360.f}, {720.f, 120.f}};
    for (auto& m : markers) {
        sf::RectangleShape marker({16.0f, 16.0f});
        marker.setPosition({m[0], m[1]});
        marker.setFillColor(sf::Color(80, 80, 120));
        marker.setOutlineColor(sf::Color(120, 120, 180));
        marker.setOutlineThickness(1.0f);
        window.draw(marker);
    }
    sf::Text markerLabel(font, "Event Points (step on to trigger events in full game)", 10);
    markerLabel.setFillColor(sf::Color(100, 100, 140));
    markerLabel.setPosition({8.0f, 70.0f});
    window.draw(markerLabel);

    // --- 渲染活跃敌人（SAN 低时出现）---
    for (auto& e : activeEnemies) {
        e->render(window);
        sf::Text label(font, e->getName(), 11);
        label.setFillColor(sf::Color(255, 180, 80));
        auto pos = e->getPosition();
        label.setPosition({pos.x - 20.0f, pos.y - 18.0f});
        window.draw(label);

        // DC/ATK 信息
        std::ostringstream ss;
        ss << "DC:" << e->getDC() << " ATK:" << e->getAttackPower();
        sf::Text info(font, ss.str(), 9);
        info.setFillColor(sf::Color(200, 160, 100));
        info.setPosition({pos.x - 20.0f, pos.y + 10.0f});
        window.draw(info);
    }

    // --- 渲染玩家 ---
    player.render(window);
    sf::Text pLabel(font, "You", 11);
    pLabel.setFillColor(sf::Color(100, 200, 255));
    auto ppos = player.getPosition();
    pLabel.setPosition({ppos.x - 8.0f, ppos.y + 10.0f});
    window.draw(pLabel);

    // --- 说明文字（右下角）---
    sf::RectangleShape legendBg({420.0f, 88.0f});
    legendBg.setPosition({530.0f, 444.0f});
    legendBg.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(legendBg);

    sf::Text legend(font,
        "How it works (aligned with plan.md):\n"
        "  [C] Stress event -- lowers SAN\n"
        "  SAN < 30/20/10: enemies randomly appear!\n"
        "  [F] Fight nearby enemy (d20 combat)\n"
        "  Enemies are NOT always on map -- they spawn when SAN drops\n"
        "  Event Points (purple) = exploration triggers (future)",
        10);
    legend.setFillColor(sf::Color(180, 180, 190));
    legend.setPosition({536.0f, 448.0f});
    window.draw(legend);

    // --- 战斗结果覆盖 ---
    if (combatResult.active) {
        sf::RectangleShape overlay({400.0f, 130.0f});
        overlay.setPosition({280.0f, 200.0f});
        overlay.setFillColor(sf::Color(20, 20, 40, 230));
        overlay.setOutlineColor(combatResult.victory ? sf::Color(100, 200, 100) : sf::Color(200, 100, 100));
        overlay.setOutlineThickness(2.0f);
        window.draw(overlay);

        std::ostringstream css;
        css << (combatResult.victory ? "VICTORY!" : "DEFEAT!")
            << "  vs " << combatResult.enemyName << "\n\n"
            << "  D20: " << combatResult.d20Roll
            << " + MOD: " << (combatResult.modifier >= 0 ? "+" : "") << combatResult.modifier
            << " = " << combatResult.total
            << " vs DC " << combatResult.dc << "\n\n"
            << (combatResult.victory ? "  SAN restored! Buff gained!" : "  SAN -15! Debuff applied!");
        sf::Text resultText(font, css.str(), 14);
        resultText.setFillColor(combatResult.victory ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
        resultText.setPosition({300.0f, 215.0f});
        window.draw(resultText);
    }
}

// ──────────────────────────────────────────────────────────────
// 通用 Quest UI 渲染（用于 SimpleQuest / ExamQuest 演示）
// ──────────────────────────────────────────────────────────────
void renderQuestUI(sf::RenderWindow& window, sf::Font& font,
                   MainQuest* quest, Player* /*player*/) {
    if (!quest) return;

    QuestPanel questPanel(font);
    questPanel.setQuest(quest);
    questPanel.render(window);
}
void renderQuestManagerDemo(sf::RenderWindow& window, sf::Font& font,
                            QuestManager& qm) {
    sf::RectangleShape bg({960.0f, 540.0f});
    bg.setFillColor(sf::Color(15, 15, 25, 220));
    window.draw(bg);

    sf::Text title(font, "QuestManager -- JSON Factory + Quest Chain", 26);
    title.setFillColor(sf::Color::White);
    title.setPosition({40.0f, 30.0f});
    window.draw(title);

    // 进度信息
    std::ostringstream info;
    info << "Events Completed: " << qm.getCompletedEventCount()
         << "  |  Current Quest: " << qm.getCurrentQuestIndex() << "/" << qm.getTotalQuests()
         << "  |  Semester Progress: " << std::fixed << std::setprecision(1)
         << (qm.getSemesterProgress() * 100.0f) << "%"
         << "  |  Next Threshold: " << (qm.getNextThreshold() >= 0
                ? std::to_string(qm.getNextThreshold()) : "None");
    sf::Text infoText(font, info.str(), 14);
    infoText.setFillColor(sf::Color(180, 200, 230));
    infoText.setPosition({40.0f, 75.0f});
    window.draw(infoText);

    // 当前活跃任务
    auto* curr = qm.getCurrentQuest();
    if (curr) {
        sf::RectangleShape activeBg({880.0f, 100.0f});
        activeBg.setFillColor(sf::Color(30, 60, 30, 200));
        activeBg.setPosition({40.0f, 110.0f});
        window.draw(activeBg);

        sf::Text activeLabel(font, "> Active Quest", 16);
        activeLabel.setFillColor(sf::Color(100, 255, 100));
        activeLabel.setPosition({50.0f, 115.0f});
        window.draw(activeLabel);

        sf::Text activeName(font, curr->getQuestName(), 20);
        activeName.setFillColor(sf::Color::White);
        activeName.setPosition({50.0f, 140.0f});
        window.draw(activeName);

        sf::Text activeDesc(font, curr->getDescription(), 14);
        activeDesc.setFillColor(sf::Color(200, 200, 200));
        activeDesc.setPosition({50.0f, 170.0f});
        window.draw(activeDesc);
    }

    // 任务链列表
    sf::Text chainTitle(font, "Quest Chain (triggered by threshold):", 16);
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

            sf::Color rowColor = (idx == qm.getCurrentQuestIndex())
                ? sf::Color(255, 200, 100)
                : sf::Color(150, 150, 150);

            std::ostringstream line;
            line << "#" << idx << "  [" << q.value("threshold", 0) << " events trigger]  "
                 << q.value("name", "???") << "  ("
                 << (typeStr == "midterm_exam" || typeStr == "final_exam"
                    ? "ExamQuest subclass" : "SimpleQuest")
                 << ")";
            sf::Text row(font, line.str(), 13);
            row.setFillColor(rowColor);
            row.setPosition({60.0f, y});
            window.draw(row);
            idx++;
        }
    }

    // 操作提示
    sf::Text hint(font,
        "[Enter] Create next quest (factory method)  |  [S] Simulate trigger check\n"
        "[E] Simulate completing a random event (+1 count)  |  [C] Reset demo",
        13);
    hint.setFillColor(sf::Color(130, 130, 150));
    hint.setPosition({40.0f, 490.0f});
    window.draw(hint);
}

// ──────────────────────────────────────────────────────────────
// main
// ──────────────────────────────────────────────────────────────
int main() {
    // ── 窗口 ────────────────────────────────────────────────
    sf::RenderWindow window(sf::VideoMode({kWindowWidth, kWindowHeight}), "CampusLifeSimulator - Class Demo");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    // 将 960×540 渲染坐标系映射到 1280×720 窗口
    sf::View gameView(sf::FloatRect({0.0f, 0.0f}, {kRenderWidth, kRenderHeight}));
    window.setView(gameView);

    // ── 字体（编译期检测平台，选择对应系统字体）─────────────
    sf::Font font;
    bool fontOk = false;
#if defined(_WIN32)
    const std::vector<std::string> fontCandidates = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/arial.ttf",
    };
    for (const auto& path : fontCandidates) {
        if (font.openFromFile(path)) { fontOk = true; break; }
    }
#elif defined(__APPLE__)
    fontOk = font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")
          || font.openFromFile("/System/Library/Fonts/PingFang.ttc");
#elif defined(__linux__)
    fontOk = font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
#endif
    if (!fontOk) {
        std::cerr << "ERROR: Failed to load font!" << std::endl;
    }

    // ── 创建 Entity 对象 ─────────────────────────────────────
    TitleScreen titleScreen(font, "assets/ui/campus_title_bg.png");
    DifficultyPanel difficultyPanel(font);
    SceneBackground sceneBackground;
    SceneTransition sceneTransition;
    TimeSystem timeSystem;
    ActivityNotice activityNotice;
    ChoicePrompt classChoicePrompt;
    TimeSkipFlash timeSkipFlash;
    GameScreen screen = GameScreen::TITLE;
    Difficulty selectedDifficulty = Difficulty::Normal;
    bool difficultyApplied = false;
    int selectedLibraryBook = 0;
    std::array<int, 4> libraryBookProgress = {0, 0, 0, 0};
    const std::array<std::string, 4> libraryBookNames = {
        "Reference Methods", "Literature Notes", "Science Primer", "Campus History"
    };
    const std::array<std::string, 4> librarySkillNames = {
        "Research", "Reflection", "Logic", "Context"
    };

    // ── 地图对象 ───────────────────────────────────────────────
    auto campusMap     = std::make_unique<CampusMap>();
    auto dormitoryMap  = std::make_unique<DormitoryInterior>();
    auto gymMap        = std::make_unique<GymInterior>();
    auto libraryMap    = std::make_unique<LibraryInterior>();
    auto classroomMap  = std::make_unique<ClassroomInterior>();
    auto cafeteriaMap  = std::make_unique<CafeteriaInterior>();

    // 设置字体
    campusMap->setFont(&font);
    dormitoryMap->setFont(&font);
    gymMap->setFont(&font);
    libraryMap->setFont(&font);
    classroomMap->setFont(&font);
    cafeteriaMap->setFont(&font);

    BuildingInterior* currentMap = campusMap.get();

    auto setCurrentMap = [&](CampusPlace place) -> BuildingInterior* {
        switch (place) {
            case CampusPlace::Campus:    return campusMap.get();
            case CampusPlace::Dormitory: return dormitoryMap.get();
            case CampusPlace::Gym:       return gymMap.get();
            case CampusPlace::Library:   return libraryMap.get();
            case CampusPlace::Classroom: return classroomMap.get();
            case CampusPlace::Cafeteria: return cafeteriaMap.get();
        }
        return campusMap.get();
    };

    Player player(480.0f, 280.0f);
    player.setName("Protagonist");
    CampusPlace currentPlace = CampusPlace::Campus;
    CampusPlace pendingPlace = CampusPlace::Campus;
    sf::Vector2f pendingSpawnPosition(480.0f, 276.0f);
    bool hasPendingMapTransition = false;

    // 活跃敌人列表（SAN 低时动态生成，不在探索地图上预放置）
    std::vector<std::unique_ptr<Enemy>> activeEnemies;
    CombatResult combatResult;

    // ── Quest 对象 ───────────────────────────────────────────
    QuestManager questManager;
    questManager.loadQuestChain(cls::resolveAssetPath("assets/config/quests.json"));

    // 独立 quest 对象用于 Simple/Midterm/Final 演示页面
    MainQuest* currentQuest = nullptr;
    std::unique_ptr<SimpleQuest> simpleQuest;
    std::unique_ptr<MidtermExamQuest> midtermQuest;
    std::unique_ptr<FinalExamQuest> finalExamQuest;
    std::unique_ptr<MainQuest> questManagerQuest;

    // ── 状态 ─────────────────────────────────────────────────
    DemoPage page = DemoPage::ENTITY;
    sf::Clock clock;
    bool keyWasPressed[static_cast<int>(sf::Keyboard::KeyCount)] = {};

    // Lambda: 检测单次按键（防止连发）
    auto justPressed = [&](sf::Keyboard::Key k) -> bool {
        bool pressed = sf::Keyboard::isKeyPressed(k);
        bool prev = keyWasPressed[static_cast<int>(k)];
        keyWasPressed[static_cast<int>(k)] = pressed;
        return pressed && !prev;
    };

    // ── Entity Demo 状态 ─────────────────────────────────────
    int spawnCounter = 0;   // 每按 C 累计，用于决定生成哪个情绪类型

    // Lambda: 尝试根据 SAN 等级生成敌人
    // 规则: SAN<30 每按一次 C 概率生成（等级越高概率越大），最多 3 个活跃
    auto trySpawnEnemy = [&]() {
        int lvl = player.getSanLevel();
        if (lvl == 0) return;  // SAN >= 30, 无敌人

        // 最大活跃敌人数随 SAN 等级递增
        int maxEnemies = (lvl == 1) ? 1 : (lvl == 2) ? 2 : 3;
        if (static_cast<int>(activeEnemies.size()) >= maxEnemies) return;

        // 概率: lvl=1: 40%, lvl=2: 60%, lvl=3: 90%
        int chance = lvl == 1 ? 40 : (lvl == 2 ? 60 : 90);
        if ((std::rand() % 100) >= chance) return;

        // 随机情绪类型
        EmotionType types[] = {
            EmotionType::ANXIETY, EmotionType::DEPRESSION, EmotionType::ANGER,
            EmotionType::FEAR, EmotionType::LONELINESS
        };
        EmotionType type = types[spawnCounter % 5];
        spawnCounter++;

        // 在玩家附近随机位置生成
        float ox = player.getPosition().x + ((std::rand() % 160) - 80);
        float oy = player.getPosition().y + ((std::rand() % 160) - 80);
        ox = std::clamp(ox, 40.0f, 920.0f);
        oy = std::clamp(oy, 80.0f, 500.0f);

        auto enemy = std::make_unique<Enemy>(ox, oy, type, 12, 5);
        enemy->scaleWithSanLevel(lvl);
        activeEnemies.push_back(std::move(enemy));

        std::cout << "[Combat] " << activeEnemies.back()->getName()
                  << " appeared! SAN=" << player.getAttributes().san
                  << " Level=" << lvl
                  << " DC=" << activeEnemies.back()->getDC()
                  << " ATK=" << activeEnemies.back()->getAttackPower() << std::endl;
    };

    // Lambda: 与附近敌人战斗
    auto fightNearestEnemy = [&]() -> bool {
        if (activeEnemies.empty() || combatResult.active) return false;

        // 找最近的敌人（距离 < 100px 内）
        float minDist = 100.0f;
        int nearestIdx = -1;
        sf::Vector2f pp = player.getPosition();
        for (int i = 0; i < static_cast<int>(activeEnemies.size()); ++i) {
            sf::Vector2f ep = activeEnemies[i]->getPosition();
            float dx = pp.x - ep.x;
            float dy = pp.y - ep.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < minDist) {
                minDist = dist;
                nearestIdx = i;
            }
        }
        if (nearestIdx < 0) return false;

        Enemy& enemy = *activeEnemies[nearestIdx];
        EmotionType etype = enemy.getEmotionType();

        // 计算检定
        int statVal = statForEmotion(player, etype);
        int modifier = (statVal - 50) / 10;
        int buffMod = player.getCombatBuffs().nextRollModifier;
        int d20 = (std::rand() % 20) + 1;
        int total = d20 + modifier + buffMod;
        int dc = enemy.getDC();
        bool win = total >= dc;

        std::cout << "[Combat] " << actionNameForEmotion(etype)
                  << " vs " << enemy.getName()
                  << " | D20:" << d20 << " MOD:" << modifier
                  << " Buff:" << buffMod << " = " << total
                  << " vs DC:" << dc
                  << " -> " << (win ? "WIN" : "LOSE") << std::endl;

        if (win) {
            player.modifyAttributes(Attributes(20, 0, 0, 0, 0));
            player.getCombatBuffs().nextEventPositive = true;
            player.getCombatBuffs().nextRollModifier = 2;
        } else {
            player.modifyAttributes(Attributes(-15, 0, 0, 0, 0));
            player.getCombatBuffs().nextEventPositive = false;
            player.getCombatBuffs().nextRollModifier = -2;
        }

        combatResult.show(win, enemy.getName(), d20, modifier + buffMod, total, dc);

        // 战斗后敌人消失
        activeEnemies.erase(activeEnemies.begin() + nearestIdx);
        return true;
    };

    // Lambda: 重置演示 quest
    auto resetSimpleDemo = [&]() {
        simpleQuest = std::make_unique<SimpleQuest>(
            "orientation_demo", "Freshman Orientation",
            "You've just stepped onto the university campus. Everything is new. Take a deep breath and begin your college adventure!",
            std::vector<std::pair<std::string, Attributes>>{
                {"Explore the campus and get familiar with the surroundings",  Attributes(0, -5, 0, 10, 0)},
                {"Check out what books the library has",    Attributes(0, -3, 8, 3, 0)},
                {"Grab a good meal at the cafeteria to relax",  Attributes(5, 10, 0, 5, -20)},
            },
            Attributes(0, 0, 0, 5, 0)
        );
        currentQuest = simpleQuest.get();
    };

    auto resetMidtermDemo = [&]() {
        midtermQuest = std::make_unique<MidtermExamQuest>();
        midtermQuest->setPhase(QuestPhase::ANNOUNCEMENT);
        currentQuest = midtermQuest.get();
    };

    auto resetFinalDemo = [&]() {
        finalExamQuest = std::make_unique<FinalExamQuest>();
        finalExamQuest->setPhase(QuestPhase::ANNOUNCEMENT);
        currentQuest = finalExamQuest.get();
    };

    auto startMapTransition = [&](const MapPortal& portal) {
        pendingPlace = portal.target;
        pendingSpawnPosition = portal.spawnPosition;
        hasPendingMapTransition = true;
        sceneTransition.start(portal.transitionBackground, portal.title, portal.subtitle);
    };

    auto finishSceneTransition = [&]() {
        if (hasPendingMapTransition) {
            currentPlace = pendingPlace;
            currentMap = setCurrentMap(pendingPlace);
            player.setPosition(pendingSpawnPosition.x, pendingSpawnPosition.y);
            player.stopMovement();
            hasPendingMapTransition = false;
        }
        sceneTransition.skip();
    };

    auto forceMorningClass = [&]() {
        timeSystem.setTimeAbsolute(TimeSystem::kClassMinute);
        timeSystem.markClassPrompted();
        currentPlace = CampusPlace::Classroom;
        currentMap = classroomMap.get();

        std::vector<const InteractionPoint*> desks;
        for (const auto& ip : classroomMap->getInteractionPoints()) {
            if (ip.label == "Sit at Desk") desks.push_back(&ip);
        }
        if (!desks.empty()) {
            const InteractionPoint* desk = desks[std::rand() % desks.size()];
            player.setPosition(desk->area.position.x + desk->area.size.x * 0.5f,
                               desk->area.position.y + desk->area.size.y * 0.5f);
        } else {
            player.setPosition(480.0f, 276.0f);
        }
        player.stopMovement();

        classChoicePrompt.show(
            timeSystem.isMidtermDay() ? "Midterm Morning" : "Morning Class",
            timeSystem.isMidtermDay()
                ? "It is Day 7. The midterm starts from this classroom seat."
                : "The bell rings at 08:50. Choose how to handle this class.",
            timeSystem.isMidtermDay() ? "Take the midterm seriously" : "Attend class carefully",
            "Skip class"
        );
    };

    auto checkClassSchedule = [&](int previousMinute) {
        if (timeSystem.crossedClassTime(previousMinute) || timeSystem.shouldForceClass()) {
            activityNotice.clear();
            forceMorningClass();
        }
    };

    auto showTimedResult = [&](const std::string& title, const std::string& body) {
        std::ostringstream message;
        message << body << "\nCurrent time: " << timeSystem.clockText();
        activityNotice.show(title, message.str());
    };

    auto runTimedActivity = [&](int minutes, const Attributes& delta,
                                const std::string& title, const std::string& body) {
        const int previousMinute = timeSystem.advanceMinutes(minutes);
        player.modifyAttributes(delta);
        timeSkipFlash.start("Time passes...");
        showTimedResult(title, body);
        checkClassSchedule(previousMinute);
    };

    auto resolveClassChoice = [&](bool attend) {
        classChoicePrompt.clear();
        timeSystem.markClassResolved();

        if (attend) {
            timeSystem.setTimeAbsolute(TimeSystem::kClassEndMinute);
            std::ostringstream body;
            if (timeSystem.isMidtermDay()) {
                const int roll = (std::rand() % 20) + 1;
                const int academicBonus = (player.getAttributes().academic - 50) / 10;
                const int total = roll + academicBonus;
                const bool passed = total >= 12;
                player.modifyAttributes(passed
                    ? Attributes(-10, -16, 12, 0, 0)
                    : Attributes(-16, -18, 4, 0, 0));
                body << "Midterm finished. Roll " << roll
                     << " + Academic Bonus " << academicBonus
                     << " = " << total << (passed ? " (pass)." : " (struggle).");
            } else {
                player.modifyAttributes(Attributes(-8, -12, 8, 0, 0));
                body << "You focused through the morning lecture. Academic +8, SAN -8, Energy -12.";
            }
            timeSkipFlash.start("Class time passes...");
            showTimedResult(timeSystem.isMidtermDay() ? "Midterm Complete" : "Class Complete", body.str());
        } else {
            timeSystem.setTimeAbsolute(TimeSystem::kRollCallMinute);
            const bool called = (std::rand() % 100) < (timeSystem.isMidtermDay() ? 80 : 45);
            std::ostringstream body;
            if (called) {
                player.modifyAttributes(timeSystem.isMidtermDay()
                    ? Attributes(-18, -4, -18, -12, 0)
                    : Attributes(-10, -2, -10, -8, 0));
                body << "At 10:20 the teacher calls attendance. You are absent and take a penalty.";
            } else {
                player.modifyAttributes(Attributes(3, -2, -2, 0, 0));
                body << "At 10:20 there is no roll call. You avoid the immediate penalty, but lose study momentum.";
            }
            timeSkipFlash.start("Skipping class...");
            showTimedResult("Roll Call Notice", body.str());
        }
    };

    auto sleepFromDormitory = [&]() {
        if (!timeSystem.canSleep()) {
            activityNotice.show("Too Early",
                "You can choose sleep after 22:30. Until then, the bed is only a short rest spot.");
            return;
        }

        const int sleptMinutes = timeSystem.sleepToNextDay();
        const int sleptHours = sleptMinutes / 60;
        const int sanGain = std::min(45, sleptHours * 5);
        const int energyGain = std::min(70, sleptHours * 8);
        player.modifyAttributes(Attributes(sanGain, energyGain, 0, 0, 0));
        player.setPosition(480.0f, 276.0f);
        player.stopMovement();
        currentPlace = CampusPlace::Dormitory;
        currentMap = dormitoryMap.get();

        std::ostringstream body;
        if (timeSystem.isFinished()) {
            body << "The 14-day project period is complete. Sleep recovered SAN +"
                 << sanGain << " and Energy +" << energyGain << ".";
        } else {
            body << "You slept " << sleptHours << " hours and woke at 08:00. SAN +"
                 << sanGain << ", Energy +" << energyGain << ".";
        }
        timeSkipFlash.start("Sleeping...");
        showTimedResult(timeSystem.isFinished() ? "Fourteen Days Complete" : "New Day", body.str());
    };

    auto handleInteraction = [&](const InteractionPoint& ip) {
        const std::string& id = ip.actionId;

        if (id.rfind("library_shelf_", 0) == 0) {
            selectedLibraryBook = std::clamp(id.back() - '0', 0, 3);
            std::ostringstream body;
            body << ip.label << " selected " << libraryBookNames[selectedLibraryBook]
                 << ". Reading progress: " << libraryBookProgress[selectedLibraryBook]
                 << "%. Browse Shelf does not consume time.";
            activityNotice.show("Shelf Browsed", body.str());
            return;
        }

        if (id == "library_table") {
            const int book = selectedLibraryBook;
            libraryBookProgress[book] = std::min(100, libraryBookProgress[book] + 25);
            Attributes delta(-3, -6, 4, 0, 0);
            if (book == 1) delta = Attributes(2, -5, 0, 3, 0);
            if (book == 2) delta = Attributes(-4, -7, 6, 0, 0);
            if (book == 3) delta = Attributes(-2, -5, 2, 4, 0);

            std::ostringstream body;
            body << "Read " << libraryBookNames[book] << " for 30 minutes. "
                 << librarySkillNames[book] << " progress is now "
                 << libraryBookProgress[book] << "%.";
            runTimedActivity(30, delta, "Reading Complete", body.str());
            return;
        }

        if (id == "cafeteria_counter" || id.rfind("cafeteria_table_", 0) == 0) {
            if (!timeSystem.isMealTime()) {
                activityNotice.show("Meal Time Closed",
                    "Food is available from 12:00-14:00 and 17:00-19:00.");
                return;
            }
            runTimedActivity(20, Attributes(5, 18, 0, 2, -15),
                             "Meal Complete",
                             "Campus meal placeholder: Gold -15, SAN +5, Energy +18, Social +2.");
            return;
        }

        if (id.rfind("gym_treadmill_", 0) == 0) {
            runTimedActivity(40, Attributes(-4, -14, 0, 2, 0),
                             "Training Complete",
                             "Treadmill run: SAN -4, Energy -14, Social +2.");
            return;
        }

        if (id.rfind("gym_barbell_", 0) == 0) {
            runTimedActivity(40, Attributes(-5, -16, 0, 1, 0),
                             "Training Complete",
                             "Barbell training: SAN -5, Energy -16, Social +1.");
            return;
        }

        if (id == "dormitory_bed") {
            sleepFromDormitory();
            return;
        }

        if (id == "dormitory_desk") {
            runTimedActivity(45, Attributes(-6, -10, 7, 0, 0),
                             "Study Complete",
                             "Desk study: Academic +7, SAN -6, Energy -10.");
            return;
        }

        if (id == "dormitory_rug") {
            activityNotice.show("Quiet Moment",
                "You sit down and collect your thoughts. This currently does not consume time.");
            return;
        }

        if (id == "classroom_board") {
            activityNotice.show("Board Reviewed",
                "The board shows today's notes. Review here is informational and does not consume time.");
            return;
        }

        if (id.rfind("classroom_desk_", 0) == 0) {
            activityNotice.show("Desk",
                "Morning class is handled by the 08:50 forced class event.");
            return;
        }

        activityNotice.show(ip.label, ip.description);
    };

    // 初始化
    resetSimpleDemo();

    // ── 主循环 ───────────────────────────────────────────────
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // 更新战斗结果计时器
        combatResult.update(dt);
        sceneBackground.update(dt);
        sceneTransition.update(dt);
        timeSkipFlash.update(dt);

        // ── 事件处理 ────────────────────────────────────────
        while (const auto eventOpt = window.pollEvent()) {
            const auto& event = *eventOpt;

            if (event.is<sf::Event::Closed>()) {
                window.close();
                return 0;
            }
            if (sceneTransition.active) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (sceneTransition.canContinue()
                        && (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Escape)) {
                        finishSceneTransition();
                    }
                } else if (event.is<sf::Event::MouseButtonPressed>()) {
                    if (sceneTransition.canContinue()) {
                        finishSceneTransition();
                    }
                }
                continue;
            }
            if (screen == GameScreen::TITLE) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Enter) {
                        screen = GameScreen::DIFFICULTY;
                    } else if (keyEv->code == sf::Keyboard::Key::H) {
                        page = DemoPage::HELP;
                        currentQuest = nullptr;
                        screen = GameScreen::GAME;
                    }
                } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
                    const auto action = titleScreen.handleClick({
                        static_cast<float>(mouseEv->position.x),
                        static_cast<float>(mouseEv->position.y)
                    });
                    if (action == TitleAction::Start) {
                        screen = GameScreen::DIFFICULTY;
                    } else if (action == TitleAction::Help) {
                        page = DemoPage::HELP;
                        currentQuest = nullptr;
                        screen = GameScreen::GAME;
                    }
                }
                continue;
            }

            if (screen == GameScreen::DIFFICULTY) {
                auto startGameWithDifficulty = [&](Difficulty difficulty) {
                    selectedDifficulty = difficulty;
                    if (!difficultyApplied) {
                        applyDifficulty(player, selectedDifficulty);
                        difficultyApplied = true;
                    }
                    page = DemoPage::ENTITY;
                    currentQuest = nullptr;
                    currentPlace = CampusPlace::Campus;
                    currentMap = campusMap.get();
                    player.setPosition(480.0f, 276.0f);
                    timeSystem = TimeSystem();
                    activityNotice.clear();
                    classChoicePrompt.clear();
                    screen = GameScreen::GAME;
                };

                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Escape) {
                        screen = GameScreen::TITLE;
                    } else if (keyEv->code == sf::Keyboard::Key::Num1) {
                        startGameWithDifficulty(Difficulty::Easy);
                    } else if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Enter) {
                        startGameWithDifficulty(Difficulty::Normal);
                    } else if (keyEv->code == sf::Keyboard::Key::Num3) {
                        startGameWithDifficulty(Difficulty::Hard);
                    }
                } else if (const auto* mouseEv = event.getIf<sf::Event::MouseButtonPressed>()) {
                    const auto action = difficultyPanel.handleClick({
                        static_cast<float>(mouseEv->position.x),
                        static_cast<float>(mouseEv->position.y)
                    });
                    if (action.type == DifficultyActionType::Back) {
                        screen = GameScreen::TITLE;
                    } else if (action.type == DifficultyActionType::Select) {
                        startGameWithDifficulty(action.difficulty);
                    }
                }
                continue;
            }

            if (timeSkipFlash.active) {
                continue;
            }

            if (classChoicePrompt.active) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Num1 || keyEv->code == sf::Keyboard::Key::Numpad1
                        || keyEv->code == sf::Keyboard::Key::Enter) {
                        resolveClassChoice(true);
                    } else if (keyEv->code == sf::Keyboard::Key::Num2 || keyEv->code == sf::Keyboard::Key::Numpad2
                               || keyEv->code == sf::Keyboard::Key::Escape) {
                        resolveClassChoice(false);
                    }
                }
                continue;
            }

            if (activityNotice.active) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    if (keyEv->code == sf::Keyboard::Key::Enter || keyEv->code == sf::Keyboard::Key::Escape) {
                        activityNotice.clear();
                    }
                } else if (event.is<sf::Event::MouseButtonPressed>()) {
                    activityNotice.clear();
                }
                continue;
            }

            // 页面切换
            if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                auto code = keyEv->code;
                if (code == sf::Keyboard::Key::Num1) {
                    page = DemoPage::ENTITY;
                    currentQuest = nullptr;
                    currentPlace = CampusPlace::Campus;
                    player.setPosition(480.0f, 276.0f);
                } else if (code == sf::Keyboard::Key::Num2) {
                    page = DemoPage::SIMPLE_QUEST;
                    resetSimpleDemo();
                } else if (code == sf::Keyboard::Key::Num3) {
                    page = DemoPage::MIDTERM_EXAM;
                    resetMidtermDemo();
                } else if (code == sf::Keyboard::Key::Num4) {
                    page = DemoPage::FINAL_EXAM;
                    resetFinalDemo();
                } else if (code == sf::Keyboard::Key::Num5) {
                    page = DemoPage::QUEST_MANAGER;
                    currentQuest = nullptr;
                } else if (code == sf::Keyboard::Key::Num0 || code == sf::Keyboard::Key::Num6) {
                    page = DemoPage::HELP;
                    currentQuest = nullptr;
                }
            }

            // Quest 模式输入
            if (currentQuest && !currentQuest->isCompleted()
                && (page == DemoPage::SIMPLE_QUEST || page == DemoPage::MIDTERM_EXAM
                    || page == DemoPage::FINAL_EXAM || page == DemoPage::QUEST_MANAGER)) {
                int choiceMade = -1;
                currentQuest->handleInput(event, player, choiceMade);

                if (currentQuest->isCompleted()) {
                    currentQuest->execute(player);
                    if (page == DemoPage::QUEST_MANAGER) {
                        questManager.onQuestCompleted();
                        currentQuest = nullptr;
                        questManagerQuest.reset();
                        std::cout << "[QuestManager] Quest completed, chain advanced to #"
                                  << questManager.getCurrentQuestIndex() << std::endl;
                    }
                }
            }

            // QuestManager 页面事件（仅在无活跃 quest 时触发）
            if (page == DemoPage::QUEST_MANAGER && !currentQuest) {
                if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                    auto code = keyEv->code;
                    if (code == sf::Keyboard::Key::E) {
                        questManager.onEventCompleted();
                        std::cout << "[QuestManager] Event completed! Total: "
                                  << questManager.getCompletedEventCount() << std::endl;
                    } else if (code == sf::Keyboard::Key::S) {
                        bool should = questManager.shouldTriggerQuest();
                        std::cout << "[QuestManager] shouldTriggerQuest() = "
                                  << (should ? "true" : "false")
                                  << "  (Events: " << questManager.getCompletedEventCount()
                                  << ", Threshold: " << questManager.getNextThreshold() << ")"
                                  << std::endl;
                    } else if (code == sf::Keyboard::Key::Enter) {
                        if (!questManager.shouldTriggerQuest()) {
                            std::cout << "[QuestManager] Conditions not met, cannot trigger next quest"
                                      << std::endl;
                        } else {
                            auto q = questManager.createNextQuest();
                            if (q) {
                                std::cout << "[QuestManager] Factory created: "
                                          << q->getQuestName() << " ("
                                          << q->getQuestId() << ")" << std::endl;
                                currentQuest = q.get();
                                questManagerQuest = std::move(q);
                            }
                        }
                    } else if (code == sf::Keyboard::Key::C) {
                        questManager = QuestManager();
                        questManager.loadQuestChain(cls::resolveAssetPath("assets/config/quests.json"));
                        currentQuest = nullptr;
                        questManagerQuest.reset();
                        std::cout << "[QuestManager] Reset complete" << std::endl;
                    }
                }
            }
        }

        // ── 持续性输入(移动) ─────────────────────────────────
        if (screen == GameScreen::TITLE) {
            titleScreen.update(dt);
            window.clear(sf::Color(20, 20, 30));
            titleScreen.render(window);
            window.display();
            continue;
        }

        if (screen == GameScreen::DIFFICULTY) {
            window.clear(sf::Color(20, 20, 30));
            difficultyPanel.render(window);
            window.display();
            continue;
        }

        if (sceneTransition.active) {
            window.clear(sf::Color(20, 20, 30));
            renderSceneTransition(window, font, sceneBackground, sceneTransition);
            window.display();
            continue;
        }

        if (timeSkipFlash.active) {
            window.clear(sf::Color(0, 0, 0));
            renderTimeSkipFlash(window, font, timeSkipFlash);
            window.display();
            continue;
        }

        if (page == DemoPage::ENTITY && !classChoicePrompt.active && !activityNotice.active) {
            float dx = 0.0f, dy = 0.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    dy = -1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  dy = 1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dx = -1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dx = 1.0f;

            if (dx != 0.0f && dy != 0.0f) {
                float inv = 1.0f / std::sqrt(2.0f);
                dx *= inv;
                dy *= inv;
            }

            player.move(dx, dy, dt);

            // 按键 C = 压力事件（降低 SAN，触发敌人出现）
            if (justPressed(sf::Keyboard::Key::C)) {
                player.modifyAttributes(Attributes(-15, 0, 0, 0, 0));
                int lvl = player.getSanLevel();
                std::cout << "[Stress] SAN dropped to " << player.getAttributes().san
                          << " Level=" << lvl << std::endl;
                // 尝试生成敌人（SAN 越低概率越大）
                trySpawnEnemy();
            }

            // 按键 F = 与附近敌人战斗
            if (justPressed(sf::Keyboard::Key::F)) {
                if (!fightNearestEnemy()) {
                    std::cout << "[Combat] No enemy nearby! Get closer or spawn one first (press C)." << std::endl;
                }
            }

            // 按键 V = 恢复 SAN（模拟休息/自我关怀）
            if (justPressed(sf::Keyboard::Key::V)) {
                player.modifyAttributes(Attributes(15, 0, 0, 0, 0));
                int lvl = player.getSanLevel();
                // 恢复后重新缩放已生成的敌人
                for (auto& e : activeEnemies) {
                    e->scaleWithSanLevel(lvl);
                }
                std::cout << "[Rest] SAN restored to " << player.getAttributes().san
                          << " Level=" << lvl << std::endl;
                // SAN 恢复后部分敌人消失
                if (lvl == 0) {
                    activeEnemies.clear();
                    std::cout << "[Combat] All enemies retreated (SAN >= 30)" << std::endl;
                } else if (lvl == 1 && static_cast<int>(activeEnemies.size()) > 1) {
                    activeEnemies.resize(1);
                    std::cout << "[Combat] Enemies reduced to 1 (SAN 20-29)" << std::endl;
                }
            }

            // 按键 X = 设置战斗 buff
            if (justPressed(sf::Keyboard::Key::X)) {
                player.getCombatBuffs().nextEventPositive = true;
                player.getCombatBuffs().nextRollModifier = 2;
                std::cout << "[Buff] Victory buff set: +2 to rolls" << std::endl;
            }

            if (justPressed(sf::Keyboard::Key::Enter)) {
                // 第一层：场景传送门（进出建筑）
                bool portalFound = false;
                for (const auto& portal : currentMap->getPortals()) {
                    if (pointInRect(player.getPosition(), portal.area)) {
                        startMapTransition(portal);
                        portalFound = true;
                        break;
                    }
                }
                // 第二层：家具交互点
                if (!portalFound) {
                    const InteractionPoint* ip = currentMap->getInteractionAt(player.getPosition());
                    if (ip) {
                        std::cout << "[Interact] " << ip->label
                                  << " (" << ip->actionId << ")"
                                  << " — " << ip->description << std::endl;
                        handleInteraction(*ip);
                    }
                }
            }

            player.update(dt);

            // 边界限制，防止走出地图
            currentMap->clampPlayer(player);
        }

        // ── 渲染 ──────────────────────────────────────────────
        window.clear(sf::Color(20, 20, 30));

        switch (page) {
            case DemoPage::ENTITY:
                currentMap->render(window);
                player.render(window);
                break;
            case DemoPage::SIMPLE_QUEST:
            case DemoPage::MIDTERM_EXAM:
            case DemoPage::FINAL_EXAM:
                if (currentQuest) {
                    renderQuestUI(window, font, currentQuest, &player);
                }
                break;
            case DemoPage::QUEST_MANAGER:
                renderQuestManagerDemo(window, font, questManager);
                if (currentQuest && !currentQuest->isCompleted()) {
                    renderQuestUI(window, font, currentQuest, &player);
                }
                break;
            case DemoPage::HELP: {
                HelpPanel helpPanel(font);
                helpPanel.render(window);
                break;
            }
        }

        // 顶部属性面板（所有页面通用）
        if (fontOk) {
        if (fontOk) {
            renderStatsPanel(window, font, player, page);
            renderTimePanel(window, font, timeSystem);
        }
        renderNotice(window, font, activityNotice);
        renderChoicePrompt(window, font, classChoicePrompt);

        window.display();
    }

    return 0;
}

