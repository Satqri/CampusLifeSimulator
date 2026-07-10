#include "minigame/PlatformerController.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kStandingHeight = 56.0f;
constexpr float kDuckHeight = 36.0f;
constexpr float kPlayerWidth = 34.0f;
constexpr float kJumpBuffer = 0.12f;
constexpr float kCoyoteTime = 0.10f;
constexpr float kHitStunDuration = 0.86f;
constexpr float kHitKnockbackSpeed = 190.0f;
constexpr float kSlopeSlideSpeed = 68.0f;

const PlatformerSlope* supportingSlope(const PlatformerRect& box,
                                       const PlatformerCollisionWorld& world,
                                       float tolerance) {
    const float centerX = box.x + box.width * 0.5f;
    for (const auto& slope : world.getSlopes()) {
        if (centerX < slope.left() || centerX > slope.right()) continue;
        if (std::abs(box.bottom() - slope.surfaceY(centerX)) <= tolerance) return &slope;
    }
    return nullptr;
}
}

void PlatformerController::reset(float x, float y, bool sleepyState) {
    sleepy = sleepyState;
    box = {x, y, kPlayerWidth, kStandingHeight, "Player"};
    velocityX = 0.0f;
    velocityY = 0.0f;
    jumpVelocity = sleepy ? 455.0f : 520.0f;
    gravity = sleepy ? 1510.0f : 1450.0f;
    maxFallSpeed = 900.0f;
    speedMultiplier = sleepy ? 0.92f : 1.0f;
    grounded = false;
    ducking = false;
    wallContact = false;
    slopeSliding = false;
    airJumpsUsed = 0;
    lastAction = PlatformerAction::None;
    jumpBufferTimer = 0.0f;
    coyoteTimer = 0.0f;
    slideTimer = 0.0f;
    hitStunTimer = 0.0f;
    hitKnockbackX = 0.0f;
    dropThroughTimer = 0.0f;
}

void PlatformerController::applyHazardBounce(float hazardCenterX) {
    const float playerCenterX = box.x + box.width * 0.5f;
    const float direction = playerCenterX < hazardCenterX ? -1.0f : 1.0f;
    hitStunTimer = kHitStunDuration;
    hitKnockbackX = direction * kHitKnockbackSpeed;
    velocityY = -150.0f;
    grounded = false;
    slideTimer = 0.0f;
    slopeSliding = false;
    lastAction = PlatformerAction::Fall;
}

bool PlatformerController::burstForward(float distance,
                                        const PlatformerCollisionWorld& world) {
    const float direction = distance >= 0.0f ? 1.0f : -1.0f;
    float remaining = std::abs(distance);
    bool blocked = false;

    while (remaining > 0.0f) {
        float stepVelocity = std::min(10.0f, remaining) * direction;
        box.x += stepVelocity;
        if (world.resolveHorizontal(box, stepVelocity)) {
            blocked = true;
            break;
        }
        remaining -= std::abs(stepVelocity);
    }

    velocityX = 440.0f * direction;
    if (grounded) {
        lastAction = PlatformerAction::Run;
    }
    return !blocked;
}

void PlatformerController::setBodyHeight(float newHeight,
                                         const PlatformerCollisionWorld& world) {
    if (std::abs(box.height - newHeight) < 0.1f) return;

    if (newHeight < box.height) {
        box.y += box.height - newHeight;
        box.height = newHeight;
        return;
    }

    PlatformerRect probe = box;
    probe.y -= newHeight - box.height;
    probe.height = newHeight;
    if (!world.intersectsSolid(probe)) {
        box = probe;
    }
}

void PlatformerController::tryConsumeJump() {
    if (jumpBufferTimer <= 0.0f) return;

    if (grounded || coyoteTimer > 0.0f) {
        velocityY = -jumpVelocity;
        grounded = false;
        coyoteTimer = 0.0f;
        jumpBufferTimer = 0.0f;
        airJumpsUsed = 0;
        slideTimer = 0.0f;
        ducking = false;
        lastAction = PlatformerAction::Jump;
        return;
    }

    if (wallContact) {
        velocityY = -jumpVelocity * 0.76f;
        jumpBufferTimer = 0.0f;
        airJumpsUsed = 0;
        ducking = false;
        wallContact = false;
        lastAction = PlatformerAction::WallTouch;
        return;
    }

    if (airJumpsUsed < 1) {
        velocityY = -jumpVelocity * 0.82f;
        jumpBufferTimer = 0.0f;
        ++airJumpsUsed;
        ducking = false;
        lastAction = PlatformerAction::DoubleJump;
    }
}

