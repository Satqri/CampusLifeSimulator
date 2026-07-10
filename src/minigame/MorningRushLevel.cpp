#include "minigame/MorningRushLevel.h"

#include <algorithm>
#include <cmath>

bool MorningRushLevel::loadFromFile(const std::string& path) {
    int stage = 1;
    if (path.find("stage_2") != std::string::npos) stage = 2;
    if (path.find("stage_3") != std::string::npos) stage = 3;
    // Runtime geometry mirrors the validated Tiled maps while the external
    // TMX loader remains disabled because of its VS Debug allocator crash.
    buildStage(stage);
    return true;
}

void MorningRushLevel::buildFallback() {
    buildStage(1);
}

void MorningRushLevel::buildStage(int stage) {
    world.clear();
    tiles.clear();
    world.setBounds(4608.0f, 760.0f);
    world.setSpawn(96.0f, 364.0f);
    world.setFinish({4512.0f, 330.0f, 64.0f, 90.0f, "ClassDoor"});
    world.addSolid({-16.0f, 0.0f, 16.0f, 760.0f, "StartWall"});
    world.addSolid({4608.0f, 0.0f, 16.0f, 760.0f, "EndWall"});

    auto addGround = [this](const PlatformerRect& rect) {
        world.addSolid(rect);
        addVisualTilesForRect(rect);
    };
    auto addOneWay = [this](const PlatformerRect& rect) {
        world.addOneWay({rect.x, rect.y, rect.width, 28.0f, rect.name});
        addVisualTilesForRect(rect);
    };
    auto addObstacle = [this](const PlatformerRect& rect, bool solid) {
        world.addHazard(rect);
        if (solid) {
            world.addSolid({rect.x + 6.0f, rect.y,
                            std::max(8.0f, rect.width - 12.0f),
                            std::max(8.0f, rect.height), rect.name});
        }
    };

    if (stage == 2) {
        addGround({0.0f, 420.0f, 4608.0f, 124.0f, "GroundSegment1"});
        const PlatformerRect platforms[] = {
            {400, 356, 192, 18, "GreenPlatform"}, {680, 308, 192, 18, "GreenPlatform"},
            {960, 260, 192, 18, "GreenPlatform"}, {1280, 340, 192, 18, "GreenPlatform"},
            {1760, 308, 192, 18, "GreenPlatform"}, {2220, 260, 192, 18, "GreenPlatform"},
            {2760, 324, 192, 18, "GreenPlatform"}, {3320, 276, 192, 18, "GreenPlatform"},
            {3890, 324, 192, 18, "GreenPlatform"}
        };
        for (const auto& platform : platforms) addOneWay(platform);
        addObstacle({1536, 338, 58, 82, "Hurdle RecyclingBin"}, true);
        addObstacle({2520, 300, 96, 120, "Hurdle FenceLeft"}, true);
        addObstacle({3072, 358, 104, 62, "Hurdle CampusPlanter"}, true);
        addObstacle({3650, 300, 96, 120, "Hurdle FenceRight"}, true);
        world.addPickup({PlatformerPickupKind::Bicycle,
                         {2264.0f, 196.0f, 104.0f, 64.0f, "Bicycle"}, false});
    } else if (stage == 3) {
        const PlatformerRect grounds[] = {
            {0, 420, 900, 124, "GroundSegment1"},
            {1420, 420, 560, 124, "GroundSegment2"},
            {2500, 420, 560, 124, "GroundSegment3"},
            {3600, 420, 1008, 124, "GroundSegment4"}
        };
        for (const auto& ground : grounds) addGround(ground);
        const PlatformerRect platforms[] = {
            {760, 330, 184, 18, "GreenPlatform"}, {1018, 276, 176, 18, "GreenPlatform"},
            {1238, 276, 168, 18, "GreenPlatform"},
            {1788, 334, 184, 18, "GreenPlatform"}, {2040, 282, 184, 18, "GreenPlatform"},
            {2280, 268, 176, 18, "GreenPlatform"},
            {2868, 332, 156, 18, "GreenPlatform"}, {3148, 278, 148, 18, "GreenPlatform"},
            {3404, 260, 140, 18, "GreenPlatform"}
        };
        for (const auto& platform : platforms) addOneWay(platform);
        addObstacle({520, 344, 54, 76, "Hurdle WetFloorSign"}, true);
        addObstacle({1500, 338, 58, 82, "Hurdle RecyclingBin"}, true);
        addObstacle({2600, 334, 104, 86, "Hurdle CrowdBarrier"}, true);
        addObstacle({900, 420, 520, 124, "Pit Gap1"}, false);
        addObstacle({1980, 420, 520, 124, "Pit Gap2"}, false);
        addObstacle({3060, 420, 540, 124, "Pit Gap3"}, false);
        world.addPickup({PlatformerPickupKind::Bicycle,
                         {2318.0f, 204.0f, 104.0f, 64.0f, "Bicycle"}, false});
    } else {
        addGround({0.0f, 420.0f, 4608.0f, 124.0f, "GroundSegment1"});
        const PlatformerRect platforms[] = {
            {2440, 356, 192, 18, "GreenPlatform"}, {2712, 308, 192, 18, "GreenPlatform"},
            {2992, 356, 192, 18, "GreenPlatform"}, {3264, 308, 192, 18, "GreenPlatform"}
        };
        for (const auto& platform : platforms) addOneWay(platform);
        addObstacle({620, 344, 54, 76, "Hurdle WetFloorSign"}, true);
        addObstacle({960, 370, 64, 50, "Hurdle BoxA"}, true);
        addObstacle({1300, 364, 44, 56, "Hurdle TrafficCone"}, true);
        addObstacle({2200, 320, 64, 100, "Hurdle BoxStack"}, true);
        addObstacle({4000, 370, 64, 50, "Hurdle BoxFinish"}, true);
        world.addPickup({PlatformerPickupKind::Bicycle,
                         {2756.0f, 244.0f, 104.0f, 64.0f, "Bicycle"}, false});
    }
}

void MorningRushLevel::addVisualTilesForRect(const PlatformerRect& rect) {
    const float tileSize = 16.0f;
    const int cols = std::max(1, static_cast<int>(std::ceil(rect.width / tileSize)));
    const int rows = std::max(1, static_cast<int>(std::ceil(rect.height / tileSize)));
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            tiles.push_back({rect.x + x * tileSize, rect.y + y * tileSize, tileSize, tileSize});
        }
    }
}
