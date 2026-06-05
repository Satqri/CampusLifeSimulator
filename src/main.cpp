/**
 * @file main.cpp
 * @brief 绫绘紨绀虹▼搴?鈥?灞曠ず Entity/Quest/State/Manager 鍚勭户鎵夸綋绯荤殑鎵€鏈夌被
 *
 * 鎿嶄綔璇存槑:
 *   WASD/鏂瑰悜閿?鈥?绉诲姩鐜╁
 *   1 鈥?Entity 婕旂ず锛堟帰绱?+ SAN 闃堝€艰Е鍙戞垬鏂楋級
 *   2 鈥?SimpleQuest 婕旂ず锛堟柊鐢熸姤鍒?3 娈靛紡浠诲姟锛?
 *   3 鈥?MidtermExamQuest 婕旂ず锛堟湡涓€冭瘯 d20 妫€瀹氾級
 *   4 鈥?FinalExamQuest 婕旂ず锛堟湡鏈€冭瘯 d20 妫€瀹氾級
 *   5 鈥?QuestManager 婕旂ず锛圝SON 鍔犺浇 + 宸ュ巶鍒涘缓 + 浠诲姟閾撅級
 *   C 鈥?闄嶄綆 SAN / 娓呴櫎 buff
 *   F 鈥?涓庨檮杩戞晫浜烘垬鏂?
 *   Enter/鈫戔啌 鈥?浠诲姟妯″紡涓嬫搷浣?
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "core/Types.h"
#include "entity/Player.h"
#include "entity/Enemy.h"
#include "quest/MainQuest.h"
#include "quest/SimpleQuest.h"
#include "quest/ExamQuest.h"
#include "quest/MidtermExamQuest.h"
#include "quest/FinalExamQuest.h"
#include "quest/QuestManager.h"
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
#include <nlohmann/json.hpp>

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// 鏈€灏?Game 绫?鈥?MainQuestState 鐨勪緷璧栵紙褰撳墠鏈娇鐢?Game* 鎴愬憳锛?
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
class Game {
public:
    sf::RenderWindow* window = nullptr;
};

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// 婕旂ず妯″紡
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
enum class DemoPage {
    ENTITY,          // Entity 缁ф壙浣撶郴婕旂ず
    SIMPLE_QUEST,    // SimpleQuest 婕旂ず
    MIDTERM_EXAM,    // MidtermExamQuest 婕旂ず
    FINAL_EXAM,      // FinalExamQuest 婕旂ず
    QUEST_MANAGER,   // QuestManager demo
    HELP             // Help and settings page
};

enum class GameScreen {
    TITLE,
    DIFFICULTY,
    GAME
};

enum class CampusPlace {
    Campus,
    Dormitory,
    Library,
    Classroom,
    Cafeteria
};

struct MapPortal {
    sf::FloatRect area;
    CampusPlace target;
    SceneBackgroundType transitionBackground;
    sf::Vector2f spawnPosition;
    std::string title;
    std::string subtitle;
};

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// 鎴樻枟缁撴灉
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
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

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// 鏍规嵁鎯呯华绫诲瀷鑾峰彇瀵瑰簲鐜╁灞炴€у€硷紙鐢ㄤ簬鎴樻枟妫€瀹氾級
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
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

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// 娓叉煋褰撳墠灞炴€ч潰鏉匡紙鎵€鏈夋ā寮忎笅閮藉湪椤堕儴鏄剧ず锛?
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
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

bool pointInRect(sf::Vector2f point, const sf::FloatRect& rect) {
    return point.x >= rect.position.x
        && point.x <= rect.position.x + rect.size.x
        && point.y >= rect.position.y
        && point.y <= rect.position.y + rect.size.y;
}

bool loadTextureFromCandidates(sf::Texture& texture, const std::string& relativePath) {
    const std::array<std::string, 4> candidates = {
        relativePath,
        "../../../" + relativePath,
        "../../../../" + relativePath,
        "D:/Campus_2D/CampusLifeSimulator/" + relativePath
    };
    for (const auto& path : candidates) {
        if (texture.loadFromFile(path)) return true;
    }
    return false;
}

void drawPixlabSprite(sf::RenderWindow& window, const sf::Texture& texture,
                      const sf::IntRect& textureRect, sf::Vector2f position, float scale = 1.0f) {
    sf::Sprite sprite(texture);
    sprite.setTextureRect(textureRect);
    sprite.setPosition(position);
    sprite.setScale({scale, scale});
    window.draw(sprite);
}

std::vector<MapPortal> portalsForPlace(CampusPlace place) {
    if (place == CampusPlace::Campus) {
        return {
            MapPortal{sf::FloatRect({80.0f, 86.0f}, {150.0f, 92.0f}), CampusPlace::Dormitory, SceneBackgroundType::Dormitory,
                {480.0f, 448.0f}, "Dormitory", "Backpacks drop by the bed; the next plan starts from a quiet room."},
            MapPortal{sf::FloatRect({702.0f, 82.0f}, {168.0f, 96.0f}), CampusPlace::Library, SceneBackgroundType::Library,
                {480.0f, 448.0f}, "Library", "Between shelves and desk lamps, tomorrow's answers begin to take shape."},
            MapPortal{sf::FloatRect({92.0f, 352.0f}, {176.0f, 104.0f}), CampusPlace::Classroom, SceneBackgroundType::Classroom,
                {480.0f, 448.0f}, "Classroom", "The bell rings softly; notes, questions, and pressure wait inside."},
            MapPortal{sf::FloatRect({690.0f, 350.0f}, {182.0f, 106.0f}), CampusPlace::Cafeteria, SceneBackgroundType::Cafeteria,
                {480.0f, 448.0f}, "Cafeteria", "Warm food and noisy tables make the campus feel briefly lighter."}
        };
    }

    SceneBackgroundType returnBackground = SceneBackgroundType::Dormitory;
    if (place == CampusPlace::Library) returnBackground = SceneBackgroundType::Library;
    if (place == CampusPlace::Classroom) returnBackground = SceneBackgroundType::Classroom;
    if (place == CampusPlace::Cafeteria) returnBackground = SceneBackgroundType::Cafeteria;

    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus, returnBackground,
            {480.0f, 276.0f}, "Campus Square", "The main paths open again; choose where the day goes next."}
    };
}

std::string placeName(CampusPlace place) {
    switch (place) {
        case CampusPlace::Campus: return "Campus Map";
        case CampusPlace::Dormitory: return "Dormitory";
        case CampusPlace::Library: return "Library";
        case CampusPlace::Classroom: return "Classroom";
        case CampusPlace::Cafeteria: return "Cafeteria";
    }
    return "Campus";
}

void drawLabel(sf::RenderWindow& window, sf::Font& font, const std::string& text,
               sf::Vector2f position, unsigned int size = 13) {
    sf::Text label(font, text, size);
    label.setFillColor(sf::Color(244, 238, 206));
    label.setOutlineColor(sf::Color(30, 34, 30));
    label.setOutlineThickness(1.0f);
    label.setPosition(position);
    window.draw(label);
}

void drawBuilding(sf::RenderWindow& window, sf::Font& font, const MapPortal& portal,
                  const std::string& label, sf::Color body, sf::Color roof) {
    sf::RectangleShape shadow({portal.area.size.x + 12.0f, portal.area.size.y + 12.0f});
    shadow.setPosition({portal.area.position.x + 6.0f, portal.area.position.y + 8.0f});
    shadow.setFillColor(sf::Color(20, 28, 24, 90));
    window.draw(shadow);

    sf::RectangleShape roofShape({portal.area.size.x + 18.0f, 24.0f});
    roofShape.setPosition({portal.area.position.x - 9.0f, portal.area.position.y - 18.0f});
    roofShape.setFillColor(roof);
    window.draw(roofShape);

    sf::RectangleShape bodyShape(portal.area.size);
    bodyShape.setPosition(portal.area.position);
    bodyShape.setFillColor(body);
    bodyShape.setOutlineColor(sf::Color(78, 68, 48));
    bodyShape.setOutlineThickness(2.0f);
    window.draw(bodyShape);

    sf::RectangleShape door({34.0f, 32.0f});
    door.setPosition({portal.area.position.x + portal.area.size.x / 2.0f - 17.0f,
                      portal.area.position.y + portal.area.size.y - 32.0f});
    door.setFillColor(sf::Color(72, 48, 34));
    window.draw(door);

    drawLabel(window, font, label, {portal.area.position.x + 14.0f, portal.area.position.y + 14.0f});
}

void renderCampusTopDownMap(sf::RenderWindow& window, sf::Font& font, Player& player,
                            CampusPlace place) {
    static sf::Texture outdoorTiles;
    static bool outdoorTilesLoaded = loadTextureFromCandidates(outdoorTiles, "assets/tilesets/pixlab24_topdown_tileset.png");

    sf::RectangleShape bg({960.0f, 540.0f});
    bg.setFillColor(place == CampusPlace::Campus ? sf::Color(139, 180, 74) : sf::Color(55, 44, 34));
    window.draw(bg);

    if (place == CampusPlace::Campus) {
        sf::RectangleShape mainPath({116.0f, 540.0f});
        mainPath.setPosition({422.0f, 0.0f});
        mainPath.setFillColor(sf::Color(214, 190, 118));
        window.draw(mainPath);

        sf::RectangleShape crossPath({960.0f, 92.0f});
        crossPath.setPosition({0.0f, 224.0f});
        crossPath.setFillColor(sf::Color(214, 190, 118));
        window.draw(crossPath);

        sf::CircleShape plaza(76.0f);
        plaza.setPosition({404.0f, 194.0f});
        plaza.setFillColor(sf::Color(188, 178, 122));
        window.draw(plaza);

        for (int x = 0; x < 960; x += 32) {
            for (int y = 42; y < 540; y += 32) {
                const bool onRoad = (x > 410 && x < 550) || (y > 210 && y < 330);
                if (onRoad) {
                    sf::CircleShape pebble(1.4f);
                    pebble.setPosition({static_cast<float>(x + (y % 11)), static_cast<float>(y + (x % 7))});
                    pebble.setFillColor(sf::Color(166, 145, 92, 95));
                    window.draw(pebble);
                } else if ((x + y) % 96 == 0) {
                    if (outdoorTilesLoaded) {
                        drawPixlabSprite(window, outdoorTiles, sf::IntRect({370, 48}, {84, 76}),
                                         {static_cast<float>(x - 16), static_cast<float>(y - 26)}, 0.42f);
                    } else {
                        sf::CircleShape bush(10.0f);
                        bush.setPosition({static_cast<float>(x + 10), static_cast<float>(y + 12)});
                        bush.setFillColor(sf::Color(91, 145, 76));
                        window.draw(bush);
                    }
                }
            }
        }

        const auto portals = portalsForPlace(place);
        drawBuilding(window, font, portals[0], "Dorm", sf::Color(176, 112, 72), sf::Color(146, 74, 60));
        drawBuilding(window, font, portals[1], "Library", sf::Color(126, 136, 154), sf::Color(72, 88, 112));
        drawBuilding(window, font, portals[2], "Classroom", sf::Color(190, 164, 98), sf::Color(134, 86, 54));
        drawBuilding(window, font, portals[3], "Cafeteria", sf::Color(190, 132, 78), sf::Color(154, 78, 48));

        sf::CircleShape fountain(24.0f);
        fountain.setPosition({456.0f, 250.0f});
        fountain.setFillColor(sf::Color(82, 156, 176));
        fountain.setOutlineColor(sf::Color(224, 220, 178));
        fountain.setOutlineThickness(4.0f);
        window.draw(fountain);

        for (const sf::Vector2f benchPos : {sf::Vector2f{322.0f, 236.0f}, sf::Vector2f{584.0f, 236.0f},
                                            sf::Vector2f{322.0f, 300.0f}, sf::Vector2f{584.0f, 300.0f}}) {
            sf::RectangleShape bench({54.0f, 14.0f});
            bench.setPosition(benchPos);
            bench.setFillColor(sf::Color(132, 72, 42));
            window.draw(bench);
        }
    } else {
        const sf::Color floor = place == CampusPlace::Dormitory ? sf::Color(158, 110, 68)
            : place == CampusPlace::Library ? sf::Color(92, 78, 56)
            : place == CampusPlace::Classroom ? sf::Color(116, 126, 112)
            : sf::Color(150, 114, 72);
        bg.setFillColor(floor);
        window.draw(bg);

        sf::RectangleShape roomFrame({876.0f, 424.0f});
        roomFrame.setPosition({42.0f, 72.0f});
        roomFrame.setFillColor(sf::Color(0, 0, 0, 0));
        roomFrame.setOutlineColor(sf::Color(42, 30, 22));
        roomFrame.setOutlineThickness(12.0f);
        window.draw(roomFrame);

        for (int x = 54; x < 900; x += 42) {
            for (int y = 84; y < 486; y += 42) {
                sf::RectangleShape tile({40.0f, 40.0f});
                tile.setPosition({static_cast<float>(x), static_cast<float>(y)});
                tile.setFillColor(sf::Color(floor.r + 8, floor.g + 8, floor.b + 8, 70));
                window.draw(tile);
            }
        }

        if (place == CampusPlace::Dormitory) {
            sf::RectangleShape bed({180.0f, 70.0f});
            bed.setPosition({90.0f, 108.0f});
            bed.setFillColor(sf::Color(88, 148, 142));
            window.draw(bed);
            sf::RectangleShape pillow({44.0f, 58.0f});
            pillow.setPosition({98.0f, 114.0f});
            pillow.setFillColor(sf::Color(230, 218, 180));
            window.draw(pillow);
            sf::RectangleShape desk({210.0f, 66.0f});
            desk.setPosition({636.0f, 112.0f});
            desk.setFillColor(sf::Color(120, 78, 44));
            window.draw(desk);
            sf::RectangleShape rug({230.0f, 110.0f});
            rug.setPosition({365.0f, 285.0f});
            rug.setFillColor(sf::Color(72, 126, 116));
            window.draw(rug);
        } else if (place == CampusPlace::Library) {
            for (int i = 0; i < 4; ++i) {
                sf::RectangleShape shelf({92.0f, 330.0f});
                shelf.setPosition({72.0f + i * 220.0f, 92.0f});
                shelf.setFillColor(sf::Color(82, 58, 36));
                window.draw(shelf);
            }
            sf::RectangleShape table({180.0f, 64.0f});
            table.setPosition({390.0f, 260.0f});
            table.setFillColor(sf::Color(132, 92, 52));
            window.draw(table);
            for (int i = 0; i < 6; ++i) {
                sf::RectangleShape lamp({12.0f, 22.0f});
                lamp.setPosition({226.0f + i * 100.0f, 154.0f + (i % 2) * 180.0f});
                lamp.setFillColor(sf::Color(232, 202, 108));
                window.draw(lamp);
            }
        } else if (place == CampusPlace::Classroom) {
            sf::RectangleShape board({520.0f, 58.0f});
            board.setPosition({220.0f, 82.0f});
            board.setFillColor(sf::Color(34, 78, 68));
            window.draw(board);
            for (int row = 0; row < 3; ++row) {
                for (int col = 0; col < 5; ++col) {
                    sf::RectangleShape desk({72.0f, 34.0f});
                    desk.setPosition({180.0f + col * 122.0f, 202.0f + row * 72.0f});
                    desk.setFillColor(sf::Color(156, 108, 58));
                    window.draw(desk);
                }
            }
        } else if (place == CampusPlace::Cafeteria) {
            sf::RectangleShape counter({760.0f, 70.0f});
            counter.setPosition({100.0f, 94.0f});
            counter.setFillColor(sf::Color(176, 104, 58));
            window.draw(counter);
            for (int i = 0; i < 7; ++i) {
                sf::CircleShape tray(12.0f);
                tray.setPosition({132.0f + i * 100.0f, 116.0f});
                tray.setFillColor(sf::Color(232, 184, 88));
                window.draw(tray);
            }
            for (int i = 0; i < 5; ++i) {
                sf::RectangleShape table({84.0f, 58.0f});
                table.setPosition({130.0f + i * 160.0f, 278.0f});
                table.setFillColor(sf::Color(120, 78, 44));
                window.draw(table);
            }
        }

        sf::RectangleShape exit({140.0f, 42.0f});
        exit.setPosition({410.0f, 482.0f});
        exit.setFillColor(sf::Color(38, 130, 100, 210));
        window.draw(exit);
        drawLabel(window, font, "Exit to Campus", {428.0f, 494.0f});
    }

    for (const auto& portal : portalsForPlace(place)) {
        sf::RectangleShape marker(portal.area.size);
        marker.setPosition(portal.area.position);
        marker.setFillColor(sf::Color(86, 255, 186, 45));
        marker.setOutlineColor(sf::Color(140, 255, 210, 160));
        marker.setOutlineThickness(2.0f);
        window.draw(marker);
    }

    player.render(window);

    sf::Text title(font, placeName(place), 22);
    title.setFillColor(sf::Color(245, 238, 205));
    title.setOutlineColor(sf::Color(20, 30, 30));
    title.setOutlineThickness(2.0f);
    title.setPosition({18.0f, 50.0f});
    window.draw(title);

    sf::Text hint(font, "WASD Move  |  Enter: enter highlighted area  |  0/6 Help", 13);
    hint.setFillColor(sf::Color(235, 235, 210));
    hint.setPosition({18.0f, 510.0f});
    window.draw(hint);
}

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// Entity 婕旂ず: 鎺㈢储鍦板浘 + SAN 闃堝€艰Е鍙戞晫浜哄嚭鐜?
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
void runEntityDemo(sf::RenderWindow& window, sf::Font& font,
                   Player& player, std::vector<std::unique_ptr<Enemy>>& activeEnemies,
                   const CombatResult& combatResult) {
    // --- 鍦板浘鑳屾櫙 ---
    sf::RectangleShape mapBg({960.0f, 540.0f});
    mapBg.setFillColor(sf::Color(30, 40, 30));
    window.draw(mapBg);

    // 鍦伴潰缃戞牸
    for (int x = 0; x < 960; x += 32) {
        for (int y = 0; y < 540; y += 32) {
            sf::RectangleShape tile({31.0f, 31.0f});
            tile.setPosition({static_cast<float>(x), static_cast<float>(y)});
            tile.setFillColor(sf::Color(40, 50, 40));
            window.draw(tile);
        }
    }

    // --- 鎺㈢储鐐逛綅鏍囪锛堜簨浠惰Е鍙戠偣锛?--
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

    // --- 娓叉煋娲昏穬鏁屼汉锛圫AN 浣庢椂鍑虹幇锛?--
    for (auto& e : activeEnemies) {
        e->render(window);
        sf::Text label(font, e->getName(), 11);
        label.setFillColor(sf::Color(255, 180, 80));
        auto pos = e->getPosition();
        label.setPosition({pos.x - 20.0f, pos.y - 18.0f});
        window.draw(label);

        // DC/ATK 淇℃伅
        std::ostringstream ss;
        ss << "DC:" << e->getDC() << " ATK:" << e->getAttackPower();
        sf::Text info(font, ss.str(), 9);
        info.setFillColor(sf::Color(200, 160, 100));
        info.setPosition({pos.x - 20.0f, pos.y + 10.0f});
        window.draw(info);
    }

    // --- 娓叉煋鐜╁ ---
    player.render(window);
    sf::Text pLabel(font, "You", 11);
    pLabel.setFillColor(sf::Color(100, 200, 255));
    auto ppos = player.getPosition();
    pLabel.setPosition({ppos.x - 8.0f, ppos.y + 10.0f});
    window.draw(pLabel);

    // --- 璇存槑鏂囧瓧锛堝彸涓嬭锛?--
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

    // --- 鎴樻枟缁撴灉瑕嗙洊 ---
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

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// 閫氱敤 Quest UI 娓叉煋锛堢敤浜?SimpleQuest / ExamQuest 婕旂ず锛?
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
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

    // 杩涘害淇℃伅
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

    // 褰撳墠娲昏穬浠诲姟
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

    // 浠诲姟閾惧垪琛?
    sf::Text chainTitle(font, "Quest Chain (triggered by threshold):", 16);
    chainTitle.setFillColor(sf::Color(200, 200, 200));
    chainTitle.setPosition({40.0f, 230.0f});
    window.draw(chainTitle);

    // 鎵嬪姩璇诲彇 JSON 灞曠ず鍘熷鏁版嵁
    using json = nlohmann::json;
    std::ifstream f("assets/config/quests.json");
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

    // 鎿嶄綔鎻愮ず
    sf::Text hint(font,
        "[Enter] Create next quest (factory method)  |  [S] Simulate trigger check\n"
        "[E] Simulate completing a random event (+1 count)  |  [C] Reset demo",
        13);
    hint.setFillColor(sf::Color(130, 130, 150));
    hint.setPosition({40.0f, 490.0f});
    window.draw(hint);
}

// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
// main
// 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
int main() {
    // 鈹€鈹€ 绐楀彛 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    sf::RenderWindow window(sf::VideoMode({960, 540}), "CampusLifeSimulator - Class Demo");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    // 鈹€鈹€ 瀛椾綋 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    sf::Font font;
    const std::vector<std::string> fontCandidates = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/PingFang.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    };
    bool fontOk = false;
    for (const auto& path : fontCandidates) {
        if (font.openFromFile(path)) { fontOk = true; break; }
    }
    if (!fontOk) {
        std::cerr << "ERROR: Failed to load any font file!" << std::endl;
    }

    // 鈹€鈹€ 鍒涘缓 Entity 瀵硅薄 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    TitleScreen titleScreen(font, "assets/ui/campus_title_bg.png");
    DifficultyPanel difficultyPanel(font);
    SceneBackground sceneBackground;
    SceneTransition sceneTransition;
    GameScreen screen = GameScreen::TITLE;
    Difficulty selectedDifficulty = Difficulty::Normal;
    bool difficultyApplied = false;

    Player player(480.0f, 280.0f);
    player.setName("Protagonist");
    CampusPlace currentPlace = CampusPlace::Campus;
    CampusPlace pendingPlace = CampusPlace::Campus;
    sf::Vector2f pendingSpawnPosition(480.0f, 276.0f);
    bool hasPendingMapTransition = false;

    // 娲昏穬鏁屼汉鍒楄〃锛圫AN 浣庢椂鍔ㄦ€佺敓鎴愶紝涓嶅湪鎺㈢储鍦板浘涓婇鏀剧疆锛?
    std::vector<std::unique_ptr<Enemy>> activeEnemies;
    CombatResult combatResult;

    // 鈹€鈹€ Quest 瀵硅薄 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    QuestManager questManager;
    questManager.loadQuestChain("assets/config/quests.json");

    // 鐙珛 quest 瀵硅薄鐢ㄤ簬 Simple/Midterm/Final 婕旂ず椤甸潰
    MainQuest* currentQuest = nullptr;
    std::unique_ptr<SimpleQuest> simpleQuest;
    std::unique_ptr<MidtermExamQuest> midtermQuest;
    std::unique_ptr<FinalExamQuest> finalExamQuest;
    std::unique_ptr<MainQuest> questManagerQuest;

    // 鈹€鈹€ 鐘舵€?鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    DemoPage page = DemoPage::ENTITY;
    sf::Clock clock;
    bool keyWasPressed[static_cast<int>(sf::Keyboard::KeyCount)] = {};

    // Lambda: 妫€娴嬪崟娆℃寜閿紙闃叉杩炲彂锛?
    auto justPressed = [&](sf::Keyboard::Key k) -> bool {
        bool pressed = sf::Keyboard::isKeyPressed(k);
        bool prev = keyWasPressed[static_cast<int>(k)];
        keyWasPressed[static_cast<int>(k)] = pressed;
        return pressed && !prev;
    };

    // 鈹€鈹€ Entity Demo 鐘舵€?鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    int spawnCounter = 0;   // 姣忔寜 C 绱锛岀敤浜庡喅瀹氱敓鎴愬摢涓儏缁被鍨?

    // Lambda: 灏濊瘯鏍规嵁 SAN 绛夌骇鐢熸垚鏁屼汉
    // 瑙勫垯: SAN<30 姣忔寜涓€娆?C 姒傜巼鐢熸垚锛堢瓑绾ц秺楂樻鐜囪秺澶э級锛屾渶澶?3 涓椿璺?
    auto trySpawnEnemy = [&]() {
        int lvl = player.getSanLevel();
        if (lvl == 0) return;  // SAN >= 30, 鏃犳晫浜?

        // 鏈€澶ф椿璺冩晫浜烘暟闅?SAN 绛夌骇閫掑
        int maxEnemies = (lvl == 1) ? 1 : (lvl == 2) ? 2 : 3;
        if (static_cast<int>(activeEnemies.size()) >= maxEnemies) return;

        // 姒傜巼: lvl=1: 40%, lvl=2: 60%, lvl=3: 90%
        int chance = lvl == 1 ? 40 : (lvl == 2 ? 60 : 90);
        if ((std::rand() % 100) >= chance) return;

        // 闅忔満鎯呯华绫诲瀷
        EmotionType types[] = {
            EmotionType::ANXIETY, EmotionType::DEPRESSION, EmotionType::ANGER,
            EmotionType::FEAR, EmotionType::LONELINESS
        };
        EmotionType type = types[spawnCounter % 5];
        spawnCounter++;

        // 鍦ㄧ帺瀹堕檮杩戦殢鏈轰綅缃敓鎴?
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

    // Lambda: 涓庨檮杩戞晫浜烘垬鏂?
    auto fightNearestEnemy = [&]() -> bool {
        if (activeEnemies.empty() || combatResult.active) return false;

        // 鎵炬渶杩戠殑鏁屼汉锛堣窛绂?< 60px 鍐咃級
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

        // 璁＄畻妫€瀹?
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

        // 鎴樻枟鍚庢晫浜烘秷澶?
        activeEnemies.erase(activeEnemies.begin() + nearestIdx);
        return true;
    };

    // Lambda: 閲嶇疆婕旂ず quest
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
            player.setPosition(pendingSpawnPosition.x, pendingSpawnPosition.y);
            player.stopMovement();
            hasPendingMapTransition = false;
        }
        sceneTransition.skip();
    };

    // 鍒濆鍖?
    resetSimpleDemo();

    // 鈹€鈹€ 涓诲惊鐜?鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // 鏇存柊鎴樻枟缁撴灉璁℃椂鍣?
        combatResult.update(dt);
        sceneBackground.update(dt);
        sceneTransition.update(dt);

        // 鈹€鈹€ 浜嬩欢澶勭悊 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
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
                    player.setPosition(480.0f, 276.0f);
                    player.stopMovement();
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

            // 椤甸潰鍒囨崲
            if (const auto* keyEv = event.getIf<sf::Event::KeyPressed>()) {
                auto code = keyEv->code;
                if (code == sf::Keyboard::Key::Num1) {
                    page = DemoPage::ENTITY;
                    currentQuest = nullptr;
                    currentPlace = CampusPlace::Campus;
                    player.setPosition(480.0f, 276.0f);
                    player.stopMovement();
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

            // Quest 妯″紡杈撳叆
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

            // QuestManager 椤甸潰浜嬩欢锛堜粎鍦ㄦ棤娲昏穬 quest 鏃惰Е鍙戯級
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
                        questManager.loadQuestChain("assets/config/quests.json");
                        currentQuest = nullptr;
                        questManagerQuest.reset();
                        std::cout << "[QuestManager] Reset complete" << std::endl;
                    }
                }
            }
        }

        // 鈹€鈹€ 鎸佺画鎬ц緭鍏?绉诲姩) 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
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

        if (page == DemoPage::ENTITY) {
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

            // 鎸夐敭 C = 鍘嬪姏浜嬩欢锛堥檷浣?SAN锛岃Е鍙戞晫浜哄嚭鐜帮級
            if (justPressed(sf::Keyboard::Key::C)) {
                player.modifyAttributes(Attributes(-15, 0, 0, 0, 0));
                int lvl = player.getSanLevel();
                std::cout << "[Stress] SAN dropped to " << player.getAttributes().san
                          << " Level=" << lvl << std::endl;
                // 灏濊瘯鐢熸垚鏁屼汉锛圫AN 瓒婁綆姒傜巼瓒婂ぇ锛?
                trySpawnEnemy();
            }

            // 鎸夐敭 F = 涓庨檮杩戞晫浜烘垬鏂?
            if (justPressed(sf::Keyboard::Key::F)) {
                if (!fightNearestEnemy()) {
                    std::cout << "[Combat] No enemy nearby! Get closer or spawn one first (press C)." << std::endl;
                }
            }

            // 鎸夐敭 V = 鎭㈠ SAN锛堟ā鎷熶紤鎭?鑷垜鍏虫€€锛?
            if (justPressed(sf::Keyboard::Key::V)) {
                player.modifyAttributes(Attributes(15, 0, 0, 0, 0));
                int lvl = player.getSanLevel();
                // 鎭㈠鍚庨噸鏂扮缉鏀惧凡鐢熸垚鐨勬晫浜?
                for (auto& e : activeEnemies) {
                    e->scaleWithSanLevel(lvl);
                }
                std::cout << "[Rest] SAN restored to " << player.getAttributes().san
                          << " Level=" << lvl << std::endl;
                // SAN 鎭㈠鍚庨儴鍒嗘晫浜烘秷澶?
                if (lvl == 0) {
                    activeEnemies.clear();
                    std::cout << "[Combat] All enemies retreated (SAN >= 30)" << std::endl;
                } else if (lvl == 1 && static_cast<int>(activeEnemies.size()) > 1) {
                    activeEnemies.resize(1);
                    std::cout << "[Combat] Enemies reduced to 1 (SAN 20-29)" << std::endl;
                }
            }

            // 鎸夐敭 X = 璁剧疆鎴樻枟 buff
            if (justPressed(sf::Keyboard::Key::X)) {
                player.getCombatBuffs().nextEventPositive = true;
                player.getCombatBuffs().nextRollModifier = 2;
                std::cout << "[Buff] Victory buff set: +2 to rolls" << std::endl;
            }

            if (justPressed(sf::Keyboard::Key::Enter)) {
                for (const auto& portal : portalsForPlace(currentPlace)) {
                    if (pointInRect(player.getPosition(), portal.area)) {
                        startMapTransition(portal);
                        break;
                    }
                }
            }

            player.update(dt);
        }

        // 鈹€鈹€ 娓叉煋 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€
        window.clear(sf::Color(20, 20, 30));

        switch (page) {
            case DemoPage::ENTITY:
                renderCampusTopDownMap(window, font, player, currentPlace);
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

        // 椤堕儴灞炴€ч潰鏉匡紙鎵€鏈夐〉闈㈤€氱敤锛?
        if (fontOk) {
            renderStatsPanel(window, font, player, page);
        }

        window.display();
    }

    return 0;
}

