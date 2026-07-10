#ifndef CLS_MINIGAME_PLATFORMERCOLLISIONWORLD_H
#define CLS_MINIGAME_PLATFORMERCOLLISIONWORLD_H

#include <string>
#include <vector>

enum class PlatformerPickupKind {
    Bicycle
};

struct PlatformerRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    std::string name;

    float left() const { return x; }
    float right() const { return x + width; }
    float top() const { return y; }
    float bottom() const { return y + height; }
};

struct PlatformerPickup {
    PlatformerPickupKind kind = PlatformerPickupKind::Bicycle;
    PlatformerRect rect;
    bool collected = false;
};

struct PlatformerSlope {
    float x = 0.0f;
    float bottomY = 420.0f;
    float width = 0.0f;
    float height = 0.0f;
    bool risesRight = true;
    std::string name;

    float left() const { return x; }
    float right() const { return x + width; }
    float surfaceY(float worldX) const;
};

class PlatformerCollisionWorld {
public:
    void clear();
    void setBounds(float width, float height);

    void addSolid(const PlatformerRect& rect);
    void addOneWay(const PlatformerRect& rect);
    void addHazard(const PlatformerRect& rect);
    void addPickup(const PlatformerPickup& pickup);
    void addSlope(const PlatformerSlope& slope);

    void setSpawn(float x, float y);
    void setFinish(const PlatformerRect& rect);

    float getWidth() const { return worldWidth; }
    float getHeight() const { return worldHeight; }
    float getSpawnX() const { return spawnX; }
    float getSpawnY() const { return spawnY; }
    const PlatformerRect& getFinish() const { return finish; }

    const std::vector<PlatformerRect>& getSolids() const { return solids; }
    const std::vector<PlatformerRect>& getOneWays() const { return oneWays; }
    const std::vector<PlatformerRect>& getHazards() const { return hazards; }
    const std::vector<PlatformerPickup>& getPickups() const { return pickups; }
    const std::vector<PlatformerSlope>& getSlopes() const { return slopes; }
    std::vector<PlatformerPickup>& editPickups() { return pickups; }

    bool intersectsSolid(const PlatformerRect& box) const;
    bool canStandOnOneWay(const PlatformerRect& box, float previousBottom,
                          bool dropThrough, PlatformerRect& landed) const;
    bool snapToGround(PlatformerRect& box, float maxDistance, bool dropThrough,
                      PlatformerRect& landed) const;
    bool resolveHorizontal(PlatformerRect& box, float& velocityX) const;
    bool resolveVertical(PlatformerRect& box, float& velocityY,
                         float previousBottom, bool dropThrough,
                         bool& grounded) const;
    const PlatformerRect* findHazard(const PlatformerRect& box) const;
    bool removeHazard(const PlatformerRect& target);

    static bool overlaps(const PlatformerRect& a, const PlatformerRect& b);

private:
    void removeSolidWithName(const std::string& name);

    float worldWidth = 3600.0f;
    float worldHeight = 540.0f;
    float spawnX = 96.0f;
    float spawnY = 364.0f;
    PlatformerRect finish{3500.0f, 330.0f, 60.0f, 90.0f, "Finish"};

    std::vector<PlatformerRect> solids;
    std::vector<PlatformerRect> oneWays;
    std::vector<PlatformerRect> hazards;
    std::vector<PlatformerPickup> pickups;
    std::vector<PlatformerSlope> slopes;
};

#endif
