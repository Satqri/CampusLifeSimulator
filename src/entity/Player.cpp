#include "entity/Player.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kDefaultMoveSpeed = 210.0f;
constexpr float kDefaultAcceleration = 900.0f;
constexpr float kDefaultDeceleration = 1200.0f;
constexpr float kDefaultStopThreshold = 8.0f;

float vectorLength(const sf::Vector2f& value) {
    return std::sqrt(value.x * value.x + value.y * value.y);
}
}

Player::Player()
    : Character()
    , velocity(0.0f, 0.0f)
    , acceleration(kDefaultAcceleration)
    , deceleration(kDefaultDeceleration)
    , stopSpeedThreshold(kDefaultStopThreshold)
{
    setMoveSpeed(kDefaultMoveSpeed);
    sprite.setSize(sf::Vector2f(16.0f, 16.0f));
    sprite.setFillColor(sf::Color(100, 200, 255)); // 浅蓝色代表玩家
    sprite.setOrigin({8.0f, 8.0f});
}

Player::Player(float x, float y)
    : Character(x, y, Attributes(), kDefaultMoveSpeed)
    , velocity(0.0f, 0.0f)
    , acceleration(kDefaultAcceleration)
    , deceleration(kDefaultDeceleration)
    , stopSpeedThreshold(kDefaultStopThreshold)
{
    sprite.setSize(sf::Vector2f(16.0f, 16.0f));
    sprite.setFillColor(sf::Color(100, 200, 255));
    sprite.setOrigin({8.0f, 8.0f});
}

void Player::update(float deltaTime) {
    (void)deltaTime;
}

void Player::render(sf::RenderWindow& window) {
    if (!visible) return;
    sprite.setPosition({posX, posY});
    window.draw(sprite);
}

void Player::move(float directionX, float directionY, float deltaTime) {
    if (deltaTime <= 0.0f) return;

    sf::Vector2f inputDirection(directionX, directionY);
    sf::Vector2f targetVelocity(0.0f, 0.0f);
    float inputLength = vectorLength(inputDirection);

    if (inputLength > 0.0f) {
        if (inputLength > 1.0f) {
            inputDirection.x /= inputLength;
            inputDirection.y /= inputLength;
        }
        targetVelocity = {inputDirection.x * moveSpeed, inputDirection.y * moveSpeed};

        float accelStep = acceleration * deltaTime;
        sf::Vector2f velocityDelta(targetVelocity.x - velocity.x, targetVelocity.y - velocity.y);
        float velocityDeltaLength = vectorLength(velocityDelta);
        if (velocityDeltaLength <= accelStep || velocityDeltaLength == 0.0f) {
            velocity = targetVelocity;
        } else {
            velocity.x += velocityDelta.x / velocityDeltaLength * accelStep;
            velocity.y += velocityDelta.y / velocityDeltaLength * accelStep;
        }
    } else {
        float currentSpeed = vectorLength(velocity);
        if (currentSpeed > 0.0f) {
            float decelStep = deceleration * deltaTime;
            float nextSpeed = std::max(0.0f, currentSpeed - decelStep);
            if (nextSpeed <= stopSpeedThreshold) {
                velocity = {0.0f, 0.0f};
            } else {
                float scale = nextSpeed / currentSpeed;
                velocity.x *= scale;
                velocity.y *= scale;
            }
        }
    }

    float currentSpeed = vectorLength(velocity);
    if (currentSpeed > moveSpeed && currentSpeed > 0.0f) {
        float scale = moveSpeed / currentSpeed;
        velocity.x *= scale;
        velocity.y *= scale;
    }

    posX += velocity.x * deltaTime;
    posY += velocity.y * deltaTime;
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

void Player::stopMovement() {
    velocity = {0.0f, 0.0f};
}

// ── Stamina & Canteen ──────────────────────────────────────────────

void Player::restoreStamina(int amount) {
    attributes.energy = std::min(100, attributes.energy + amount);
}

void Player::consumeStamina(int amount) {
    attributes.energy = std::max(0, attributes.energy - amount);
}

bool Player::canVisitCanteen() const {
    return canteenVisitsToday < kCanteenLimit;
}

int Player::getCanteenVisitsToday() const {
    return canteenVisitsToday;
}

int Player::getCanteenLimit() const {
    return kCanteenLimit;
}

void Player::useCanteenVisit() {
    if (canteenVisitsToday < kCanteenLimit) canteenVisitsToday++;
}

void Player::resetCanteenVisits() {
    canteenVisitsToday = 0;
}

void Player::advanceToNextDay() {
    attributes.energy = 100;
    canteenVisitsToday = 0;
}
