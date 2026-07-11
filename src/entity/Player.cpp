#include "entity/Player.h"
#include "utils/AssetPath.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace {
constexpr float kDefaultMoveSpeed = 210.0f;
constexpr float kDefaultAcceleration = 900.0f;
constexpr float kDefaultDeceleration = 1200.0f;
constexpr float kDefaultStopThreshold = 8.0f;
constexpr float kMoveTargetArrivalRadius = 10.0f;
constexpr float kCharacterSpriteHeight = 38.0f;
constexpr float kWalkFramesPerSecond = 8.0f;
constexpr float kWalkCyclePi = 6.28318530718f;

int frameIndex(int direction, bool walking) {
    return direction * 2 + (walking ? 1 : 0);
}

struct CharacterFrame {
    sf::Texture texture;
    sf::IntRect source;
    bool loaded = false;
};

float vectorLength(const sf::Vector2f& value) {
    return std::sqrt(value.x * value.x + value.y * value.y);
}

std::array<CharacterFrame, 8>& playerCharacterFrames() {
    static std::array<CharacterFrame, 8> frames;
    static bool initialized = false;
    if (initialized) return frames;
    initialized = true;

    const auto loadFrame = [&](int direction, bool walking, const std::string& path,
                               sf::IntRect source) {
        CharacterFrame& frame = frames[frameIndex(direction, walking)];
        frame.loaded = frame.texture.loadFromFile(cls::resolveAssetPath(path));
        if (frame.loaded) {
            frame.texture.setSmooth(false);
            frame.source = source;
        }
    };

    loadFrame(0, false, "assets/image/characters/player/male_front_idle.png",
              sf::IntRect({316, 40}, {412, 984}));
    loadFrame(0, true, "assets/image/characters/player/male_front_walk.png",
              sf::IntRect({40, 57}, {143, 316}));
    loadFrame(1, false, "assets/image/characters/player/male_back_idle.png",
              sf::IntRect({314, 81}, {396, 897}));
    loadFrame(1, true, "assets/image/characters/player/male_back_walk.png",
              sf::IntRect({46, 31}, {141, 317}));
    loadFrame(2, false, "assets/image/characters/player/male_left_idle.png",
              sf::IntRect({309, 65}, {382, 896}));
    loadFrame(2, true, "assets/image/characters/player/male_left_walk.png",
              sf::IntRect({49, 40}, {156, 316}));
    loadFrame(3, false, "assets/image/characters/player/male_right_idle.png",
              sf::IntRect({316, 64}, {397, 896}));
    loadFrame(3, true, "assets/image/characters/player/male_right_walk.png",
              sf::IntRect({37, 23}, {151, 316}));

    return frames;
}

const CharacterFrame* selectCharacterFrame(int direction, bool walking) {
    auto& frames = playerCharacterFrames();
    const CharacterFrame& preferred = frames[frameIndex(direction, walking)];
    if (preferred.loaded) return &preferred;

    const CharacterFrame& idle = frames[frameIndex(direction, false)];
    if (idle.loaded) return &idle;

    const CharacterFrame& frontIdle = frames[frameIndex(0, false)];
    return frontIdle.loaded ? &frontIdle : nullptr;
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
    if (deltaTime <= 0.0f) return;

    const float currentSpeed = vectorLength(velocity);
    if (currentSpeed > stopSpeedThreshold) {
        const float speedScale = moveSpeed > 0.0f
            ? std::clamp(currentSpeed / moveSpeed, 0.45f, 1.5f)
            : 1.0f;
        walkAnimationTime += deltaTime * speedScale;
    } else {
        walkAnimationTime = 0.0f;
    }
}

void Player::render(sf::RenderWindow& window) {
    if (!visible) return;
    const float currentSpeed = vectorLength(velocity);
    const bool walking = currentSpeed > stopSpeedThreshold;
    const int direction = static_cast<int>(facingDirection);
    const int stepFrame = walking
        ? static_cast<int>(walkAnimationTime * kWalkFramesPerSecond) % 4
        : 0;
    const bool useWalkPose = walking && (stepFrame == 1 || stepFrame == 3);
    if (const CharacterFrame* frame = selectCharacterFrame(direction, useWalkPose)) {
        const float phase = walkAnimationTime * kWalkFramesPerSecond * 0.25f * kWalkCyclePi;
        const float bodyBob = walking ? -std::abs(std::sin(phase)) * 1.4f : 0.0f;
        const float bodySway = walking ? std::sin(phase) * 0.45f : 0.0f;
        const bool mirrorStep = walking
            && stepFrame == 3
            && (facingDirection == FacingDirection::Front || facingDirection == FacingDirection::Back);

        sf::CircleShape shadow(1.0f, 24);
        shadow.setOrigin({1.0f, 1.0f});
        shadow.setScale({8.0f, 2.2f});
        shadow.setFillColor(sf::Color(0, 0, 0, walking ? 58 : 48));
        shadow.setPosition({posX, posY + kCharacterSpriteHeight * 0.43f});
        window.draw(shadow);

        sf::Sprite character(frame->texture);
        character.setTextureRect(frame->source);
        character.setOrigin({
            static_cast<float>(frame->source.size.x) * 0.5f,
            static_cast<float>(frame->source.size.y)
        });
        const float scale = kCharacterSpriteHeight / static_cast<float>(frame->source.size.y);
        character.setScale({mirrorStep ? -scale : scale, scale});
        character.setPosition({
            posX + bodySway,
            posY + kCharacterSpriteHeight * 0.5f + bodyBob
        });
        window.draw(character);
        return;
    }

    sprite.setPosition({posX, posY});
    window.draw(sprite);
}

void Player::move(float directionX, float directionY, float deltaTime) {
    if (deltaTime <= 0.0f) return;

    sf::Vector2f inputDirection(directionX, directionY);
    float inputLength = vectorLength(inputDirection);

    if (inputLength > 0.0f) {
        if (inputLength > 1.0f) {
            inputDirection.x /= inputLength;
            inputDirection.y /= inputLength;
        }
        if (std::abs(inputDirection.x) > std::abs(inputDirection.y)) {
            facingDirection = inputDirection.x < 0.0f
                ? FacingDirection::Left
                : FacingDirection::Right;
        } else {
            facingDirection = inputDirection.y < 0.0f
                ? FacingDirection::Back
                : FacingDirection::Front;
        }
        velocity = {inputDirection.x * moveSpeed, inputDirection.y * moveSpeed};
    } else {
        velocity = {0.0f, 0.0f};
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
    const float maxStep = moveSpeed * deltaTime;
    if (distance <= kMoveTargetArrivalRadius || distance <= maxStep) {
        posX = moveTarget.x;
        posY = moveTarget.y;
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
    walkAnimationTime = 0.0f;
    clearMoveTarget();
}
