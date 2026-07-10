#include "minigame/PlatformerCollisionWorld.h"

#include <algorithm>
#include <cmath>

namespace {
bool isObstacleSolid(const PlatformerRect& rect) {
    return rect.name.find("Hurdle") != std::string::npos;
}

bool isClearingObstacle(const PlatformerRect& box, const PlatformerRect& obstacle) {
    const float boxCenterX = box.x + box.width * 0.5f;
    const float obstacleCenterX = obstacle.x + obstacle.width * 0.5f;
    const bool aboveObstacle = box.bottom() <= obstacle.top() + 22.0f;
    const bool alreadyPastFront = boxCenterX > obstacleCenterX
        && box.bottom() <= obstacle.bottom() + 18.0f;
    return aboveObstacle || alreadyPastFront;
}
}

float PlatformerSlope::surfaceY(float worldX) const {
    const float progress = std::clamp((worldX - x) / std::max(1.0f, width), 0.0f, 1.0f);
    return risesRight ? bottomY - progress * height
                      : bottomY - (1.0f - progress) * height;
}

void PlatformerCollisionWorld::clear() {
    solids.clear();
    oneWays.clear();
    hazards.clear();
    pickups.clear();
    slopes.clear();
    finish = {3500.0f, 330.0f, 60.0f, 90.0f, "Finish"};
    spawnX = 96.0f;
    spawnY = 364.0f;
}

void PlatformerCollisionWorld::setBounds(float width, float height) {
    worldWidth = width;
    worldHeight = height;
}

void PlatformerCollisionWorld::addSolid(const PlatformerRect& rect) {
    if (rect.width > 0.0f && rect.height > 0.0f) solids.push_back(rect);
}

void PlatformerCollisionWorld::addOneWay(const PlatformerRect& rect) {
    if (rect.width > 0.0f && rect.height > 0.0f) oneWays.push_back(rect);
}

void PlatformerCollisionWorld::addHazard(const PlatformerRect& rect) {
    if (rect.width > 0.0f && rect.height > 0.0f) hazards.push_back(rect);
}

void PlatformerCollisionWorld::addPickup(const PlatformerPickup& pickup) {
    pickups.push_back(pickup);
}

void PlatformerCollisionWorld::addSlope(const PlatformerSlope& slope) {
    if (slope.width > 0.0f && slope.height > 0.0f) slopes.push_back(slope);
}

void PlatformerCollisionWorld::setSpawn(float x, float y) {
    spawnX = x;
    spawnY = y;
}

void PlatformerCollisionWorld::setFinish(const PlatformerRect& rect) {
    finish = rect;
}

bool PlatformerCollisionWorld::overlaps(const PlatformerRect& a, const PlatformerRect& b) {
    return a.left() < b.right() && a.right() > b.left()
        && a.top() < b.bottom() && a.bottom() > b.top();
}

bool PlatformerCollisionWorld::intersectsSolid(const PlatformerRect& box) const {
    for (const auto& solid : solids) {
        if (overlaps(box, solid)) return true;
    }
    return false;
}

bool PlatformerCollisionWorld::canStandOnOneWay(const PlatformerRect& box,
                                                float previousBottom,
                                                bool dropThrough,
                                                PlatformerRect& landed) const {
    if (dropThrough) return false;

    for (const auto& platform : oneWays) {
        const float overlap = std::min(box.right(), platform.right())
            - std::max(box.left(), platform.left());
        if (overlap < 8.0f) continue;

        const bool crossedTop = previousBottom <= platform.top() + 10.0f
            && box.bottom() >= platform.top();
        if (crossedTop && box.bottom() <= platform.top() + 30.0f) {
            landed = platform;
            return true;
        }
    }

    return false;
}