void PlatformerController::update(float dt, const PlatformerInput& input,
                                  const PlatformerCollisionWorld& world,
                                  float autoSpeed) {
    if (input.jumpPressed) jumpBufferTimer = kJumpBuffer;
    jumpBufferTimer = std::max(0.0f, jumpBufferTimer - dt);

    if (grounded) {
        coyoteTimer = kCoyoteTime;
    } else {
        coyoteTimer = std::max(0.0f, coyoteTimer - dt);
    }

    const float neutral = sleepy ? 0.92f : 1.0f;
    if (speedMultiplier > neutral) {
        speedMultiplier = std::max(neutral, speedMultiplier - 2.2f * dt);
    } else if (speedMultiplier < neutral) {
        speedMultiplier = std::min(neutral, speedMultiplier + 2.2f * dt);
    }

    if (input.slidePressed && grounded && slideTimer <= 0.0f) {
        slideTimer = 0.42f;
        lastAction = PlatformerAction::Slide;
    }
    slideTimer = std::max(0.0f, slideTimer - dt);
    hitStunTimer = std::max(0.0f, hitStunTimer - dt);
    slopeSliding = false;

    const bool wantsShortBody = (input.downHeld && grounded) || slideTimer > 0.0f;
    ducking = wantsShortBody && slideTimer <= 0.0f;
    setBodyHeight(wantsShortBody ? kDuckHeight : kStandingHeight, world);

    dropThroughTimer = 0.0f;

    tryConsumeJump();

    if (!input.jumpHeld && velocityY < -220.0f) {
        velocityY = -220.0f;
    }

    const PlatformerSlope* slopeBeforeMove = grounded
        ? supportingSlope(box, world, 6.0f)
        : nullptr;
    if (hitStunTimer > 0.0f) {
        velocityX = hitKnockbackX;
    } else {
        int moveDirection = (input.moveRight ? 1 : 0) - (input.moveLeft ? 1 : 0);
        const bool crawlingForward = input.downHeld && grounded && moveDirection == 0;
        if (crawlingForward) {
            moveDirection = input.facingRight ? 1 : -1;
        }
        if (moveDirection == 0 && slopeBeforeMove != nullptr && !input.downHeld) {
            const float downhillDirection = slopeBeforeMove->risesRight ? -1.0f : 1.0f;
            velocityX = downhillDirection * kSlopeSlideSpeed;
            slopeSliding = true;
        } else {
            const float crawlMultiplier = input.downHeld && grounded ? 0.48f : 1.0f;
            velocityX = autoSpeed * speedMultiplier * crawlMultiplier * static_cast<float>(moveDirection);
            if (slideTimer > 0.0f) velocityX *= 1.16f;
        }
    }

    const float xBeforeMove = box.x;
    box.x += velocityX * dt;
    wallContact = world.resolveHorizontal(box, velocityX);
    const float horizontalDistance = std::abs(box.x - xBeforeMove);

    const float previousBottom = box.bottom();
    velocityY = std::min(maxFallSpeed, velocityY + gravity * dt);
    box.y += velocityY * dt;

    bool landed = false;
    world.resolveVertical(box, velocityY, previousBottom, false, landed);
    grounded = landed;

    if (velocityY >= 0.0f) {
        const float centerX = box.x + box.width * 0.5f;
        for (const auto& slope : world.getSlopes()) {
            if (centerX < slope.left() || centerX > slope.right()) continue;
            const float surface = slope.surfaceY(centerX);
            const bool crossedSurface = previousBottom <= surface + 4.0f
                && box.bottom() >= surface;
            const bool remainsSupported = std::abs(box.bottom() - surface) <= 8.0f;
            const float maximumUphillStep = horizontalDistance
                * slope.height / std::max(1.0f, slope.width) + 6.0f;
            const bool followsConnectedSlope = slopeBeforeMove != nullptr
                && surface <= box.bottom() + 8.0f
                && previousBottom - surface <= maximumUphillStep;
            if (!crossedSurface && !remainsSupported && !followsConnectedSlope) continue;
            box.y = surface - box.height;
            velocityY = 0.0f;
            grounded = true;
            break;
        }
    }

    if (!grounded && velocityY >= 0.0f) {
        PlatformerRect landedRect;
        if (world.snapToGround(box, 4.0f, false, landedRect)) {
            velocityY = 0.0f;
            grounded = true;
        }
    }

    if (grounded) {
        airJumpsUsed = 0;
        if (slideTimer > 0.0f) {
            lastAction = PlatformerAction::Slide;
        } else if (ducking) {
            lastAction = PlatformerAction::Duck;
        } else if (slopeSliding) {
            lastAction = PlatformerAction::None;
        } else if (std::abs(velocityX) > 2.0f) {
            lastAction = PlatformerAction::Run;
        } else {
            lastAction = PlatformerAction::None;
        }
    } else if (wallContact) {
        lastAction = PlatformerAction::WallTouch;
    } else if (lastAction != PlatformerAction::DoubleJump) {
        lastAction = velocityY < 0.0f ? PlatformerAction::Jump : PlatformerAction::Fall;
    }
}
