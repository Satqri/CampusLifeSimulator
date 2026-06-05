#include "entity/Player.h"

Player::Player()
    : Character()
    , moveCooldown(0.15f)
    , moveCooldownTimer(0.0f)
{
    setMoveSpeed(210.0f);
    sprite.setSize(sf::Vector2f(16.0f, 16.0f));
    sprite.setFillColor(sf::Color(100, 200, 255)); // 浅蓝色代表玩家
    sprite.setOrigin({8.0f, 8.0f});
}

Player::Player(float x, float y)
    : Character(x, y, Attributes(), 210.0f)
    , moveCooldown(0.15f)
    , moveCooldownTimer(0.0f)
{
    sprite.setSize(sf::Vector2f(16.0f, 16.0f));
    sprite.setFillColor(sf::Color(100, 200, 255));
    sprite.setOrigin({8.0f, 8.0f});
}

void Player::update(float deltaTime) {
    if (moveCooldownTimer > 0.0f) {
        moveCooldownTimer -= deltaTime;
    }
}

void Player::render(sf::RenderWindow& window) {
    if (!visible) return;
    sprite.setPosition({posX, posY});
    window.draw(sprite);
}

void Player::move(float directionX, float directionY, float deltaTime) {
    if (moveCooldownTimer > 0.0f) return;
    if (directionX == 0.0f && directionY == 0.0f) return;

    float distance = moveSpeed * deltaTime;
    posX += directionX * distance;
    posY += directionY * distance;

    moveCooldownTimer = 0.035f;
}

void Player::modifyAttributes(const Attributes& delta) {
    attributes.san      += delta.san;
    attributes.energy   += delta.energy;
    attributes.academic += delta.academic;
    attributes.social   += delta.social;
    attributes.gold     += delta.gold;
    clampAttributes();
}

bool Player::isSanCritical() const {
    return attributes.san < 30;
}

bool Player::isSanDangerous() const {
    return attributes.san < 10;
}

int Player::getSanLevel() const {
    if (attributes.san >= 30) return 0;
    if (attributes.san >= 20) return 1;
    if (attributes.san >= 10) return 2;
    return 3;
}

CombatBuffs& Player::getCombatBuffs() {
    return combatBuffs;
}

const CombatBuffs& Player::getCombatBuffs() const {
    return combatBuffs;
}

void Player::clearBuffs() {
    combatBuffs.clear();
}
