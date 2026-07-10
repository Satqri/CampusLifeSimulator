#ifndef CLS_MINIGAME_PLATFORMERCONTROLLER_H
#define CLS_MINIGAME_PLATFORMERCONTROLLER_H

#include "minigame/PlatformerCollisionWorld.h"

enum class PlatformerAction {
    None,
    Run,
    Jump,
    DoubleJump,
    Fall,
    Slide,
    Duck,
    WallTouch
};

struct PlatformerInput {
    bool jumpPressed = false;
    bool jumpHeld = false;
    bool downHeld = false;
    bool slidePressed = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool facingRight = true;
};

class PlatformerController {
public:
    void reset(float x, float y, bool sleepy);
    void update(float dt, const PlatformerInput& input,
                const PlatformerCollisionWorld& world, float autoSpeed);
    void applyHazardBounce(float hazardCenterX);
    bool burstForward(float distance, const PlatformerCollisionWorld& world);

    const PlatformerRect& getBox() const { return box; }
    float getVelocityX() const { return velocityX; }
    float getVelocityY() const { return velocityY; }
    bool isGrounded() const { return grounded; }
    bool isDucking() const { return ducking; }
    bool isSliding() const { return slideTimer > 0.0f; }
    bool isHitStunned() const { return hitStunTimer > 0.0f; }
    bool isSlopeSliding() const { return slopeSliding; }
    float getHitStunTimer() const { return hitStunTimer; }
    bool isWallContact() const { return wallContact; }
    int getAirJumpCount() const { return airJumpsUsed; }
    PlatformerAction getLastAction() const { return lastAction; }
    float getSpeedMultiplier() const { return speedMultiplier; }

private:
    void setBodyHeight(float newHeight, const PlatformerCollisionWorld& world);
    void tryConsumeJump();

    PlatformerRect box{96.0f, 364.0f, 34.0f, 56.0f, "Player"};
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float jumpVelocity = 520.0f;
    float gravity = 1450.0f;
    float maxFallSpeed = 900.0f;
    float speedMultiplier = 1.0f;

    bool sleepy = false;
    bool grounded = false;
    bool ducking = false;
    bool wallContact = false;
    bool slopeSliding = false;
    int airJumpsUsed = 0;
    PlatformerAction lastAction = PlatformerAction::None;

    float jumpBufferTimer = 0.0f;
    float coyoteTimer = 0.0f;
    float slideTimer = 0.0f;
    float hitStunTimer = 0.0f;
    float hitKnockbackX = 0.0f;
    float dropThroughTimer = 0.0f;
};

#endif
