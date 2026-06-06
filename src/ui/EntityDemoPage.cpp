#include "ui/EntityDemoPage.h"
#include "core/CombatResult.h"
#include "entity/Player.h"
#include "entity/Enemy.h"
#include <sstream>

EntityDemoPage::EntityDemoPage(sf::Font& font) : mFont(font) {}

void EntityDemoPage::setPlayer(Player* player) { mPlayer = player; }
void EntityDemoPage::setActiveEnemies(const std::vector<std::unique_ptr<Enemy>>* enemies) { mActiveEnemies = enemies; }
void EntityDemoPage::setCombatResult(const CombatResult* result) { mCombatResult = result; }
void EntityDemoPage::update(float) {}

void EntityDemoPage::render(sf::RenderWindow& window) {
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
    sf::Text markerLabel(mFont, "Event Points (step on to trigger events in full game)", 10);
    markerLabel.setFillColor(sf::Color(100, 100, 140));
    markerLabel.setPosition({8.0f, 70.0f});
    window.draw(markerLabel);

    // --- 渲染活跃敌人（SAN 低时出现）---
    if (mActiveEnemies) {
        for (auto& e : *mActiveEnemies) {
            e->render(window);
            sf::Text label(mFont, e->getName(), 11);
            label.setFillColor(sf::Color(255, 180, 80));
            auto pos = e->getPosition();
            label.setPosition({pos.x - 20.0f, pos.y - 18.0f});
            window.draw(label);

            std::ostringstream ss;
            ss << "DC:" << e->getDC() << " ATK:" << e->getAttackPower();
            sf::Text info(mFont, ss.str(), 9);
            info.setFillColor(sf::Color(200, 160, 100));
            info.setPosition({pos.x - 20.0f, pos.y + 10.0f});
            window.draw(info);
        }
    }

    // --- 渲染玩家 ---
    if (mPlayer) {
        mPlayer->render(window);
        sf::Text pLabel(mFont, "You", 11);
        pLabel.setFillColor(sf::Color(100, 200, 255));
        auto ppos = mPlayer->getPosition();
        pLabel.setPosition({ppos.x - 8.0f, ppos.y + 10.0f});
        window.draw(pLabel);
    }

    // --- 说明文字（右下角）---
    sf::RectangleShape legendBg({420.0f, 88.0f});
    legendBg.setPosition({530.0f, 444.0f});
    legendBg.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(legendBg);

    sf::Text legend(mFont,
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
    if (mCombatResult && mCombatResult->active) {
        sf::RectangleShape overlay({400.0f, 130.0f});
        overlay.setPosition({280.0f, 200.0f});
        overlay.setFillColor(sf::Color(20, 20, 40, 230));
        overlay.setOutlineColor(mCombatResult->victory ? sf::Color(100, 200, 100) : sf::Color(200, 100, 100));
        overlay.setOutlineThickness(2.0f);
        window.draw(overlay);

        std::ostringstream css;
        css << (mCombatResult->victory ? "VICTORY!" : "DEFEAT!")
            << "  vs " << mCombatResult->enemyName << "\n\n"
            << "  D20: " << mCombatResult->d20Roll
            << " + MOD: " << (mCombatResult->modifier >= 0 ? "+" : "") << mCombatResult->modifier
            << " = " << mCombatResult->total
            << " vs DC " << mCombatResult->dc << "\n\n"
            << (mCombatResult->victory ? "  SAN restored! Buff gained!" : "  SAN -15! Debuff applied!");
        sf::Text resultText(mFont, css.str(), 14);
        resultText.setFillColor(mCombatResult->victory ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
        resultText.setPosition({300.0f, 215.0f});
        window.draw(resultText);
    }
}
