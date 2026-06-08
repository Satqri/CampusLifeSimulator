#include "core/CombatSystem.h"
#include "core/GameContext.h"
#include "entity/CombatHelper.h"
#include <cmath>
#include <iostream>

namespace CombatSystem {

void trySpawnEnemy(GameContext& ctx) {
    int lvl = ctx.player.getSanLevel();
    if (lvl == 0) return;

    int maxEnemies = (lvl == 1) ? 1 : (lvl == 2) ? 2 : 3;
    if (static_cast<int>(ctx.activeEnemies.size()) >= maxEnemies) return;

    int chance = lvl == 1 ? 40 : (lvl == 2 ? 60 : 90);
    if ((std::rand() % 100) >= chance) return;

    EmotionType types[] = {
        EmotionType::ANXIETY, EmotionType::DEPRESSION, EmotionType::ANGER,
        EmotionType::FEAR, EmotionType::LONELINESS
    };
    EmotionType type = types[ctx.spawnCounter % 5];
    ctx.spawnCounter++;

    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
    float dist = 80.0f + (std::rand() % 60);
    float ox = ctx.player.getPosition().x + std::cos(angle) * dist;
    float oy = ctx.player.getPosition().y + std::sin(angle) * dist;

    auto enemy = std::make_unique<Enemy>(ox, oy, type, 12, 5);
    enemy->scaleWithSanLevel(lvl);
    ctx.activeEnemies.push_back(std::move(enemy));
}

bool fightNearestEnemy(GameContext& ctx) {
    if (ctx.activeEnemies.empty() || ctx.combatResult.active) return false;

    float nearestDist = 100.0f;
    int nearestIdx = -1;
    const sf::Vector2f ppos = ctx.player.getPosition();

    for (int i = 0; i < static_cast<int>(ctx.activeEnemies.size()); ++i) {
        sf::Vector2f epos = ctx.activeEnemies[i]->getPosition();
        float dx = ppos.x - epos.x;
        float dy = ppos.y - epos.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < nearestDist) {
            nearestDist = dist;
            nearestIdx = i;
        }
    }

    if (nearestIdx < 0) return false;

    auto& enemy = ctx.activeEnemies[nearestIdx];
    EmotionType etype = enemy->getEmotionType();

    int d20 = (std::rand() % 20) + 1;
    int stat = statForEmotion(ctx.player, etype);
    int mod = (stat - 50) / 10;
    if (ctx.player.getCombatBuffs().nextEventPositive)
        mod += ctx.player.getCombatBuffs().nextRollModifier;
    int total = d20 + mod;
    int dc = enemy->getDC();
    bool win = total >= dc;

    std::cout << "[Combat] " << actionNameForEmotion(etype)
              << " | D20=" << d20 << " + MOD=" << mod
              << " = " << total << " vs DC=" << dc
              << " → " << (win ? "WIN" : "LOSE") << std::endl;

    ctx.combatResult.show(win, enemy->getName(), d20, mod, total, dc);

    if (win) {
        ctx.player.modifyAttributes(Attributes(10, 0, 0, 0, 0));
        ctx.player.getCombatBuffs().nextEventPositive = true;
        ctx.player.getCombatBuffs().nextRollModifier = 2;
    } else {
        ctx.player.modifyAttributes(Attributes(-15, 0, 0, 0, 0));
        ctx.player.getCombatBuffs().nextEventPositive = false;
        ctx.player.getCombatBuffs().nextRollModifier = 0;
    }

    ctx.activeEnemies.erase(ctx.activeEnemies.begin() + nearestIdx);
    return true;
}

} // namespace CombatSystem

// ── 显式参数版本（moyoulingsen，Game 类 / 状态机使用）────────

bool CombatSystem::trySpawnEnemy(Player& player,
                                 std::vector<std::unique_ptr<Enemy>>& enemies,
                                 int& spawnCounter) {
    const int level = player.getSanLevel();
    if (level == 0) return false;

    const int maxEnemies = (level == 1) ? 1 : (level == 2) ? 2 : 3;
    if (static_cast<int>(enemies.size()) >= maxEnemies) return false;

    const int chance = level == 1 ? 40 : (level == 2 ? 60 : 90);
    if ((std::rand() % 100) >= chance) return false;

    EmotionType types[] = {
        EmotionType::ANXIETY, EmotionType::DEPRESSION, EmotionType::ANGER,
        EmotionType::FEAR, EmotionType::LONELINESS
    };
    const EmotionType type = types[spawnCounter % 5];
    ++spawnCounter;

    float ox = player.getPosition().x + static_cast<float>((std::rand() % 160) - 80);
    float oy = player.getPosition().y + static_cast<float>((std::rand() % 160) - 80);
    ox = std::clamp(ox, 40.0f, 920.0f);
    oy = std::clamp(oy, 80.0f, 500.0f);

    auto enemy = std::make_unique<Enemy>(ox, oy, type, 12, 5);
    enemy->scaleWithSanLevel(level);
    enemies.push_back(std::move(enemy));
    return true;
}

std::optional<std::size_t> CombatSystem::findNearestEnemy(
    const Player& player,
    const std::vector<std::unique_ptr<Enemy>>& enemies,
    float maxDistance) {
    if (enemies.empty()) return std::nullopt;

    float minDist = maxDistance;
    int nearestIdx = -1;
    const sf::Vector2f pp = player.getPosition();
    for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
        const sf::Vector2f ep = enemies[i]->getPosition();
        const float dx = pp.x - ep.x;
        const float dy = pp.y - ep.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < minDist) {
            minDist = dist;
            nearestIdx = i;
        }
    }

    if (nearestIdx < 0) return std::nullopt;
    return static_cast<std::size_t>(nearestIdx);
}

CombatRollResult CombatSystem::resolveRoll(Player& player, const Enemy& enemy) {
    const int modifier = (statForEmotion(player, enemy.getEmotionType()) - 50) / 10
        + player.getCombatBuffs().nextRollModifier;
    const int d20 = (std::rand() % 20) + 1;
    const int total = d20 + modifier;
    const int dc = enemy.getDC();

    CombatRollResult result;
    result.victory = total >= dc;
    result.d20Roll = d20;
    result.modifier = modifier;
    result.total = total;
    result.dc = dc;
    return result;
}
