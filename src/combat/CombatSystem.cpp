#include "combat/CombatSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

bool CombatSystem::trySpawnEnemy(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
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

int CombatSystem::statForEmotion(const Player& player, EmotionType type) {
    const auto& a = player.getAttributes();
    switch (type) {
        case EmotionType::ANXIETY:    return a.academic;
        case EmotionType::ANGER:      return a.energy;
        case EmotionType::LONELINESS: return a.social;
        case EmotionType::DEPRESSION:
        case EmotionType::FEAR:       return a.san;
    }
    return a.san;
}

const char* CombatSystem::actionNameForEmotion(EmotionType type) {
    switch (type) {
        case EmotionType::ANXIETY:    return "Rational Analysis";
        case EmotionType::DEPRESSION: return "Will Stand";
        case EmotionType::ANGER:      return "Vent / Release";
        case EmotionType::FEAR:       return "Will Stand";
        case EmotionType::LONELINESS: return "Confide / Seek Help";
    }
    return "Unknown";
}
