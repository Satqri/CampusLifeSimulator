#include "minigame/MorningRushGame.h"
#include "utils/AssetPath.h"

#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace {
constexpr float kGroundY = 420.0f;
constexpr float kFixedStep = 1.0f / 120.0f;
constexpr float kCameraLead = 260.0f;
constexpr float kBurstDashDistance = 170.0f;
constexpr float kBurstDashCost = 14.0f;
constexpr float kBurstDashCooldown = 0.48f;
constexpr float kBurstLungeDuration = 0.24f;

PickupKind toRushPickupKind(PlatformerPickupKind) {
    return PickupKind::Bicycle;
}

bool isJumpKey(sf::Keyboard::Key key) {
    return key == sf::Keyboard::Key::W;
}

bool isDownKey(sf::Keyboard::Key key) {
    return key == sf::Keyboard::Key::J;
}

bool isWallTurnKey(sf::Keyboard::Key key) {
    return key == sf::Keyboard::Key::T;
}

bool keyHeld(sf::Keyboard::Key key) {
    return sf::Keyboard::isKeyPressed(key);
}

bool isOnOrLeavingMatchingSolidTop(const PlatformerRect& runner,
                                   const PlatformerRect& hazard,
                                   const std::vector<PlatformerRect>& solids) {
    if (hazard.name.find("Pit") != std::string::npos) return false;

    for (const auto& solid : solids) {
        if (solid.name != hazard.name) continue;
        const float overlap = std::min(runner.right(), solid.right())
            - std::max(runner.left(), solid.left());
        if (overlap <= 0.5f) continue;

        const bool onTop = runner.bottom() >= solid.top() - 3.0f
            && runner.bottom() <= solid.top() + 7.0f
            && runner.top() < solid.top();
        const bool leavingTopEdge = runner.top() < solid.top()
            && runner.bottom() <= solid.top() + 34.0f;
        if (onTop || leavingTopEdge) return true;
    }

    return false;
}

bool hasClearedHurdleFromAboveOrRight(const PlatformerRect& runner,
                                      const PlatformerRect& hazard) {
    if (hazard.name.find("Pit") != std::string::npos) return false;
    const float runnerCenterX = runner.x + runner.width * 0.5f;
    const float hazardCenterX = hazard.x + hazard.width * 0.5f;
    const bool aboveHurdleTop = runner.bottom() <= hazard.top() + 22.0f;
    const bool alreadyPastRightSide = runnerCenterX > hazardCenterX
        && runner.bottom() <= hazard.bottom() + 18.0f;
    return aboveHurdleTop || alreadyPastRightSide;
}

bool isLeftSideObstacleImpact(const PlatformerRect& runner,
                              const PlatformerRect& hazard) {
    if (hazard.name.find("Pit") != std::string::npos) return true;
    const float runnerCenterX = runner.x + runner.width * 0.5f;
    const float hazardCenterX = hazard.x + hazard.width * 0.5f;
    const bool reachesBody = runner.bottom() > hazard.top() + 18.0f;
    const bool touchesLeftFace = runner.right() > hazard.left() + 4.0f
        && runner.left() < hazardCenterX;
    return reachesBody && runnerCenterX <= hazardCenterX && touchesLeftFace;
}

bool sameHazardRect(const PlatformerRect& a, const PlatformerRect& b) {
    return std::fabs(a.x - b.x) < 0.01f
        && std::fabs(a.y - b.y) < 0.01f
        && std::fabs(a.width - b.width) < 0.01f
        && std::fabs(a.height - b.height) < 0.01f
        && a.name == b.name;
}

}

void MorningRushGame::start(int day, bool shortSleep, bool mealRisk, int penalty,
                            MorningRushRoute selectedRoute, bool practice) {
    active = true;
    completed = false;
    resultApplied = false;
    sleptShort = shortSleep;
    breakfastRisk = mealRisk;
    checkPenalty = penalty;
    practiceMode = practice;
    dayStarted = day;
    route = selectedRoute;
    phase = MorningRushPhase::Intro;
    outcome = MorningRushOutcome::None;
    resultText.clear();
    resetRunState();
}

