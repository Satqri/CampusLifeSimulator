/**
 * @file main.cpp
 * @brief Campus Life Simulator - Demo
 *
 * Controls:
 *   WASD/Arrows - Move
 *   1 - Entity demo (explore + SAN trigger combat)
 *   2 - SimpleQuest demo
 *   3 - MidtermExam demo
 *   4 - FinalExam demo
 *   5 - QuestManager demo
 *   0/6 - Help
 *   C - Stress (lower SAN)
 *   F - Fight nearby enemy
 *   L - Interact (dice game, canteen, sleep)
 *   Enter - Portal / interact
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "core/AssetPath.h"
#include "core/Types.h"
#include "entity/Player.h"
#include "entity/Enemy.h"
#include "entity/Student.h"
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
#include "state/DiceGameState.h"
#include "state/CanteenGameState.h"

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

class Game {
public:
    sf::RenderWindow* window = nullptr;
};

enum class DemoPage {
    ENTITY, SIMPLE_QUEST, MIDTERM_EXAM,
    FINAL_EXAM, QUEST_MANAGER, HELP
};

enum class GameScreen { TITLE, DIFFICULTY, GAME };

struct CombatResult {
    bool active = false, victory = false;
    float timer = 0.0f;
    std::string enemyName;
    int d20Roll = 0, modifier = 0, total = 0, dc = 0;

    void show(bool win, const std::string& name, int d20, int mod, int tot, int d) {
        active = true; victory = win; timer = 3.0f;
        enemyName = name; d20Roll = d20; modifier = mod; total = tot; dc = d;
    }
    void update(float dt) { if (active) { timer -= dt; if (timer <= 0.0f) active = false; } }
    void clear() { active = false; timer = 0.0f; }
};

struct SceneTransition {
    bool active = false; float timer = 0.0f;
    SceneBackgroundType background = SceneBackgroundType::Dormitory;
    std::string title, subtitle;

    void start(SceneBackgroundType bg, const std::string& h, const std::string& l) {
        active = true; timer = 0.0f; background = bg; title = h; subtitle = l;
    }
    void update(float dt) { if (active) timer += dt; }
    bool canContinue() const { return timer >= 0.45f; }
    void skip() { active = false; timer = 0.0f; }
};

static int statForEmotion(const Player& player, EmotionType type) {
    const auto& a = player.getAttributes();
    switch (type) {
        case EmotionType::ANXIETY:    return a.academic;
        case EmotionType::ANGER:      return a.energy;
        case EmotionType::LONELINESS: return a.social;
        case EmotionType::DEPRESSION: case EmotionType::FEAR: return a.san;
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

void renderStatsPanel(sf::RenderWindow& w, sf::Font& f, const Player& player, DemoPage page) {
    const char* names[] = {"Entity","SimpleQuest","MidtermExam","FinalExam","QuestManager","Help"};
    HUD hud(f); hud.setPlayer(&player); hud.setPageName(names[static_cast<int>(page)]); hud.render(w);
}

void applyDifficulty(Player& p, Difficulty d) {
    switch (d) {
        case Difficulty::Easy:   p.modifyAttributes(Attributes(20,15,5,5,0)); break;
        case Difficulty::Normal: break;
        case Difficulty::Hard:   p.modifyAttributes(Attributes(-20,-10,0,-5,0)); break;
    }
}

void renderSceneTransition(sf::RenderWindow& w, sf::Font& f,
                           SceneBackground& bg, const SceneTransition& tr) {
    bg.render(w, tr.background, sf::Color(0,0,0,70));
    sf::RectangleShape plate({640,132}); plate.setPosition({160,332});
    plate.setFillColor(sf::Color(9,22,28,176)); plate.setOutlineColor(sf::Color(230,212,148,170)); plate.setOutlineThickness(2); w.draw(plate);
    sf::Text t1(f, tr.title, 34); t1.setFillColor(sf::Color(250,240,205)); t1.setOutlineColor(sf::Color(18,42,45)); t1.setOutlineThickness(2); t1.setPosition({190,352}); w.draw(t1);
    sf::Text t2(f, tr.subtitle, 18); t2.setFillColor(sf::Color(224,238,220)); t2.setPosition({190,407}); w.draw(t2);
    bool ready = tr.canContinue();
    sf::Text h(f, ready ? "Press Enter to enter" : "Entering...", 13);
    h.setFillColor(ready ? sf::Color(210,210,190,180) : sf::Color(210,210,190,110)); h.setPosition({190,444}); w.draw(h);
}

void renderQuestUI(sf::RenderWindow& w, sf::Font& f, MainQuest* q, Player* /*p*/) {
    if (!q) return; QuestPanel qp(f); qp.setQuest(q); qp.render(w);
}

