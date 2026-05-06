#include "entity/Enemy.h"

Enemy::Enemy()
    : Character()
    , emotionType(EmotionType::ANXIETY)
    , baseDC(12)
    , baseAttack(5)
    , scaledDC(12)
    , scaledAttack(5)
{
    setMoveSpeed(0.0f);
    sprite.setSize(sf::Vector2f(16.0f, 16.0f));
    sprite.setOrigin({8.0f, 8.0f});
    sprite.setFillColor(colorForEmotion(emotionType));
    setName(nameForEmotion(emotionType));
}

Enemy::Enemy(float x, float y, EmotionType type, int baseDC, int baseAttack)
    : Character(x, y, Attributes(50, 50, 50, 50, 0), 0.0f)
    , emotionType(type)
    , baseDC(baseDC)
    , baseAttack(baseAttack)
    , scaledDC(baseDC)
    , scaledAttack(baseAttack)
{
    sprite.setSize(sf::Vector2f(16.0f, 16.0f));
    sprite.setOrigin({8.0f, 8.0f});
    sprite.setFillColor(colorForEmotion(emotionType));
    setName(nameForEmotion(emotionType));
}

void Enemy::move(float /*directionX*/, float /*directionY*/, float /*deltaTime*/) {
    // 敌人不在地图上自由移动，战斗中由 CombatSystem 控制位置
}

void Enemy::update(float /*deltaTime*/) {
    // 敌人逻辑由 CombatSystem 驱动，此处无需每帧更新
}

void Enemy::render(sf::RenderWindow& window) {
    if (!visible) return;
    sprite.setPosition({posX, posY});
    window.draw(sprite);
}

void Enemy::scaleWithSanLevel(int sanLevel) {
    switch (sanLevel) {
        case 1: // SAN 20-29: 轻度
            scaledDC = baseDC + 2;
            scaledAttack = baseAttack;
            break;
        case 2: // SAN 10-19: 中度
            scaledDC = baseDC + 4;
            scaledAttack = baseAttack + 3;
            break;
        case 3: // SAN <10: 危险
            scaledDC = baseDC + 6;
            scaledAttack = baseAttack + 5;
            break;
        default: // SAN >= 30: 正常
            scaledDC = baseDC;
            scaledAttack = baseAttack;
            break;
    }
}

int Enemy::getDC() const {
    return scaledDC;
}

int Enemy::getAttackPower() const {
    return scaledAttack;
}

EmotionType Enemy::getEmotionType() const {
    return emotionType;
}

void Enemy::setEmotionType(EmotionType type) {
    emotionType = type;
    sprite.setFillColor(colorForEmotion(type));
    setName(nameForEmotion(type));
}

int Enemy::getBaseDC() const {
    return baseDC;
}

int Enemy::getBaseAttack() const {
    return baseAttack;
}

sf::Color Enemy::colorForEmotion(EmotionType type) {
    switch (type) {
        case EmotionType::ANXIETY:    return sf::Color(255, 200, 50);  // 橙黄
        case EmotionType::DEPRESSION: return sf::Color(100, 100, 200); // 暗蓝
        case EmotionType::ANGER:      return sf::Color(220, 60, 60);   // 红色
        case EmotionType::FEAR:       return sf::Color(160, 80, 180);  // 紫色
        case EmotionType::LONELINESS: return sf::Color(120, 160, 200); // 灰蓝
    }
    return sf::Color::White;
}

const char* Enemy::nameForEmotion(EmotionType type) {
    switch (type) {
        case EmotionType::ANXIETY:    return "焦虑";
        case EmotionType::DEPRESSION: return "抑郁";
        case EmotionType::ANGER:      return "愤怒";
        case EmotionType::FEAR:       return "恐惧";
        case EmotionType::LONELINESS: return "孤独";
    }
    return "未知";
}