void MorningRushGame::resetRunState() {
    fixedAccumulator = 0.0f;
    cameraX = 0.0f;
    timeLeft = 58.0f;
    baseRunSpeed = 162.0f;
    nextLuckDistance = 800.0f;
    invulnerableTimer = 0.0f;
    maxStamina = sleptShort ? 82.0f : 100.0f;
    stamina = maxStamina;
    burstRunning = false;
    facingRight = true;
    jumpQueued = false;
    slideQueued = false;
    burstImpulseQueued = false;
    hasBicycle_ = false;
    pitFailure = false;
    pitFallPending = false;
    triggeredHazards.clear();
    collisionCount = 0;
    comboCount = 0;
    luckEvent = {};
    actionCue = RushAction::None;
    actionCueTimer = 0.0f;
    burstDashCooldown = 0.0f;
    burstLungeTimer = 0.0f;
    burstLungeOffset = 0.0f;
    previousJumpKeyHeld = false;
    previousBurstKeyHeld = false;

    if (sleptShort) {
        baseRunSpeed *= 0.9f;
        timeLeft -= 2.0f;
    }
    if (route == MorningRushRoute::Rainy) {
        baseRunSpeed *= 0.94f;
    }
    if (route == MorningRushRoute::Dormitory) {
        timeLeft += 1.0f;
    } else if (route == MorningRushRoute::Library) {
        baseRunSpeed *= 1.04f;
    } else if (route == MorningRushRoute::Cafeteria) {
        timeLeft -= 1.0f;
    }

    switch (route) {
        case MorningRushRoute::Library: stageIndex = 1; break;
        case MorningRushRoute::Cafeteria: stageIndex = 2; break;
        case MorningRushRoute::Dormitory: stageIndex = 3; break;
        case MorningRushRoute::Campus:
        case MorningRushRoute::Rainy:
            stageIndex = 1;
            break;
    }
    level.loadFromFile(cls::resolveAssetPath(
        "assets/maps/morning_rush_green/morning_rush_stage_"
        + std::to_string(stageIndex) + ".tmx"));
    targetDistance = std::max(900.0f, level.getWorld().getFinish().x);
    controller.reset(level.getWorld().getSpawnX(), level.getWorld().getSpawnY(), sleptShort);
    distance = controller.getBox().x;
    rebuildLegacyViews();
}

bool MorningRushGame::handleInput(const sf::Event& event, Player&, int&) {
    if (!active) return false;

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (phase == MorningRushPhase::Intro) {
            if (key->code == sf::Keyboard::Key::Enter) {
                phase = MorningRushPhase::Running;
            }
            return true;
        }

        if (phase == MorningRushPhase::FinalResult) {
            if (key->code == sf::Keyboard::Key::Enter) {
                active = false;
                completed = true;
            }
            return true;
        }

        if (phase == MorningRushPhase::Running) {
            if (isJumpKey(key->code)) {
                jumpQueued = true;
                cueAction(RushAction::Jump, 0.28f);
            } else if (isWallTurnKey(key->code)) {
                cueAction(RushAction::WallTurn, 0.72f);
            } else if (key->code == sf::Keyboard::Key::K) {
                if (!hasBicycle_) {
                    luckEvent = {"K locked", "Pick up the bicycle to unlock the lunge.",
                                 0, 0, false, 1.5f};
                } else if (stamina >= kBurstDashCost && burstDashCooldown <= 0.0f) {
                    burstImpulseQueued = true;
                    burstDashCooldown = kBurstDashCooldown;
                    burstLungeTimer = kBurstLungeDuration;
                    cueAction(RushAction::BurstRun, 0.42f);
                }
            }
            return true;
        }
    }

    return true;
}