void renderQuestManagerDemo(sf::RenderWindow& w, sf::Font& f, QuestManager& qm) {
    sf::RectangleShape bg({960,540}); bg.setFillColor(sf::Color(15,15,25,220)); w.draw(bg);
    sf::Text t(f, "QuestManager -- JSON Factory + Quest Chain", 26); t.setFillColor(sf::Color::White); t.setPosition({40,30}); w.draw(t);
    std::ostringstream info;
    info << "Events: " << qm.getCompletedEventCount() << " | Quest: " << qm.getCurrentQuestIndex() << "/" << qm.getTotalQuests()
         << " | Progress: " << std::fixed << std::setprecision(1) << (qm.getSemesterProgress()*100) << "%";
    sf::Text it(f, info.str(), 14); it.setFillColor(sf::Color(180,200,230)); it.setPosition({40,75}); w.draw(it);
    auto* curr = qm.getCurrentQuest();
    if (curr) {
        sf::RectangleShape ab({880,100}); ab.setFillColor(sf::Color(30,60,30,200)); ab.setPosition({40,110}); w.draw(ab);
        sf::Text al(f, "> Active Quest", 16); al.setFillColor(sf::Color(100,255,100)); al.setPosition({50,115}); w.draw(al);
        sf::Text an(f, curr->getQuestName(), 20); an.setFillColor(sf::Color::White); an.setPosition({50,140}); w.draw(an);
        sf::Text ad(f, curr->getDescription(), 14); ad.setFillColor(sf::Color(200,200,200)); ad.setPosition({50,170}); w.draw(ad);
    }
    sf::Text ct(f, "Quest Chain:", 16); ct.setFillColor(sf::Color(200,200,200)); ct.setPosition({40,230}); w.draw(ct);
    using json = nlohmann::json;
    std::ifstream fs(cls::resolveAssetPath("assets/config/quests.json"));
    if (fs.is_open()) {
        json data = json::parse(fs); int idx = 0;
        for (const auto& q : data["quests"]) {
            float y = 260.f + idx * 30.f; std::string ts = q.value("type","?");
            sf::Color rc = (idx == qm.getCurrentQuestIndex()) ? sf::Color(255,200,100) : sf::Color(150,150,150);
            std::ostringstream ln;
            ln << "#" << idx << " [" << q.value("threshold",0) << " trigger] " << q.value("name","???")
               << " (" << (ts=="midterm_exam"||ts=="final_exam"?"Exam":"Simple") << ")";
            sf::Text rw(f, ln.str(), 13); rw.setFillColor(rc); rw.setPosition({60,y}); w.draw(rw);
            idx++;
        }
    }
    sf::Text ht(f, "[Enter] Create next quest | [S] Trigger check | [E] +1 event | [C] Reset", 13);
    ht.setFillColor(sf::Color(130,130,150)); ht.setPosition({40,490}); w.draw(ht);
}

