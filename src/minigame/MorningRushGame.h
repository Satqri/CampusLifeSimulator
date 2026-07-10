#ifndef CLS_MINIGAME_MORNINGRUSHGAME_H
#define CLS_MINIGAME_MORNINGRUSHGAME_H

#include "entity/Player.h"
#include "minigame/MorningRushLevel.h"
#include "minigame/PlatformerController.h"
#include <SFML/Window/Event.hpp>
#include <string>
#include <vector>

enum class MorningRushPhase { Intro, Running, FinalResult };
enum class MorningRushOutcome { None, GreatSuccess, Success, Failure, CriticalFailure };
enum class RushAction { None, Jump, HurdleJump, Squeeze, BurstRun, WallTurn, DoubleJump };
enum class TerrainZone { Dormitory, Campus, Classroom };
enum class MorningRushRoute { Dormitory, Cafeteria, Library, Campus, Rainy };
enum class PickupKind { Bicycle };
enum class MorningRushObstacleKind { Ground, Overhead };
enum class MorningRushObstacleRule { Hurdle, Squeeze, WallTurn, BurstRun, Clear };

struct MorningRushObstacle {
    std::string name;
    MorningRushObstacleKind kind = MorningRushObstacleKind::Ground;
    MorningRushObstacleRule rule = MorningRushObstacleRule::Clear;
    float x = 0.0f;
    float width = 40.0f;
    float height = 40.0f;
    float yOffset = 0.0f;
    int penaltySeconds = 3;
    bool hit = false;
    bool destructible = false;
    bool moving = false;
    float moveRange = 0.0f;
    float moveSpeed = 0.0f;
    float moveBase = 0.0f;
    bool standableTop = false;
};

struct RushPlatform {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 18.0f;
    bool destructible = false;
    std::string name;
};

struct RushPickup {
    PickupKind kind = PickupKind::Bicycle;
    float x = 0.0f;
    float y = 356.0f;
    float width = 104.0f;
    float height = 64.0f;
    bool collected = false;
};

struct MorningRushLuckEvent {
    std::string title;
    std::string text;
    int d20 = 0;
    int dc = 0;
    bool success = false;
    float displayTimer = 0.0f;
};

class MorningRushGame {
public:
    void start(int day, bool sleptShort, bool breakfastRisk, int checkPenalty,
               MorningRushRoute route = MorningRushRoute::Campus,
               bool practice = false);
    void update(float deltaTime, Player& player, int& teacherTrust);
    bool handleInput(const sf::Event& event, Player& player, int& teacherTrust);
    bool consumeCompleted();

    bool isActive() const { return active; }
    MorningRushPhase getPhase() const { return phase; }
    MorningRushOutcome getOutcome() const { return outcome; }
    int getDayStarted() const { return dayStarted; }
    bool wasLate() const;
    bool wasCriticalFailure() const { return outcome == MorningRushOutcome::CriticalFailure; }
    const std::string& getResultText() const { return resultText; }
    float getPulse() const { return pulse; }

    float getDistance() const { return distance; }
    float getTargetDistance() const { return targetDistance; }
    float getTimeLeft() const { return timeLeft; }
    float getRunnerLift() const;
    float getRunnerScreenX() const;
    float getRunnerTopY() const { return controller.getBox().y; }
    float getRunnerHeight() const { return controller.getBox().height; }
    float getRunSpeed() const { return controller.getVelocityX(); }
    bool isGrounded() const { return controller.isGrounded(); }
    bool isDucking() const { return controller.isDucking(); }
    bool isSliding() const { return controller.isSliding(); }
    bool isHitStunned() const { return controller.isHitStunned(); }
    bool isSlopeSliding() const { return controller.isSlopeSliding(); }
    float getHitStunTimer() const { return controller.getHitStunTimer(); }
    bool isBurstRunning() const { return burstRunning; }
    bool isFacingRight() const { return facingRight; }
    int getAirJumpCount() const { return controller.getAirJumpCount(); }
    RushAction getLastAction() const;
    RushAction getActionCue() const { return actionCue; }
    float getActionCueTimer() const { return actionCueTimer; }
    bool isWallContact() const { return controller.isWallContact(); }
    int getCollisionCount() const { return collisionCount; }
    float getInvulnerableTimer() const { return invulnerableTimer; }