void MorningRushGame::update(float deltaTime, Player& player, int& teacherTrust) {
    if (!active) return;

    const float dt = std::clamp(deltaTime, 0.0f, 0.05f);
    pulse += dt;

    if (luckEvent.displayTimer > 0.0f) {
        luckEvent.displayTimer = std::max(0.0f, luckEvent.displayTimer - dt);
    }
    if (actionCueTimer > 0.0f) {
        actionCueTimer = std::max(0.0f, actionCueTimer - dt);
        if (actionCueTimer <= 0.0f) actionCue = RushAction::None;
    }

    if (phase != MorningRushPhase::Running) {
        previousJumpKeyHeld = keyHeld(sf::Keyboard::Key::W);
        previousBurstKeyHeld = keyHeld(sf::Keyboard::Key::K);
        return;
    }

    timeLeft -= dt;
    invulnerableTimer = std::max(0.0f, invulnerableTimer - dt);
    burstDashCooldown = std::max(0.0f, burstDashCooldown - dt);
    burstLungeTimer = std::max(0.0f, burstLungeTimer - dt);

    const bool jumpHeldNow = keyHeld(sf::Keyboard::Key::W);
    const bool burstHeldNow = keyHeld(sf::Keyboard::Key::K);
    if (jumpHeldNow && !previousJumpKeyHeld) {
        jumpQueued = true;
        cueAction(RushAction::Jump, 0.28f);
    }
    if (hasBicycle_ && burstHeldNow && !previousBurstKeyHeld &&
        stamina >= kBurstDashCost && burstDashCooldown <= 0.0f) {
        burstImpulseQueued = true;
        burstDashCooldown = kBurstDashCooldown;
        burstLungeTimer = kBurstLungeDuration;
        cueAction(RushAction::BurstRun, 0.42f);
    }
    previousJumpKeyHeld = jumpHeldNow;
    previousBurstKeyHeld = burstHeldNow;

    if (!controller.isHitStunned()) {
        if (keyHeld(sf::Keyboard::Key::D) && !keyHeld(sf::Keyboard::Key::A)) {
            facingRight = true;
        } else if (keyHeld(sf::Keyboard::Key::A) && !keyHeld(sf::Keyboard::Key::D)) {
            facingRight = false;
        }
    }
    if (burstLungeTimer > 0.0f) {
        const float t = burstLungeTimer / kBurstLungeDuration;
        burstLungeOffset = (facingRight ? 72.0f : -72.0f) * std::sin(t * 3.1415926f);
    } else {
        burstLungeOffset = 0.0f;
    }
    burstRunning = burstLungeTimer > 0.0f;
    if (!burstRunning) stamina = std::min(maxStamina, stamina + 22.0f * dt);

    fixedAccumulator = std::min(0.12f, fixedAccumulator + dt);
    int steps = 0;
    while (fixedAccumulator >= kFixedStep && steps < 10) {
        PlatformerInput input;
        input.jumpPressed = jumpQueued;
        input.slidePressed = slideQueued;
        input.jumpHeld = jumpHeldNow;
        input.downHeld = keyHeld(sf::Keyboard::Key::J);
        input.moveLeft = keyHeld(sf::Keyboard::Key::A);
        input.moveRight = keyHeld(sf::Keyboard::Key::D);
        input.facingRight = facingRight;

        controller.update(kFixedStep, input, level.getWorld(), effectiveAutoSpeed());
        if (burstImpulseQueued) {
            controller.burstForward(facingRight ? kBurstDashDistance : -kBurstDashDistance,
                                    level.getWorld());
            stamina = std::max(0.0f, stamina - kBurstDashCost);
            burstImpulseQueued = false;
        }

        jumpQueued = false;
        slideQueued = false;
        fixedAccumulator -= kFixedStep;
        ++steps;
    }

    distance = controller.getBox().x;
    const float maxCamera = std::max(0.0f, level.getWorld().getWidth() - 960.0f);
    const float targetCameraX = std::clamp(distance - kCameraLead, 0.0f, maxCamera);
    const float cameraFollow = std::clamp(dt * 5.5f, 0.0f, 1.0f);
    cameraX += (targetCameraX - cameraX) * cameraFollow;

    while (distance >= nextLuckDistance && nextLuckDistance < targetDistance) {
        triggerLuckEvent(player);
        nextLuckDistance += 800.0f;
    }

    checkHazards();
    if (pitFailure) {
        finishRun(player, teacherTrust);
        return;
    }
    checkPickups();
    rebuildLegacyViews();

    if (PlatformerCollisionWorld::overlaps(controller.getBox(), level.getWorld().getFinish())
        || distance >= targetDistance
        || timeLeft <= 0.0f) {
        finishRun(player, teacherTrust);
    }
}