// ============================================================================
//  main
// ============================================================================
int main() {
    sf::RenderWindow window(sf::VideoMode({kWindowWidth, kWindowHeight}), "CampusLifeSimulator");
    window.setFramerateLimit(60); window.setKeyRepeatEnabled(false);
    sf::View gameView(sf::FloatRect({0,0},{kRenderWidth,kRenderHeight})); window.setView(gameView);

    sf::Font font; bool fontOk = false;
#if defined(_WIN32)
    for (auto& p : {"C:/Windows/Fonts/msyh.ttc","C:/Windows/Fonts/msyh.ttf","C:/Windows/Fonts/simhei.ttf","C:/Windows/Fonts/arial.ttf"})
        if (font.openFromFile(p)) { fontOk = true; break; }
#elif defined(__APPLE__)
    fontOk = font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf") || font.openFromFile("/System/Library/Fonts/PingFang.ttc");
#else
    fontOk = font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
#endif
    if (!fontOk) std::cerr << "ERROR: Failed to load font!" << std::endl;

    TitleScreen titleScreen(font, "assets/ui/campus_title_bg.png");
    DifficultyPanel difficultyPanel(font);
    SceneBackground sceneBackground;
    SceneTransition sceneTransition;
    GameScreen screen = GameScreen::TITLE;
    Difficulty selectedDifficulty = Difficulty::Normal;
    bool difficultyApplied = false;

    auto campusMap    = std::make_unique<CampusMap>();
    auto dormitoryMap = std::make_unique<DormitoryInterior>();
    auto gymMap       = std::make_unique<GymInterior>();
    auto libraryMap   = std::make_unique<LibraryInterior>();
    auto classroomMap = std::make_unique<ClassroomInterior>();
    auto cafeteriaMap = std::make_unique<CafeteriaInterior>();

    campusMap->setFont(&font); dormitoryMap->setFont(&font); gymMap->setFont(&font);
    libraryMap->setFont(&font); classroomMap->setFont(&font); cafeteriaMap->setFont(&font);

    BuildingInterior* currentMap = campusMap.get();
    auto setCurrentMap = [&](CampusPlace p) -> BuildingInterior* {
        switch (p) {
            case CampusPlace::Campus:    return campusMap.get();
            case CampusPlace::Dormitory: return dormitoryMap.get();
            case CampusPlace::Gym:       return gymMap.get();
            case CampusPlace::Library:   return libraryMap.get();
            case CampusPlace::Classroom: return classroomMap.get();
            case CampusPlace::Cafeteria: return cafeteriaMap.get();
        }
        return campusMap.get();
    };

    Player player(480, 280); player.setName("Protagonist");
    CampusPlace currentPlace = CampusPlace::Campus;
    CampusPlace pendingPlace = CampusPlace::Campus;
    sf::Vector2f pendingSpawn(480, 276);
    bool hasPending = false;

    // NPCs
    Student dormStudent(800, 140);
    dormStudent.loadTexture("assets/gameset/dormitory/student.png");
    dormitoryMap->setStudent(&dormStudent);

    Student cafeteriaNpc(640, 140);
    cafeteriaNpc.loadTexture("assets/gameset/dormitory/student.png");
    cafeteriaMap->setStudent(&cafeteriaNpc);

    // Game state objects
    Game game; game.window = &window;
    std::unique_ptr<DiceGameState> diceGameState;
    std::unique_ptr<CanteenGameState> canteenGameState;

    std::vector<std::unique_ptr<Enemy>> activeEnemies;
    CombatResult combatResult;

    QuestManager questManager;
    questManager.loadQuestChain(cls::resolveAssetPath("assets/config/quests.json"));
    MainQuest* currentQuest = nullptr;
    std::unique_ptr<SimpleQuest> simpleQuest;
    std::unique_ptr<MidtermExamQuest> midtermQuest;
    std::unique_ptr<FinalExamQuest> finalExamQuest;
    std::unique_ptr<MainQuest> questManagerQuest;

    DemoPage page = DemoPage::ENTITY;
    sf::Clock clock;
    bool keyWasPressed[static_cast<int>(sf::Keyboard::KeyCount)] = {};
    auto justPressed = [&](sf::Keyboard::Key k) -> bool {
        bool p = sf::Keyboard::isKeyPressed(k), prev = keyWasPressed[static_cast<int>(k)];
        keyWasPressed[static_cast<int>(k)] = p; return p && !prev;
    };

    int spawnCounter = 0;

    auto trySpawnEnemy = [&]() {
        int lvl = player.getSanLevel(); if (lvl == 0) return;
        int maxE = lvl == 1 ? 1 : lvl == 2 ? 2 : 3;
        if (static_cast<int>(activeEnemies.size()) >= maxE) return;
        int chance = lvl == 1 ? 40 : lvl == 2 ? 60 : 90;
        if ((std::rand()%100) >= chance) return;
        EmotionType types[] = {EmotionType::ANXIETY,EmotionType::DEPRESSION,EmotionType::ANGER,EmotionType::FEAR,EmotionType::LONELINESS};
        auto e = std::make_unique<Enemy>(
            std::clamp(player.getPosition().x + (std::rand()%160-80), 40.f, 920.f),
            std::clamp(player.getPosition().y + (std::rand()%160-80), 80.f, 500.f),
            types[spawnCounter%5], 12, 5);
        e->scaleWithSanLevel(lvl); activeEnemies.push_back(std::move(e)); spawnCounter++;
        std::cout << "[Combat] Enemy appeared! SAN=" << player.getAttributes().san << " Lvl=" << lvl << std::endl;
    };

    auto fightNearestEnemy = [&]() -> bool {
        if (activeEnemies.empty() || combatResult.active) return false;
        float md = 100; int ni = -1; sf::Vector2f pp = player.getPosition();
        for (int i = 0; i < static_cast<int>(activeEnemies.size()); ++i) {
            auto ep = activeEnemies[i]->getPosition();
            float d = std::sqrt((pp.x-ep.x)*(pp.x-ep.x)+(pp.y-ep.y)*(pp.y-ep.y));
            if (d < md) { md = d; ni = i; }
        }
        if (ni < 0) return false;
        auto& en = *activeEnemies[ni]; auto et = en.getEmotionType();
        int sv = statForEmotion(player, et), mod = (sv-50)/10, bm = player.getCombatBuffs().nextRollModifier;
        int d20 = (std::rand()%20)+1, total = d20 + mod + bm; bool win = total >= en.getDC();
        if (win) { player.modifyAttributes(Attributes(20,0,0,0,0)); player.getCombatBuffs().nextEventPositive=true; player.getCombatBuffs().nextRollModifier=2; }
        else     { player.modifyAttributes(Attributes(-15,0,0,0,0)); player.getCombatBuffs().nextEventPositive=false; player.getCombatBuffs().nextRollModifier=-2; }
        combatResult.show(win, en.getName(), d20, mod+bm, total, en.getDC());
        activeEnemies.erase(activeEnemies.begin()+ni); return true;
    };

    auto resetSimpleDemo = [&]() {
        simpleQuest = std::make_unique<SimpleQuest>("orientation_demo","Freshman Orientation",
            "You've just stepped onto the university campus...",
            std::vector<std::pair<std::string, Attributes>>{
                {"Explore campus", Attributes(0,-5,0,10,0)},
                {"Check library",  Attributes(0,-3,8,3,0)},
                {"Grab a meal",    Attributes(5,10,0,5,-20)}},
            Attributes(0,0,0,5,0));
        currentQuest = simpleQuest.get();
    };
    auto resetMidtermDemo = [&]() { midtermQuest = std::make_unique<MidtermExamQuest>(); midtermQuest->setPhase(QuestPhase::ANNOUNCEMENT); currentQuest = midtermQuest.get(); };
    auto resetFinalDemo  = [&]() { finalExamQuest = std::make_unique<FinalExamQuest>(); finalExamQuest->setPhase(QuestPhase::ANNOUNCEMENT); currentQuest = finalExamQuest.get(); };
    auto startMapTransition = [&](const MapPortal& portal) {
        pendingPlace = portal.target; pendingSpawn = portal.spawnPosition; hasPending = true;
        sceneTransition.start(portal.transitionBackground, portal.title, portal.subtitle);
    };
    auto finishSceneTransition = [&]() {
        if (hasPending) { currentPlace = pendingPlace; currentMap = setCurrentMap(pendingPlace);
            player.setPosition(pendingSpawn.x, pendingSpawn.y); player.stopMovement(); hasPending = false; }
        sceneTransition.skip();
    };

    resetSimpleDemo();

    // ======== MAIN LOOP ========
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        combatResult.update(dt); sceneBackground.update(dt); sceneTransition.update(dt);

        // Update sub-games
        if (diceGameState) { if (diceGameState->isRunning()) diceGameState->update(dt); else diceGameState.reset(); }
        if (canteenGameState) { if (canteenGameState->isRunning()) canteenGameState->update(dt); else canteenGameState.reset(); }

        // ---- Event handling ----
        while (auto eo = window.pollEvent()) {
            auto& ev = *eo;
            if (ev.is<sf::Event::Closed>()) { window.close(); return 0; }

            if (diceGameState && diceGameState->isRunning()) { diceGameState->handleInput(ev); continue; }
            if (canteenGameState && canteenGameState->isRunning()) { canteenGameState->handleInput(ev); continue; }

            if (sceneTransition.active) {
                if (auto* ke = ev.getIf<sf::Event::KeyPressed>())
                    if (sceneTransition.canContinue() && (ke->code==sf::Keyboard::Key::Enter || ke->code==sf::Keyboard::Key::Escape))
                        finishSceneTransition();
                else if (ev.is<sf::Event::MouseButtonPressed>() && sceneTransition.canContinue()) finishSceneTransition();
                continue;
            }

            if (screen == GameScreen::TITLE) {
                if (auto* ke = ev.getIf<sf::Event::KeyPressed>()) {
                    if (ke->code == sf::Keyboard::Key::Enter) screen = GameScreen::DIFFICULTY;
                    else if (ke->code == sf::Keyboard::Key::H) { page = DemoPage::HELP; currentQuest = nullptr; screen = GameScreen::GAME; }
                } else if (auto* me = ev.getIf<sf::Event::MouseButtonPressed>()) {
                    auto a = titleScreen.handleClick({static_cast<float>(me->position.x), static_cast<float>(me->position.y)});
                    if (a == TitleAction::Start) screen = GameScreen::DIFFICULTY;
                    else if (a == TitleAction::Help) { page = DemoPage::HELP; currentQuest = nullptr; screen = GameScreen::GAME; }
                }
                continue;
            }

            if (screen == GameScreen::DIFFICULTY) {
                auto startGame = [&](Difficulty d) { selectedDifficulty=d; if(!difficultyApplied){applyDifficulty(player,d);difficultyApplied=true;} page=DemoPage::ENTITY; currentQuest=nullptr; currentPlace=CampusPlace::Campus; player.setPosition(480,276); screen=GameScreen::GAME; };
                if (auto* ke = ev.getIf<sf::Event::KeyPressed>()) {
                    if (ke->code == sf::Keyboard::Key::Escape) screen = GameScreen::TITLE;
                    else if (ke->code == sf::Keyboard::Key::Num1) startGame(Difficulty::Easy);
                    else if (ke->code == sf::Keyboard::Key::Num2 || ke->code == sf::Keyboard::Key::Enter) startGame(Difficulty::Normal);
                    else if (ke->code == sf::Keyboard::Key::Num3) startGame(Difficulty::Hard);
                } else if (auto* me = ev.getIf<sf::Event::MouseButtonPressed>()) {
                    auto a = difficultyPanel.handleClick({static_cast<float>(me->position.x), static_cast<float>(me->position.y)});
                    if (a.type == DifficultyActionType::Back) screen = GameScreen::TITLE;
                    else if (a.type == DifficultyActionType::Select) startGame(a.difficulty);
                }
                continue;
            }

            // Page switch
            if (auto* ke = ev.getIf<sf::Event::KeyPressed>()) {
                auto c = ke->code;
                if (c == sf::Keyboard::Key::Num1) { page=DemoPage::ENTITY; currentQuest=nullptr; currentPlace=CampusPlace::Campus; player.setPosition(480,276); }
                else if (c == sf::Keyboard::Key::Num2) { page=DemoPage::SIMPLE_QUEST; resetSimpleDemo(); }
                else if (c == sf::Keyboard::Key::Num3) { page=DemoPage::MIDTERM_EXAM; resetMidtermDemo(); }
                else if (c == sf::Keyboard::Key::Num4) { page=DemoPage::FINAL_EXAM; resetFinalDemo(); }
                else if (c == sf::Keyboard::Key::Num5) { page=DemoPage::QUEST_MANAGER; currentQuest=nullptr; }
                else if (c == sf::Keyboard::Key::Num0 || c == sf::Keyboard::Key::Num6) { page=DemoPage::HELP; currentQuest=nullptr; }
            }

            // Quest input
            if (currentQuest && !currentQuest->isCompleted() &&
                (page==DemoPage::SIMPLE_QUEST||page==DemoPage::MIDTERM_EXAM||page==DemoPage::FINAL_EXAM||page==DemoPage::QUEST_MANAGER)) {
                int cm = -1; currentQuest->handleInput(ev, player, cm);
                if (currentQuest->isCompleted()) { currentQuest->execute(player); if (page==DemoPage::QUEST_MANAGER) { questManager.onQuestCompleted(); currentQuest=nullptr; questManagerQuest.reset(); } }
            }

            // QuestManager page
            if (page == DemoPage::QUEST_MANAGER && !currentQuest) {
                if (auto* ke = ev.getIf<sf::Event::KeyPressed>()) {
                    auto c = ke->code;
                    if (c == sf::Keyboard::Key::E) questManager.onEventCompleted();
                    else if (c == sf::Keyboard::Key::S) questManager.shouldTriggerQuest();
                    else if (c == sf::Keyboard::Key::Enter) { if (questManager.shouldTriggerQuest()) { auto q = questManager.createNextQuest(); if (q) { currentQuest = q.get(); questManagerQuest = std::move(q); } } }
                    else if (c == sf::Keyboard::Key::C) { questManager = QuestManager(); questManager.loadQuestChain(cls::resolveAssetPath("assets/config/quests.json")); currentQuest=nullptr; questManagerQuest.reset(); }
                }
            }
        }

        // ---- Continuous input ----
        if (screen == GameScreen::TITLE) { titleScreen.update(dt); window.clear(sf::Color(20,20,30)); titleScreen.render(window); window.display(); continue; }
        if (screen == GameScreen::DIFFICULTY) { window.clear(sf::Color(20,20,30)); difficultyPanel.render(window); window.display(); continue; }
        if (sceneTransition.active) { window.clear(sf::Color(20,20,30)); renderSceneTransition(window,font,sceneBackground,sceneTransition); window.display(); continue; }

        if (page == DemoPage::ENTITY) {
            float dx=0, dy=0;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) dy=-1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) dy=1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) dx=-1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dx=1;
            if (dx!=0&&dy!=0) { float inv=1.f/std::sqrt(2.f); dx*=inv; dy*=inv; }
            player.move(dx, dy, dt);

            // C - stress
            if (justPressed(sf::Keyboard::Key::C)) { player.modifyAttributes(Attributes(-15,0,0,0,0)); trySpawnEnemy(); }
            // F - fight
            if (justPressed(sf::Keyboard::Key::F)) fightNearestEnemy();
            // V - rest
            if (justPressed(sf::Keyboard::Key::V)) { player.modifyAttributes(Attributes(15,0,0,0,0)); for (auto& e : activeEnemies) e->scaleWithSanLevel(player.getSanLevel()); if (player.getSanLevel()==0) activeEnemies.clear(); }
            // X - buff
            if (justPressed(sf::Keyboard::Key::X)) { player.getCombatBuffs().nextEventPositive=true; player.getCombatBuffs().nextRollModifier=2; }
            // N - debug: advance to next day (reset canteen + stamina)
            if (justPressed(sf::Keyboard::Key::N)) { player.advanceToNextDay(); player.consumeStamina(20); std::cout << "[Debug] Day advanced. Stamina -20. Canteen reset." << std::endl; }

            // ---- L-key interaction system ----
            if (justPressed(sf::Keyboard::Key::L)) {
                bool handled = false;

                // Dormitory: dice game with student
                if (!handled && currentPlace==CampusPlace::Dormitory && !diceGameState && !canteenGameState && dormStudent.isNearPlayer(player.getPosition(), 60)) {
                    diceGameState = std::make_unique<DiceGameState>(&game, &player, &dormStudent);
                    diceGameState->setFont(&font); handled = true;
                }

                // Cafeteria: canteen game with chef NPC (always available for testing)
                if (!handled && currentPlace==CampusPlace::Cafeteria && !diceGameState && !canteenGameState && cafeteriaNpc.isNearPlayer(player.getPosition(), 60)) {
                    canteenGameState = std::make_unique<CanteenGameState>(&game, &player);
                    canteenGameState->setFont(&font);
                    handled = true;
                }

                // Dormitory: sleep on bed
                if (!handled && currentPlace==CampusPlace::Dormitory && !diceGameState && !canteenGameState) {
                    const InteractionPoint* ip = currentMap->getInteractionAt(player.getPosition());
                    if (ip && ip->actionId == "dormitory_bed") { player.advanceToNextDay(); handled = true; }
                }
            }

            // Enter: portal / interact
            if (justPressed(sf::Keyboard::Key::Enter)) {
                bool pf = false;
                for (auto& p : currentMap->getPortals()) if (pointInRect(player.getPosition(), p.area)) { startMapTransition(p); pf=true; break; }
                if (!pf) {
                    const InteractionPoint* ip = currentMap->getInteractionAt(player.getPosition());
                    if (ip) std::cout << "[Interact] " << ip->label << " (" << ip->actionId << ")" << std::endl;
                }
            }

            player.update(dt); currentMap->clampPlayer(player);
        }

        // ---- Render ----
        window.clear(sf::Color(20,20,30));

        if (diceGameState && diceGameState->isRunning()) { diceGameState->render(window); window.display(); continue; }
        if (canteenGameState && canteenGameState->isRunning()) { canteenGameState->render(window); window.display(); continue; }

        switch (page) {
            case DemoPage::ENTITY: currentMap->render(window); player.render(window); break;
            case DemoPage::SIMPLE_QUEST: case DemoPage::MIDTERM_EXAM: case DemoPage::FINAL_EXAM:
                if (currentQuest) renderQuestUI(window, font, currentQuest, &player); break;
            case DemoPage::QUEST_MANAGER: renderQuestManagerDemo(window, font, questManager);
                if (currentQuest && !currentQuest->isCompleted()) renderQuestUI(window, font, currentQuest, &player); break;
            case DemoPage::HELP: { HelpPanel hp(font); hp.render(window); break; }
        }

        if (fontOk) renderStatsPanel(window, font, player, page);
        window.display();
    }
    return 0;
}