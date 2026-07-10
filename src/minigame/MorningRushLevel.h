#ifndef CLS_MINIGAME_MORNINGRUSHLEVEL_H
#define CLS_MINIGAME_MORNINGRUSHLEVEL_H

#include "minigame/PlatformerCollisionWorld.h"
#include <string>
#include <vector>

struct MorningRushTile {
    float x = 0.0f;
    float y = 0.0f;
    float width = 16.0f;
    float height = 16.0f;
};

class MorningRushLevel {
public:
    bool loadFromFile(const std::string& path);
    void buildStage(int stage);
    void buildFallback();

    const PlatformerCollisionWorld& getWorld() const { return world; }
    PlatformerCollisionWorld& editWorld() { return world; }
    const std::vector<MorningRushTile>& getTiles() const { return tiles; }

private:
    void addVisualTilesForRect(const PlatformerRect& rect);

    PlatformerCollisionWorld world;
    std::vector<MorningRushTile> tiles;
};

#endif