float MorningRushGame::effectiveAutoSpeed() const {
    float speed = baseRunSpeed;
    if (invulnerableTimer > 0.0f) speed *= 0.88f;
    return speed;
}

void MorningRushGame::cueAction(RushAction action, float duration) {
    actionCue = action;
    actionCueTimer = duration;
}

void MorningRushGame::triggerLuckEvent(Player& player) {
    const int roll = std::rand() % 20 + 1;
    const int energyBonus = (player.getAttributes().energy - 50) / 10;
    const int dc = 11 + checkPenalty;
    const bool success = roll + energyBonus >= dc;

    luckEvent.title = "Campus luck check";
    luckEvent.d20 = roll;
    luckEvent.dc = dc;
    luckEvent.success = success;
    luckEvent.displayTimer = 2.6f;

    if (success) {
        timeLeft += 2.0f;
        ++comboCount;
        luckEvent.text = "Found a cleaner line through the crowd. +2s";
    } else {
        timeLeft -= 2.0f;
        comboCount = 0;
        luckEvent.text = "A sudden jam slows the sprint. -2s";
    }
}

void MorningRushGame::checkHazards() {
    if (pitFallPending) {
        if (controller.getBox().top() > 560.0f) {
            pitFailure = true;
            ++collisionCount;
            comboCount = 0;
            luckEvent = {"Late arrival", "Fell off the route. Morning Rush failed.",
                         0, 0, false, 2.0f};
            return;
        }
        if (controller.isGrounded()) {
            pitFallPending = false;
        }
    }

    const PlatformerRect* hazard = level.getWorld().findHazard(controller.getBox());
    if (!hazard) return;

    const PlatformerRect hitHazard = *hazard;
    if (hitHazard.name.find("Pit") != std::string::npos) {
        pitFallPending = true;
        return;
    }
    if (isOnOrLeavingMatchingSolidTop(controller.getBox(), hitHazard,
                                      level.getWorld().getSolids())) {
        return;
    }
    if (hasClearedHurdleFromAboveOrRight(controller.getBox(), hitHazard)) {
        return;
    }
    if (!isLeftSideObstacleImpact(controller.getBox(), hitHazard)) {
        return;
    }
    if (invulnerableTimer > 0.0f) return;
    const bool alreadyTriggered = std::any_of(triggeredHazards.begin(), triggeredHazards.end(),
        [&hitHazard](const PlatformerRect& hazard) {
            return sameHazardRect(hazard, hitHazard);
        });
    if (alreadyTriggered) return;

    const MorningRushObstacleRule rule = obstacleRuleForName(hitHazard.name);
    if (satisfiesObstacleRule(hitHazard)) {
        ++comboCount;
        timeLeft = std::min(65.0f, timeLeft + 0.65f);
        invulnerableTimer = 0.12f;
        luckEvent = {"Clean move", obstacleHint(rule) + " cleared. +0.6s", 0, 0, true, 1.6f};
        return;
    }

    controller.applyHazardBounce(hitHazard.x + hitHazard.width * 0.5f);
    triggeredHazards.push_back(hitHazard);

    timeLeft -= 3.0f;
    ++collisionCount;
    comboCount = 0;
    invulnerableTimer = 0.86f;
    luckEvent = {"Wrong move", obstacleHint(rule) + " needed. -3s", 0, 0, false, 2.0f};
}

void MorningRushGame::checkPickups() {
    auto& worldPickups = level.editWorld().editPickups();
    for (auto& pickup : worldPickups) {
        if (pickup.collected) continue;
        if (!PlatformerCollisionWorld::overlaps(controller.getBox(), pickup.rect)) continue;

        pickup.collected = true;
        ++comboCount;

        hasBicycle_ = true;
        luckEvent = {"Bicycle acquired", "K lunge unlocked.", 0, 0, true, 2.2f};
    }
}