    TerrainZone getCurrentZone() const;
    MorningRushRoute getRoute() const { return route; }
    int getStageIndex() const { return stageIndex; }
    bool isPracticeMode() const { return practiceMode; }
    const std::vector<MorningRushObstacle>& getObstacles() const { return obstacles; }
    const std::vector<RushPlatform>& getPlatforms() const { return platforms; }
    const std::vector<RushPickup>& getPickups() const { return pickups; }
    const MorningRushLuckEvent& getLuckEvent() const { return luckEvent; }
    bool canStandOnObstacle(const MorningRushObstacle& obstacle) const { return obstacle.standableTop; }

    bool hasSleepPenalty() const { return sleptShort; }
    bool hasBicycle() const { return hasBicycle_; }
    float getBurstLungeOffset() const { return burstLungeOffset; }
    float getStamina() const { return stamina; }
    float getMaxStamina() const { return maxStamina; }
    int getComboCount() const { return comboCount; }

    const MorningRushLevel& getLevel() const { return level; }
    const std::vector<PlatformerRect>& getSolidRects() const { return level.getWorld().getSolids(); }
    const std::vector<PlatformerRect>& getOneWayRects() const { return level.getWorld().getOneWays(); }
    const std::vector<PlatformerRect>& getHazardRects() const { return level.getWorld().getHazards(); }
    const std::vector<PlatformerSlope>& getSlopeRects() const { return level.getWorld().getSlopes(); }
    const std::vector<PlatformerPickup>& getWorldPickups() const { return level.getWorld().getPickups(); }
    const PlatformerRect& getFinishRect() const { return level.getWorld().getFinish(); }

    float worldToScreenX(float worldX) const { return worldX - cameraX; }
    float worldToScreenY(float worldY) const { return worldY; }

private:
    void resetRunState();
    void finishRun(Player& player, int& teacherTrust);
    void triggerLuckEvent(Player& player);
    void checkHazards();
    void checkPickups();
    float effectiveAutoSpeed() const;
    void cueAction(RushAction action, float duration = 0.42f);
    void rebuildLegacyViews();
    MorningRushObstacleRule obstacleRuleForName(const std::string& name) const;
    bool satisfiesObstacleRule(const PlatformerRect& hazard) const;
    bool isNearWallTurnGate() const;
    std::string obstacleHint(MorningRushObstacleRule rule) const;

    bool active = false;
    bool completed = false;
    bool resultApplied = false;
    bool practiceMode = false;
    bool sleptShort = false;
    bool breakfastRisk = false;
    int checkPenalty = 0;
    int dayStarted = -1;
    int stageIndex = 1;
    MorningRushRoute route = MorningRushRoute::Campus;
    float pulse = 0.0f;

    MorningRushPhase phase = MorningRushPhase::Intro;
    MorningRushOutcome outcome = MorningRushOutcome::None;
    std::string resultText;

    MorningRushLevel level;
    PlatformerController controller;

    float fixedAccumulator = 0.0f;
    float cameraX = 0.0f;
    float distance = 0.0f;
    float targetDistance = 3500.0f;
    float timeLeft = 50.0f;
    float baseRunSpeed = 155.0f;
    float nextLuckDistance = 800.0f;
    float invulnerableTimer = 0.0f;
    RushAction actionCue = RushAction::None;
    float actionCueTimer = 0.0f;

    bool jumpQueued = false;
    bool slideQueued = false;
    bool burstImpulseQueued = false;
    bool burstRunning = false;
    bool facingRight = true;
    bool previousJumpKeyHeld = false;
    bool previousBurstKeyHeld = false;
    bool hasBicycle_ = false;
    bool pitFailure = false;
    bool pitFallPending = false;
    int collisionCount = 0;
    int comboCount = 0;
    float stamina = 100.0f;
    float maxStamina = 100.0f;
    float burstDashCooldown = 0.0f;
    float burstLungeTimer = 0.0f;
    float burstLungeOffset = 0.0f;

    std::vector<MorningRushObstacle> obstacles;
    std::vector<RushPlatform> platforms;
    std::vector<RushPickup> pickups;
    std::vector<PlatformerRect> triggeredHazards;
    MorningRushLuckEvent luckEvent;
};

#endif
