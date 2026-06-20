#include "entity/Player.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kDefaultMoveSpeed = 210.0f;
constexpr float kDefaultAcceleration = 900.0f;
constexpr float kDefaultDeceleration = 1200.0f;
constexpr float kDefaultStopThreshold = 8.0f;
constexpr float kMoveTargetArrivalRadius = 10.0f;

float vectorLength(const sf::Vector2f& value) {
    return std::sqrt(value.x * value.x + value.y * value.y);
}

void applyVelocityTowardsTarget(sf::Vector2f& velocity, sf::Vector2f targetVelocity,
                                float acceleration, float deltaTime) {
    float accelStep = acceleration * deltaTime;
    sf::Vector2f velocityDelta(targetVelocity.x - velocity.x, targetVelocity.y - velocity.y);
    float velocityDeltaLength = vectorLength(velocityDelta);
    if (velocityDeltaLength <= accelStep || velocityDeltaLength == 0.0f) {
        velocity = targetVelocity;
    } else {
        velocity.x += velocityDelta.x / velocityDeltaLength * accelStep;
        velocity.y += velocityDelta.y / velocityDeltaLength * accelStep;
    }
}
}

Player::Player()
    : Character()
    , velocity(0.0f, 0.0f)
    , acceleration(kDefaultAcceleration)
    , deceleration(kDefaultDeceleration)
    , stopSpeedThreshold(kDefaultStopThreshold)
{
    attributes = defaultPlayerAttributes();
    mHidden["healthIndex"] = attributes.health;
    setMoveSpeed(kDefaultMoveSpeed);
    sprite.setSize(sf::Vector2f(16.0f, 16.0f));
    sprite.setFillColor(sf::Color(100, 200, 255)); // 浅蓝色代表玩家
    sprite.setOrigin({8.0f, 8.0f});
}

Player::Player(float x, float y)
    : Character(x, y, defaultPlayerAttributes(), kDefaultMoveSpeed)
    , velocity(0.0f, 0.0f)
    , acceleration(kDefaultAcceleration)
    , deceleration(kDefaultDeceleration)
    , stopSpeedThreshold(kDefaultStopThreshold)
{
    mHidden["healthIndex"] = attributes.health;
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
        applyVelocityTowardsTarget(velocity, targetVelocity, acceleration, deltaTime);
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

void Player::setMoveTarget(sf::Vector2f target) {
    moveTarget = target;
    moveTargetActive = true;
}

void Player::clearMoveTarget() {
    moveTargetActive = false;
}

void Player::moveToTarget(float deltaTime) {
    if (!moveTargetActive) {
        move(0.0f, 0.0f, deltaTime);
        return;
    }

    const sf::Vector2f currentPos(posX, posY);
    sf::Vector2f toTarget(moveTarget.x - currentPos.x, moveTarget.y - currentPos.y);
    const float distance = vectorLength(toTarget);
    if (distance <= kMoveTargetArrivalRadius) {
        clearMoveTarget();
        stopMovement();
        return;
    }

    if (distance > 0.0f) {
        toTarget.x /= distance;
        toTarget.y /= distance;
    }
    move(toTarget.x, toTarget.y, deltaTime);

    const sf::Vector2f newPos(posX, posY);
    const sf::Vector2f remaining(moveTarget.x - newPos.x, moveTarget.y - newPos.y);
    if (vectorLength(remaining) <= kMoveTargetArrivalRadius) {
        clearMoveTarget();
        stopMovement();
    }
}

void Player::modifyAttributes(const Attributes& delta) {
    attributes.energy   += delta.energy;
    if (delta.health != 0) {
        HiddenMap hiddenDelta = HiddenMap::object();
        hiddenDelta["healthIndex"] = delta.health;
        mergeHidden(mHidden, hiddenDelta);
    }
    attributes.gold     += delta.gold;
    attributes.san      += delta.san;
    attributes.academic += delta.academic;
    attributes.social   += delta.social;
    clampAttributes();
    syncVisibleHealthFromHidden(attributes, mHidden);
}

void Player::dailyAttributeCheck() {
    constexpr int kLowThreshold = 30;
    constexpr int kConsecutiveDays = 2;
    constexpr int kHealthPenalty = 8;
    constexpr int kSanPenalty = 8;
    constexpr int kAcademicPenalty = 5;
    constexpr int kSocialPenalty = 5;
    constexpr int kLateNightWarning = 20;
    constexpr int kLateNightDanger = 35;
    constexpr int kLateNightHealthPenalty = 6;
    constexpr int kLateNightSanPenalty = 6;
    constexpr int kLateNightRecovery = 4;

    // 长期低体力 → 扣健康
    if (attributes.energy < kLowThreshold) {
        lowEnergyDays++;
        mHidden["lowEnergyDays"] = lowEnergyDays;
        if (lowEnergyDays >= kConsecutiveDays) {
            HiddenMap hiddenDelta = HiddenMap::object();
            hiddenDelta["healthIndex"] = -kHealthPenalty;
            mergeHidden(mHidden, hiddenDelta);
        }
    } else {
        lowEnergyDays = 0;
        mHidden["lowEnergyDays"] = 0;
    }

    // 长期低健康 → 扣 SAN/知识/社交
    syncVisibleHealthFromHidden(attributes, mHidden);
    if (attributes.health < kLowThreshold) {
        lowHealthDays++;
        mHidden["lowHealthDays"] = lowHealthDays;
        if (lowHealthDays >= kConsecutiveDays) {
            attributes.san      -= kSanPenalty;
            attributes.academic -= kAcademicPenalty;
            attributes.social   -= kSocialPenalty;
        }
    } else {
        lowHealthDays = 0;
        mHidden["lowHealthDays"] = 0;
    }

    const int lateNightLevel = mHidden.value("lateNightLevel", 0);
    if (lateNightLevel >= kLateNightWarning) {
        HiddenMap hiddenDelta = HiddenMap::object();
        hiddenDelta["healthIndex"] = lateNightLevel >= kLateNightDanger
            ? -kLateNightHealthPenalty * 2
            : -kLateNightHealthPenalty;
        mergeHidden(mHidden, hiddenDelta);
        attributes.san -= lateNightLevel >= kLateNightDanger
            ? kLateNightSanPenalty * 2
            : kLateNightSanPenalty;
    }
    if (lateNightLevel > 0) {
        HiddenMap recoveryDelta = HiddenMap::object();
        recoveryDelta["lateNightLevel"] = -kLateNightRecovery;
        mergeHidden(mHidden, recoveryDelta);
    }

    clampAttributes();
    syncVisibleHealthFromHidden(attributes, mHidden);
}

void Player::syncDailyCountersFromHidden() {
    normalizeHidden(mHidden);
    lowEnergyDays = mHidden.value("lowEnergyDays", 0);
    lowHealthDays = mHidden.value("lowHealthDays", 0);
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
    clearMoveTarget();
}