void MorningRushGame::finishRun(Player& player, int& teacherTrust) {
    if (resultApplied) return;
    resultApplied = true;
    phase = MorningRushPhase::FinalResult;

    const bool reachedClass = distance >= targetDistance
        || PlatformerCollisionWorld::overlaps(controller.getBox(), level.getWorld().getFinish());
    const int remaining = static_cast<int>(std::floor(std::max(0.0f, timeLeft)));
    const auto applyAttributes = [&](const Attributes& delta) {
        if (!practiceMode) player.modifyAttributes(delta);
    };

    std::ostringstream text;
    if (pitFailure) {
        outcome = MorningRushOutcome::Failure;
        applyAttributes(Attributes{.san = -5, .academic = -3});
        text << "Fell off the route and arrived late. Academic -3, SAN -5.";
        resultText = text.str();
        return;
    }
    if (reachedClass && remaining >= 12) {
        outcome = MorningRushOutcome::GreatSuccess;
        applyAttributes(Attributes{.san = 2, .academic = 1});
        text << "Arrived with " << remaining << "s left. SAN +2, Academic +1.";
    } else if (reachedClass || remaining > 0) {
        outcome = MorningRushOutcome::Success;
        applyAttributes(Attributes{.san = 2});
        text << "Barely made it through the door. SAN +2.";
    } else if (collisionCount >= 4 || timeLeft < -5.0f) {
        outcome = MorningRushOutcome::CriticalFailure;
        applyAttributes(Attributes{.san = -8, .academic = -5});
        if (!practiceMode) teacherTrust -= 10;
        text << "Missed the bell badly. Academic -5, SAN -8, Teacher Trust -10.";
    } else {
        outcome = MorningRushOutcome::Failure;
        applyAttributes(Attributes{.san = -5, .academic = -3});
        text << "Reached class late. Academic -3, SAN -5.";
    }
    resultText = text.str();
}

TerrainZone MorningRushGame::getCurrentZone() const {
    const float pct = targetDistance > 0.0f ? distance / targetDistance : 0.0f;
    if (pct < 0.34f) return TerrainZone::Dormitory;
    if (pct < 0.78f) return TerrainZone::Campus;
    return TerrainZone::Classroom;
}

bool MorningRushGame::consumeCompleted() {
    if (!completed) return false;
    completed = false;
    return true;
}

bool MorningRushGame::wasLate() const {
    return outcome == MorningRushOutcome::Failure
        || outcome == MorningRushOutcome::CriticalFailure;
}

float MorningRushGame::getRunnerLift() const {
    return std::max(0.0f, kGroundY - controller.getBox().bottom());
}

float MorningRushGame::getRunnerScreenX() const {
    return worldToScreenX(controller.getBox().x);
}

RushAction MorningRushGame::getLastAction() const {
    switch (controller.getLastAction()) {
        case PlatformerAction::Jump: return RushAction::Jump;
        case PlatformerAction::DoubleJump: return RushAction::DoubleJump;
        case PlatformerAction::Slide: return RushAction::Squeeze;
        case PlatformerAction::WallTouch: return RushAction::WallTurn;
        default:
            return burstRunning ? RushAction::BurstRun : RushAction::None;
    }
}

MorningRushObstacleRule MorningRushGame::obstacleRuleForName(const std::string& name) const {
    if (name.find("Hurdle") != std::string::npos
        || name.find("Railing") != std::string::npos
        || name.find("Flowerbed") != std::string::npos
        || name.find("Fountain") != std::string::npos
        || name.find("Fence") != std::string::npos) {
        return MorningRushObstacleRule::Hurdle;
    }
    if (name.find("Squeeze") != std::string::npos
        || name.find("Crowd") != std::string::npos
        || name.find("Narrow") != std::string::npos) {
        return MorningRushObstacleRule::Squeeze;
    }
    if (name.find("WallTurn") != std::string::npos
        || name.find("Wall Turn") != std::string::npos
        || name.find("Pillar") != std::string::npos
        || name.find("Doorframe") != std::string::npos) {
        return MorningRushObstacleRule::WallTurn;
    }
    if (name.find("Burst") != std::string::npos
        || name.find("Sprint") != std::string::npos
        || name.find("Corridor") != std::string::npos) {
        return MorningRushObstacleRule::BurstRun;
    }
    return MorningRushObstacleRule::Clear;
}