bool PlatformerCollisionWorld::snapToGround(PlatformerRect& box,
                                            float maxDistance,
                                            bool dropThrough,
                                            PlatformerRect& landed) const {
    PlatformerRect probe = box;
    probe.y += maxDistance;

    for (const auto& solid : solids) {
        if (isObstacleSolid(solid)) continue;
        const float overlap = std::min(probe.right(), solid.right())
            - std::max(probe.left(), solid.left());
        if (overlap >= 8.0f && box.bottom() <= solid.top()
            && solid.top() - box.bottom() <= maxDistance) {
            box.y = solid.top() - box.height;
            landed = solid;
            return true;
        }
    }

    if (!dropThrough) {
        for (const auto& platform : oneWays) {
            const float overlap = std::min(probe.right(), platform.right())
                - std::max(probe.left(), platform.left());
            if (overlap >= 8.0f && box.bottom() <= platform.top()
                && platform.top() - box.bottom() <= maxDistance) {
                box.y = platform.top() - box.height;
                landed = platform;
                return true;
            }
        }
    }

    return false;
}

bool PlatformerCollisionWorld::resolveHorizontal(PlatformerRect& box,
                                                 float& velocityX) const {
    bool hitWall = false;

    for (const auto& solid : solids) {
        if (isObstacleSolid(solid) && isClearingObstacle(box, solid)) continue;
        if (!overlaps(box, solid)) continue;

        if (velocityX > 0.0f) {
            box.x = solid.left() - box.width;
        } else if (velocityX < 0.0f) {
            box.x = solid.right();
        }
        velocityX = 0.0f;
        hitWall = true;
    }

    if (box.x < 0.0f) {
        box.x = 0.0f;
        velocityX = 0.0f;
        hitWall = true;
    } else if (box.right() > worldWidth) {
        box.x = worldWidth - box.width;
        velocityX = 0.0f;
        hitWall = true;
    }

    return hitWall;
}

bool PlatformerCollisionWorld::resolveVertical(PlatformerRect& box,
                                               float& velocityY,
                                               float previousBottom,
                                               bool dropThrough,
                                               bool& grounded) const {
    grounded = false;
    bool hitCeiling = false;

    for (const auto& solid : solids) {
        if (isObstacleSolid(solid)) continue;
        if (!overlaps(box, solid)) continue;
        const float horizontalOverlap = std::min(box.right(), solid.right())
            - std::max(box.left(), solid.left());

        if (velocityY > 0.0f) {
            if (horizontalOverlap < 8.0f) continue;
            box.y = solid.top() - box.height;
            grounded = true;
        } else if (velocityY < 0.0f) {
            if (horizontalOverlap < 8.0f) continue;
            box.y = solid.bottom();
            hitCeiling = true;
        }
        velocityY = 0.0f;
    }

    if (velocityY >= 0.0f) {
        PlatformerRect landed;
        if (canStandOnOneWay(box, previousBottom, dropThrough, landed)) {
            box.y = landed.top() - box.height;
            velocityY = 0.0f;
            grounded = true;
        }
    }

    if (box.bottom() > worldHeight) {
        box.y = worldHeight - box.height;
        velocityY = 0.0f;
        grounded = true;
    }

    return grounded || hitCeiling;
}

const PlatformerRect* PlatformerCollisionWorld::findHazard(const PlatformerRect& box) const {
    for (const auto& hazard : hazards) {
        if (overlaps(box, hazard)) return &hazard;
    }
    return nullptr;
}

bool PlatformerCollisionWorld::removeHazard(const PlatformerRect& target) {
    auto it = std::find_if(hazards.begin(), hazards.end(), [&target](const PlatformerRect& hazard) {
        return std::fabs(hazard.x - target.x) < 0.01f
            && std::fabs(hazard.y - target.y) < 0.01f
            && std::fabs(hazard.width - target.width) < 0.01f
            && std::fabs(hazard.height - target.height) < 0.01f
            && hazard.name == target.name;
    });
    if (it == hazards.end()) return false;
    const std::string name = it->name;
    hazards.erase(it);
    removeSolidWithName(name);
    return true;
}

void PlatformerCollisionWorld::removeSolidWithName(const std::string& name) {
    if (name.empty()) return;
    solids.erase(std::remove_if(solids.begin(), solids.end(), [&name](const PlatformerRect& solid) {
        return solid.name == name;
    }), solids.end());
}