bool MorningRushGame::satisfiesObstacleRule(const PlatformerRect& hazard) const {
    const MorningRushObstacleRule rule = obstacleRuleForName(hazard.name);
    const RushAction currentAction = actionCueTimer > 0.0f ? actionCue : getLastAction();

    switch (rule) {
        case MorningRushObstacleRule::Hurdle:
            // Hurdles are cleared by physically passing above them. Reaching
            // this overlap check means the player actually touched one.
            return false;
        case MorningRushObstacleRule::Squeeze:
            return controller.isSliding()
                || controller.isDucking()
                || currentAction == RushAction::Squeeze;
        case MorningRushObstacleRule::WallTurn:
            return currentAction == RushAction::WallTurn;
        case MorningRushObstacleRule::BurstRun:
            return burstRunning || currentAction == RushAction::BurstRun;
        case MorningRushObstacleRule::Clear:
            return true;
    }
    return false;
}

bool MorningRushGame::isNearWallTurnGate() const {
    const PlatformerRect& runner = controller.getBox();
    for (const auto& hazard : level.getWorld().getHazards()) {
        if (obstacleRuleForName(hazard.name) != MorningRushObstacleRule::WallTurn) continue;
        const bool nearX = hazard.left() <= runner.right() + 92.0f
            && hazard.right() >= runner.left() - 18.0f;
        const bool nearY = runner.bottom() >= hazard.top() - 54.0f
            && runner.top() <= hazard.bottom() + 12.0f;
        if (nearX && nearY) return true;
    }
    return false;
}

std::string MorningRushGame::obstacleHint(MorningRushObstacleRule rule) const {
    switch (rule) {
        case MorningRushObstacleRule::Hurdle: return "W hurdle";
        case MorningRushObstacleRule::Squeeze: return "J crawl";
        case MorningRushObstacleRule::WallTurn: return "T wall turn";
        case MorningRushObstacleRule::BurstRun: return "K burst run";
        case MorningRushObstacleRule::Clear: return "Clean line";
    }
    return "Clean line";
}

void MorningRushGame::rebuildLegacyViews() {
    platforms.clear();
    platforms.reserve(level.getWorld().getOneWays().size());
    for (const auto& platform : level.getWorld().getOneWays()) {
        platforms.push_back({platform.x, platform.y, platform.width, 18.0f,
                             false, platform.name});
    }

    obstacles.clear();
    obstacles.reserve(level.getWorld().getHazards().size());
    for (const auto& hazard : level.getWorld().getHazards()) {
        MorningRushObstacle obstacle;
        obstacle.name = hazard.name.empty() ? "Hazard" : hazard.name;
        obstacle.kind = MorningRushObstacleKind::Ground;
        obstacle.rule = obstacleRuleForName(obstacle.name);
        obstacle.x = hazard.x;
        obstacle.width = hazard.width;
        obstacle.height = hazard.height;
        obstacle.yOffset = std::max(0.0f, kGroundY - hazard.y);
        obstacle.penaltySeconds = 3;
        obstacle.destructible = true;
        obstacle.standableTop = false;
        obstacles.push_back(obstacle);
    }

    pickups.clear();
    pickups.reserve(level.getWorld().getPickups().size());
    for (const auto& pickup : level.getWorld().getPickups()) {
        pickups.push_back({toRushPickupKind(pickup.kind),
                           pickup.rect.x,
                           pickup.rect.y,
                           pickup.rect.width,
                           pickup.rect.height,
                           pickup.collected});
    }
}
