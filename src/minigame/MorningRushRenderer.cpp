#include "minigame/MorningRushRenderer.h"
#include "utils/AssetPath.h"
#include "utils/TextUtils.h"
#include "map/MapPortal.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

MorningRushRenderer::MorningRushRenderer(sf::Font& f, const MorningRushGame& g)
    : font(f), game(g) {
    const bool tilesOk = tilemapTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/kenney_pixel_platformer/Tilemap/tilemap.png"));
    const bool bgOk = backgroundTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/kenney_pixel_platformer/Tilemap/tilemap-backgrounds.png"));
    if (tilesOk) tilemapTexture.setSmooth(false);
    if (bgOk) backgroundTexture.setSmooth(false);
    assetsLoaded = tilesOk && bgOk;

    adventureBackgroundLoaded =
        adventureBlueBackgroundTexture.loadFromFile(cls::resolveAssetPath(
            "assets/sprites/parkour/pixel_adventure/Background/Blue.png")) &&
        adventureGreenBackgroundTexture.loadFromFile(cls::resolveAssetPath(
            "assets/sprites/parkour/pixel_adventure/Background/Green.png"));
    adventureTerrainLoaded = adventureTerrainTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/pixel_adventure/Terrain/Terrain_16x16_1.png"));
    adventureEndLoaded = adventureEndTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/pixel_adventure/Items/Checkpoints/End/End_Idle.png"));
    schoolTilesetLoaded = schoolTilesetTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/school/coolschool_tileset_48px/CoolSchool_tileset_48px/CoolSchool_tileset.png"));
    studentActionLoaded = studentActionTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/player/morning_rush_student/student_actions.png"));
    studentActionDirectionalLoaded =
        studentActionLeftTexture.loadFromFile(cls::resolveAssetPath(
            "assets/sprites/player/morning_rush_student/student_actions_left.png")) &&
        studentActionRightTexture.loadFromFile(cls::resolveAssetPath(
            "assets/sprites/player/morning_rush_student/student_actions_right.png"));
    campusBackgroundLoaded =
        campusGateBackgroundTexture.loadFromFile(cls::resolveAssetPath(
            "assets/sprites/parkour/campus_backgrounds/campus_gate.png")) &&
        campusPathBackgroundTexture.loadFromFile(cls::resolveAssetPath(
            "assets/sprites/parkour/campus_backgrounds/campus_path.png"));
    wetFloorSignLoaded = wetFloorSignTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/campus_obstacles/wet_floor_sign.png"));
    trafficConeLoaded = trafficConeTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/campus_obstacles/traffic_cone.png"));
    recyclingBinLoaded = recyclingBinTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/campus_obstacles/recycling_bin.png"));
    campusPlanterLoaded = campusPlanterTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/campus_obstacles/campus_planter.png"));
    crowdBarrierLoaded = crowdBarrierTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/campus_obstacles/crowd_barrier.png"));
    bicyclePickupLoaded = bicyclePickupTexture.loadFromFile(cls::resolveAssetPath(
        "assets/sprites/parkour/campus_pickups/bicycle.png"));
    const std::string greenZoneRoot = "assets/sprites/parkour/green_zone/";
    greenZoneTilesLoaded =
        greenGroundTopTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "ground_top.png")) &&
        greenGroundBodyTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "ground_body.png")) &&
        greenPlatformTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "platform.png"));
    greenZoneObjectsLoaded =
        greenBoxTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "box.png")) &&
        greenRampLeftTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "ramp_left.png")) &&
        greenRampRightTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "ramp_right.png")) &&
        greenHalfPipeTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "half_pipe.png")) &&
        greenFountainLowTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "fountain_low.png")) &&
        greenFountainTallTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "fountain_tall.png")) &&
        greenFenceLeftTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "fence_left.png")) &&
        greenFenceRightTexture.loadFromFile(cls::resolveAssetPath(greenZoneRoot + "fence_right.png"));
    if (studentActionLoaded) studentActionTexture.setSmooth(false);
    if (studentActionDirectionalLoaded) {
        studentActionLeftTexture.setSmooth(false);
        studentActionRightTexture.setSmooth(false);
    }
    if (campusBackgroundLoaded) {
        campusGateBackgroundTexture.setSmooth(true);
        campusPathBackgroundTexture.setSmooth(true);
    }
    if (wetFloorSignLoaded) wetFloorSignTexture.setSmooth(false);
    if (trafficConeLoaded) trafficConeTexture.setSmooth(false);
    if (recyclingBinLoaded) recyclingBinTexture.setSmooth(false);
    if (campusPlanterLoaded) campusPlanterTexture.setSmooth(false);
    if (crowdBarrierLoaded) crowdBarrierTexture.setSmooth(false);
    if (bicyclePickupLoaded) bicyclePickupTexture.setSmooth(false);
    if (greenZoneTilesLoaded) {
        greenGroundTopTexture.setSmooth(false);
        greenGroundBodyTexture.setSmooth(false);
        greenPlatformTexture.setSmooth(false);
    }
    if (greenZoneObjectsLoaded) {
        greenBoxTexture.setSmooth(false);
        greenRampLeftTexture.setSmooth(false);
        greenRampRightTexture.setSmooth(false);
        greenHalfPipeTexture.setSmooth(false);
        greenFountainLowTexture.setSmooth(false);
        greenFountainTallTexture.setSmooth(false);
        greenFenceLeftTexture.setSmooth(false);
        greenFenceRightTexture.setSmooth(false);
    }
    if (adventureBackgroundLoaded) {
        adventureBlueBackgroundTexture.setSmooth(false);
        adventureGreenBackgroundTexture.setSmooth(false);
    }
    if (adventureTerrainLoaded) adventureTerrainTexture.setSmooth(false);
    if (adventureEndLoaded) adventureEndTexture.setSmooth(false);
    if (schoolTilesetLoaded) schoolTilesetTexture.setSmooth(false);
}

void MorningRushRenderer::update(float) {}

void MorningRushRenderer::render(sf::RenderWindow& window) {
    drawScene(window);
    drawHud(window);

    if (game.getPhase() == MorningRushPhase::Intro) {
        drawPixelFrame(window, {196.0f, 156.0f}, {568.0f, 210.0f},
                       sf::Color(18, 31, 42, 236), sf::Color(238, 200, 110));
        drawText(window, "MORNING RUSH", {240.0f, 180.0f}, 28, sf::Color(255, 235, 170));
        drawText(window, "DOUBLE JUMP: press W / Up / Space again in midair",
                 {226.0f, 244.0f}, 14, sf::Color(220, 230, 235));
        drawText(window, "ENTER  start sprint", {370.0f, 330.0f}, 16, sf::Color(245, 240, 215));
    } else if (game.getPhase() == MorningRushPhase::FinalResult) {
        drawFinal(window);
    }
}

// 鈹€鈹€ helpers 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

void MorningRushRenderer::drawPixelFrame(sf::RenderWindow& window, sf::Vector2f position,
                                         sf::Vector2f size, sf::Color fill, sf::Color outline) const {
    sf::RectangleShape shadow(size + sf::Vector2f(8.0f, 8.0f));
    shadow.setPosition(position + sf::Vector2f(8.0f, 8.0f));
    shadow.setFillColor(sf::Color(0, 0, 0, 120));
    window.draw(shadow);
    sf::RectangleShape border(size);
    border.setPosition(position);
    border.setFillColor(outline);
    window.draw(border);
    sf::RectangleShape inner(size - sf::Vector2f(8.0f, 8.0f));
    inner.setPosition(position + sf::Vector2f(4.0f, 4.0f));
    inner.setFillColor(fill);
    window.draw(inner);
}

void MorningRushRenderer::drawPixelBlock(sf::RenderWindow& window, sf::Vector2f position,
                                         sf::Vector2f size, sf::Color color) const {
    sf::RectangleShape block(size);
    block.setPosition(position);
    block.setFillColor(color);
    window.draw(block);
}

void MorningRushRenderer::drawTile(sf::RenderWindow& window, const sf::Texture& texture,
                                   sf::IntRect rect, sf::Vector2f position, float scale) const {
    sf::Sprite sprite(texture);
    sprite.setTextureRect(rect);
    sprite.setPosition(position);
    sprite.setScale({scale, scale});
    window.draw(sprite);
}

void MorningRushRenderer::drawSchoolTile(sf::RenderWindow& window, int col, int row,
                                         sf::Vector2f position, float scale) const {
    if (!schoolTilesetLoaded) return;
    drawTile(window, schoolTilesetTexture, sf::IntRect({col * 48, row * 48}, {48, 48}), position, scale);
}

// 鈹€鈹€ scene 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

void MorningRushRenderer::drawScene(sf::RenderWindow& window) const {
    drawBackground(window);
    drawGround(window);
    drawPlatforms(window);
    drawCampusDecor(window);
    drawPickups(window);
    drawFinishGate(window);

    for (const auto& ob : game.getObstacles()) {
        const float sx = game.worldToScreenX(ob.x);
        if (sx > -140.0f && sx < 1030.0f) drawObstacle(window, ob);
    }

    drawRunner(window);
    drawLuckBanner(window);

    sf::RectangleShape shade({kRenderWidth, kRenderHeight});
    shade.setFillColor(sf::Color(8, 12, 18, 48));
    window.draw(shade);
}

void MorningRushRenderer::drawBackground(sf::RenderWindow& window) const {
    auto zone = game.getCurrentZone();
    const float dist = game.getDistance();

    if (campusBackgroundLoaded) {
        const sf::Texture& bg = dist < game.getTargetDistance() * 0.52f
            ? campusGateBackgroundTexture
            : campusPathBackgroundTexture;
        const auto size = bg.getSize();
        const float scale = std::max(kRenderWidth / static_cast<float>(size.x),
                                     kRenderHeight / static_cast<float>(size.y));
        const float drawW = static_cast<float>(size.x) * scale;
        const float drawH = static_cast<float>(size.y) * scale;
        const float maxShift = std::max(0.0f, drawW - kRenderWidth);
        const float routePct = game.getTargetDistance() > 1.0f
            ? std::clamp(dist / game.getTargetDistance(), 0.0f, 1.0f)
            : 0.0f;
        const float shift = maxShift * routePct;

        sf::Sprite backdrop(bg);
        backdrop.setScale({scale, scale});
        backdrop.setPosition({-shift, kRenderHeight - drawH});
        window.draw(backdrop);

        sf::RectangleShape shade({kRenderWidth, kRenderHeight});
        shade.setFillColor(sf::Color(255, 255, 255, 18));
        window.draw(shade);
        return;
    }

    sf::Color skyColor;
    sf::Color farColor, midColor;
    switch (zone) {
        case TerrainZone::Dormitory:
            skyColor = {52, 48, 68};   farColor = {72, 62, 88};   midColor = {88, 76, 100};
            break;
        case TerrainZone::Campus:
            skyColor = {74, 132, 170}; farColor = {100, 160, 190}; midColor = {68, 140, 80};
            break;
        case TerrainZone::Classroom:
            skyColor = {58, 62, 72};   farColor = {78, 82, 92};   midColor = {90, 88, 84};
            break;
    }

    sf::RectangleShape sky({kRenderWidth, kRenderHeight});
    sky.setFillColor(skyColor);
    window.draw(sky);

    if (adventureBackgroundLoaded) {
        const sf::Texture& bg = (zone == TerrainZone::Campus)
            ? adventureGreenBackgroundTexture
            : adventureBlueBackgroundTexture;
        const auto size = bg.getSize();
        const float scale = 7.0f;
        const float tileW = static_cast<float>(size.x) * scale;
        const float tileH = static_cast<float>(size.y) * scale;
        const float shift = std::fmod(dist * 0.12f, tileW);
        for (float y = 0.0f; y < 430.0f; y += tileH) {
            for (float x = -tileW; x < kRenderWidth + tileW; x += tileW) {
                sf::Sprite bgSprite(bg);
                bgSprite.setPosition({x - shift, y});
                bgSprite.setScale({scale, scale});
                bgSprite.setColor(sf::Color(255, 255, 255, 150));
                window.draw(bgSprite);
            }
        }

        const float hillShift = std::fmod(dist * 0.22f, 160.0f);
        for (int i = -1; i < 8; ++i) {
            const float x = i * 160.0f - hillShift;
            drawPixelBlock(window, {x, 268.0f}, {120.0f, 92.0f}, sf::Color(34, 57, 72, 120));
            drawPixelBlock(window, {x + 36.0f, 236.0f}, {56.0f, 124.0f}, sf::Color(40, 64, 76, 120));
        }
        return;
    }

    if (assetsLoaded) {
        // 杩滄櫙
        const float farShift = std::fmod(dist * 0.18f, 72.0f);
        for (int i = -1; i < 15; ++i) {
            const float x = i * 72.0f - farShift;
            drawTile(window, backgroundTexture, sf::IntRect({0, 0}, {18, 18}), {x, 130.0f}, 4.0f);
            drawTile(window, backgroundTexture, sf::IntRect({36, 0}, {18, 18}), {x + 26.0f, 168.0f}, 4.0f);
        }
        // 涓櫙
        const float midShift = std::fmod(dist * 0.38f, 72.0f);
        for (int i = -1; i < 18; ++i) {
            const float x = i * 64.0f - midShift;
            drawTile(window, tilemapTexture, sf::IntRect({18 * 4, 18 * 6}, {18, 18}), {x, 318.0f}, 3.0f);
            drawTile(window, tilemapTexture, sf::IntRect({18 * 5, 18 * 6}, {18, 18}), {x + 28.0f, 318.0f}, 3.0f);
        }
    } else {
        for (int i = -1; i < 12; ++i) {
            const float x = i * 96.0f - std::fmod(dist * 0.18f, 96.0f);
            drawPixelBlock(window, {x, 174.0f}, {72.0f, 170.0f}, farColor);
            drawPixelBlock(window, {x + 12.0f, 190.0f}, {48.0f, 18.0f}, midColor);
        }
    }
}

void MorningRushRenderer::drawGround(sf::RenderWindow& window) const {
    auto zone = game.getCurrentZone();
    const float dist = game.getDistance();

    if (campusBackgroundLoaded && greenZoneTilesLoaded) {
        for (const auto& solid : game.getSolidRects()) {
            if (solid.name.find("GroundSegment") == std::string::npos) continue;
            const float segmentX = game.worldToScreenX(solid.x);
            const float segmentRight = segmentX + solid.width;
            if (segmentRight < -32.0f || segmentX > kRenderWidth + 32.0f) continue;

            const float skip = std::max(0.0f, std::floor((-segmentX) / 32.0f) * 32.0f);
            for (float x = segmentX + skip; x < segmentRight && x < kRenderWidth + 32.0f; x += 32.0f) {
                sf::Sprite top(greenGroundTopTexture);
                top.setPosition({x, 420.0f});
                window.draw(top);
                for (int row = 1; row < 4; ++row) {
                    sf::Sprite body(greenGroundBodyTexture);
                    body.setPosition({x, 420.0f + row * 32.0f});
                    window.draw(body);
                }
            }
        }
        return;
    }

    sf::Color groundTop, groundBody;
    switch (zone) {
        case TerrainZone::Dormitory:
            groundTop = {148, 108, 68};  groundBody = {120, 84, 52};
            break;
        case TerrainZone::Campus:
            groundTop = {100, 168, 72};  groundBody = {76, 130, 56};
            break;
        case TerrainZone::Classroom:
            groundTop = {140, 138, 128}; groundBody = {110, 108, 100};
            break;
    }

    // 鍦伴潰涓讳綋
    if (schoolTilesetLoaded) {
        const float tile = 48.0f;
        const float roadShift = std::fmod(dist, tile);
        drawPixelBlock(window, {0.0f, 420.0f}, {kRenderWidth, 124.0f}, groundBody);
        drawPixelBlock(window, {0.0f, 420.0f}, {kRenderWidth, 5.0f}, groundTop);
        for (int i = -1; i < 23; ++i) {
            const float x = i * tile - roadShift;
            sf::Color tileColor = (i % 2 == 0)
                ? sf::Color(172, 188, 188, 210)
                : sf::Color(142, 164, 172, 210);
            if (zone == TerrainZone::Dormitory) {
                tileColor = (i % 2 == 0) ? sf::Color(164, 133, 108) : sf::Color(138, 105, 86);
            } else if (zone == TerrainZone::Classroom) {
                tileColor = (i % 2 == 0) ? sf::Color(162, 166, 174) : sf::Color(132, 138, 148);
            }
            drawPixelBlock(window, {x, 426.0f}, {46.0f, 42.0f}, tileColor);
            drawPixelBlock(window, {x, 468.0f}, {46.0f, 38.0f},
                           sf::Color(tileColor.r - 22, tileColor.g - 22, tileColor.b - 22));
            drawPixelBlock(window, {x, 506.0f}, {46.0f, 38.0f},
                           sf::Color(tileColor.r - 34, tileColor.g - 34, tileColor.b - 34));
        }
        const float lineShift = std::fmod(dist * 1.6f, 96.0f);
        for (int i = -1; i < 13; ++i) {
            const float x = i * 96.0f - lineShift;
            drawPixelBlock(window, {x, 415.0f}, {62.0f, 4.0f}, sf::Color(245, 224, 136, 190));
        }
        return;
    }

    if (adventureTerrainLoaded) {
        const float tile = 48.0f;
        const float roadShift = std::fmod(dist, tile);
        for (int i = -1; i < 23; ++i) {
            const float x = i * tile - roadShift;
            drawTile(window, adventureTerrainTexture, sf::IntRect({16, 0}, {16, 16}), {x, 420.0f}, 3.0f);
            drawTile(window, adventureTerrainTexture, sf::IntRect({16, 16}, {16, 16}), {x, 468.0f}, 3.0f);
            drawTile(window, adventureTerrainTexture, sf::IntRect({16, 32}, {16, 16}), {x, 516.0f}, 3.0f);
        }

        const float trimShift = std::fmod(dist * 1.35f, 64.0f);
        for (int i = -1; i < 18; ++i) {
            const float x = i * 64.0f - trimShift;
            drawPixelBlock(window, {x, 414.0f}, {40.0f, 5.0f}, sf::Color(42, 34, 42, 190));
        }
        return;
    }

    drawPixelBlock(window, {0.0f, 420.0f}, {kRenderWidth, 120.0f}, groundBody);
    drawPixelBlock(window, {0.0f, 420.0f}, {kRenderWidth, 6.0f}, groundTop);

    // 鍦伴潰绾圭悊婊氬姩
    const float roadShift = std::fmod(dist * 1.0f, 54.0f);
    if (assetsLoaded) {
        for (int i = -1; i < 22; ++i) {
            const float x = i * 54.0f - roadShift;
            drawTile(window, tilemapTexture, sf::IntRect({0, 0}, {18, 18}), {x, 420.0f}, 3.0f);
            drawTile(window, tilemapTexture, sf::IntRect({18, 0}, {18, 18}), {x, 474.0f}, 3.0f);
        }
    } else {
        for (int i = -1; i < 20; ++i) {
            const float x = i * 64.0f - std::fmod(dist, 64.0f);
            drawPixelBlock(window, {x, 426.0f}, {62.0f, 86.0f}, sf::Color(groundBody.r - 10, groundBody.g - 10, groundBody.b - 10));
        }
    }
}

void MorningRushRenderer::drawPlatforms(sf::RenderWindow& window) const {
    auto zone = game.getCurrentZone();
    sf::Color platTop, platBody;
    switch (zone) {
        case TerrainZone::Dormitory:
            platTop = {168, 128, 78};  platBody = {136, 100, 60};
            break;
        case TerrainZone::Campus:
            platTop = {120, 180, 90};  platBody = {90, 144, 64};
            break;
        case TerrainZone::Classroom:
            platTop = {160, 156, 148}; platBody = {128, 124, 116};
            break;
    }

    if (greenZoneObjectsLoaded) {
        for (const auto& slope : game.getSlopeRects()) {
            const float sx = game.worldToScreenX(slope.x);
            if (sx < -220.0f || sx > 1060.0f) continue;
            if (slope.name.find("HalfPipeRight") != std::string::npos) continue;
            if (slope.name.find("HalfPipeLeft") != std::string::npos) {
                const auto textureSize = greenHalfPipeTexture.getSize();
                sf::Sprite halfPipe(greenHalfPipeTexture);
                halfPipe.setPosition({sx, slope.bottomY - slope.height});
                halfPipe.setScale({slope.width * 2.0f / static_cast<float>(textureSize.x),
                                   slope.height / static_cast<float>(textureSize.y)});
                window.draw(halfPipe);
                continue;
            }
            const sf::Texture& texture = slope.risesRight
                ? greenRampLeftTexture
                : greenRampRightTexture;
            const auto textureSize = texture.getSize();
            sf::Sprite ramp(texture);
            ramp.setPosition({sx, slope.bottomY - slope.height});
            ramp.setScale({slope.width / static_cast<float>(textureSize.x),
                           slope.height / static_cast<float>(textureSize.y)});
            window.draw(ramp);
        }
    }

    for (const auto& plat : game.getPlatforms()) {
        const float sx = game.worldToScreenX(plat.x);
        if (sx > -200.0f && sx < 1060.0f) {
            const sf::Texture* rampTexture = nullptr;
            if (greenZoneObjectsLoaded && plat.name.find("RampLeft") != std::string::npos) {
                rampTexture = &greenRampLeftTexture;
            } else if (greenZoneObjectsLoaded && plat.name.find("RampRight") != std::string::npos) {
                rampTexture = &greenRampRightTexture;
            } else if (greenZoneObjectsLoaded && plat.name.find("HalfPipe") != std::string::npos) {
                rampTexture = &greenHalfPipeTexture;
            }
            if (rampTexture != nullptr) {
                const auto textureSize = rampTexture->getSize();
                const float targetHeight = 420.0f - plat.y;
                sf::Sprite ramp(*rampTexture);
                ramp.setPosition({sx, plat.y});
                ramp.setScale({plat.width / static_cast<float>(textureSize.x),
                               targetHeight / static_cast<float>(textureSize.y)});
                window.draw(ramp);
                continue;
            }
            if (greenZoneTilesLoaded) {
                for (float x = sx; x < sx + plat.width; x += 32.0f) {
                    sf::Sprite tile(greenPlatformTexture);
                    tile.setPosition({x, plat.y});
                    window.draw(tile);
                }
                continue;
            }
            if (schoolTilesetLoaded) {
                sf::Color top = platTop;
                sf::Color body = platBody;
                if (plat.y < 320.0f) {
                    top = sf::Color(196, 174, 118);
                    body = sf::Color(134, 116, 92);
                } else if (plat.y > 365.0f) {
                    top = sf::Color(184, 142, 94);
                    body = sf::Color(130, 92, 68);
                }
                drawPixelBlock(window, {sx, plat.y}, {plat.width, plat.height}, body);
                drawPixelBlock(window, {sx, plat.y - 3.0f}, {plat.width, 5.0f}, top);
                drawPixelBlock(window, {sx + 6.0f, plat.y + 8.0f},
                               {std::max(0.0f, plat.width - 12.0f), 3.0f},
                               sf::Color(255, 236, 164, 120));
                for (float px = sx; px < sx + plat.width - 8.0f; px += 48.0f) {
                    if (plat.y > 340.0f) {
                        drawSchoolTile(window, 0, 5, {px, plat.y - 42.0f}, 1.0f);
                    } else {
                        drawSchoolTile(window, 5, 1, {px, plat.y - 50.0f}, 1.0f);
                    }
                }
                drawPixelBlock(window, {sx + 6.0f, plat.y + plat.height},
                               {std::max(0.0f, plat.width - 12.0f), 4.0f},
                               sf::Color(0, 0, 0, 60));
                continue;
            }

            if (adventureTerrainLoaded) {
                const float tile = 48.0f;
                const int tileCount = std::max(1, static_cast<int>(std::floor(plat.width / tile)));
                drawPixelBlock(window, {sx, plat.y}, {plat.width, plat.height}, sf::Color(22, 23, 37));
                for (int i = 0; i < tileCount; ++i) {
                    const float x = sx + i * tile;
                    drawTile(window, adventureTerrainTexture, sf::IntRect({16, 0}, {16, 16}), {x, plat.y}, 3.0f);
                }
                const float remainder = plat.width - tileCount * tile;
                if (remainder > 2.0f) {
                    drawPixelBlock(window, {sx + tileCount * tile, plat.y}, {remainder, plat.height},
                                   sf::Color(83, 93, 108));
                    drawPixelBlock(window, {sx + tileCount * tile + 4.0f, plat.y + 10.0f},
                                   {std::max(0.0f, remainder - 8.0f), 5.0f}, sf::Color(185, 196, 207));
                }
                drawPixelBlock(window, {sx, plat.y - 2.0f}, {plat.width, 3.0f},
                               sf::Color(190, 206, 214));
                drawPixelBlock(window, {sx + 4.0f, plat.y + plat.height}, {plat.width - 8.0f, 4.0f},
                               sf::Color(0, 0, 0, 65));
                continue;
            }

            drawPixelBlock(window, {sx, plat.y}, {plat.width, plat.height}, platBody);
            drawPixelBlock(window, {sx, plat.y}, {plat.width, 4.0f}, platTop);
            drawPixelBlock(window, {sx + 6.0f, plat.y + 7.0f}, {plat.width - 12.0f, 3.0f},
                           sf::Color(245, 230, 170, 75));
            drawPixelBlock(window, {sx + 6.0f, plat.y + plat.height - 6.0f}, {plat.width - 12.0f, 4.0f},
                           sf::Color(42, 32, 24, 130));
            drawPixelBlock(window, {sx + 8.0f, plat.y + plat.height - 2.0f}, {8.0f, 12.0f},
                           sf::Color(44, 32, 24, 120));
            drawPixelBlock(window, {sx + plat.width - 16.0f, plat.y + plat.height - 2.0f}, {8.0f, 12.0f},
                           sf::Color(44, 32, 24, 120));
            // 骞冲彴搴曢儴闃村奖
            drawPixelBlock(window, {sx + 4.0f, plat.y + plat.height}, {plat.width - 8.0f, 3.0f},
                           sf::Color(0, 0, 0, 40));
        }
    }
}

void MorningRushRenderer::drawCampusDecor(sf::RenderWindow& window) const {
    if (campusBackgroundLoaded) return;

    struct Prop {
        float x;
        float y;
        int col;
        int row;
        float scale;
    };

    const Prop props[] = {
        {70.0f, 312.0f, 0, 1, 1.7f},
        {260.0f, 290.0f, 3, 0, 1.35f},
        {760.0f, 276.0f, 5, 1, 1.35f},
        {980.0f, 344.0f, 0, 9, 1.25f},
        {1260.0f, 226.0f, 2, 2, 2.2f},
        {1538.0f, 286.0f, 0, 10, 1.15f},
        {1746.0f, 318.0f, 1, 8, 1.35f},
        {2030.0f, 252.0f, 5, 1, 1.55f},
        {2255.0f, 284.0f, 2, 3, 1.9f},
        {2485.0f, 284.0f, 6, 1, 1.35f},
        {2772.0f, 308.0f, 1, 9, 1.15f},
        {3020.0f, 250.0f, 6, 5, 1.45f},
        {3270.0f, 300.0f, 0, 5, 1.35f},
        {3524.0f, 246.0f, 3, 0, 1.45f},
        {3838.0f, 276.0f, 2, 2, 1.95f},
        {4144.0f, 318.0f, 0, 8, 1.2f},
        {4418.0f, 282.0f, 0, 1, 1.65f}
    };

    for (const auto& prop : props) {
        const float sx = game.worldToScreenX(prop.x);
        if (sx < -120.0f || sx > 1040.0f) continue;
        if (schoolTilesetLoaded) {
            drawSchoolTile(window, prop.col, prop.row, {sx, prop.y}, prop.scale);
        } else {
            drawPixelFrame(window, {sx, prop.y}, {48.0f * prop.scale, 48.0f * prop.scale},
                           sf::Color(92, 110, 128), sf::Color(34, 38, 48));
        }
    }

    const float dist = game.getDistance();
    const float bannerShift = std::fmod(dist * 0.5f, 640.0f);
    for (int i = -1; i < 9; ++i) {
        const float x = i * 640.0f - bannerShift;
        drawPixelBlock(window, {x + 80.0f, 118.0f}, {118.0f, 42.0f}, sf::Color(246, 232, 154, 150));
        drawPixelBlock(window, {x + 88.0f, 126.0f}, {102.0f, 6.0f}, sf::Color(60, 83, 112, 160));
        drawPixelBlock(window, {x + 88.0f, 140.0f}, {74.0f, 5.0f}, sf::Color(105, 150, 98, 160));
        drawPixelBlock(window, {x + 88.0f, 152.0f}, {90.0f, 5.0f}, sf::Color(196, 112, 96, 150));
    }
}

void MorningRushRenderer::drawFinishGate(sf::RenderWindow& window) const {
    const float sx = game.worldToScreenX(game.getTargetDistance());
    if (sx < -120.0f || sx > 1040.0f) return;

    const float ground = 420.0f;
    if (adventureEndLoaded) {
        const auto size = adventureEndTexture.getSize();
        const int frameW = 64;
        const int frameH = 64;
        const int frames = std::max(1, static_cast<int>(size.x) / frameW);
        const int frame = static_cast<int>(game.getPulse() * 8.0f) % frames;
        sf::Sprite endSprite(adventureEndTexture);
        endSprite.setTextureRect(sf::IntRect({frame * frameW, 0}, {frameW, frameH}));
        endSprite.setOrigin({32.0f, 58.0f});
        endSprite.setPosition({sx + 20.0f, ground + 2.0f});
        endSprite.setScale({1.55f, 1.55f});
        window.draw(endSprite);
    } else {
        drawPixelFrame(window, {sx, ground - 92.0f}, {46.0f, 92.0f},
                       sf::Color(40, 66, 70), sf::Color(238, 200, 110));
        drawText(window, "CLASS", {sx - 2.0f, ground - 118.0f}, 12, sf::Color(255, 235, 170));
    }

    drawPixelBlock(window, {sx - 16.0f, ground - 4.0f}, {92.0f, 5.0f}, sf::Color(255, 231, 111, 210));
}

void MorningRushRenderer::drawPickups(sf::RenderWindow& window) const {
    const float bob = std::sin(game.getPulse() * 3.5f) * 3.0f;

    for (const auto& pk : game.getPickups()) {
        if (pk.collected) continue;
        const float sx = game.worldToScreenX(pk.x);
        if (sx < -130.0f || sx > 1030.0f) continue;
        const float pickupBottom = pk.y + pk.height;

        drawPixelBlock(window, {sx + 8.0f, pickupBottom - 5.0f},
                       {std::max(20.0f, pk.width - 16.0f), 5.0f},
                       sf::Color(12, 18, 24, 100));
        if (bicyclePickupLoaded) {
            const auto textureSize = bicyclePickupTexture.getSize();
            const float scale = pk.width / static_cast<float>(textureSize.x);
            sf::Sprite bicycle(bicyclePickupTexture);
            bicycle.setOrigin({textureSize.x * 0.5f, static_cast<float>(textureSize.y)});
            bicycle.setPosition({sx + pk.width * 0.5f, pickupBottom + bob});
            bicycle.setScale({scale, scale});
            window.draw(bicycle);
        } else {
            sf::CircleShape rearWheel(20.0f);
            rearWheel.setPosition({sx + 5.0f, pickupBottom - 40.0f + bob});
            rearWheel.setFillColor(sf::Color::Transparent);
            rearWheel.setOutlineThickness(4.0f);
            rearWheel.setOutlineColor(sf::Color(190, 215, 230));
            sf::CircleShape frontWheel = rearWheel;
            frontWheel.setPosition({sx + 62.0f, pickupBottom - 40.0f + bob});
            window.draw(rearWheel);
            window.draw(frontWheel);
            drawPixelBlock(window, {sx + 25.0f, pickupBottom - 48.0f + bob}, {45.0f, 5.0f},
                           sf::Color(54, 112, 170));
        }
    }
}

// 鈹€鈹€ HUD 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

void MorningRushRenderer::drawHud(sf::RenderWindow& window) const {
    drawPixelFrame(window, {24.0f, 22.0f}, {912.0f, 72.0f}, sf::Color(18, 31, 42, 228), sf::Color(8, 10, 14));

    // 鍖哄煙鏍囩
    std::string zoneName;
    switch (game.getCurrentZone()) {
        case TerrainZone::Dormitory: zoneName = "DORM";   break;
        case TerrainZone::Campus:    zoneName = "CAMPUS"; break;
        case TerrainZone::Classroom: zoneName = "SCHOOL"; break;
    }
    drawText(window, zoneName, {44.0f, 34.0f}, 14, sf::Color(180, 200, 220));
    drawText(window, "LEVEL " + std::to_string(game.getStageIndex()),
             {132.0f, 34.0f}, 12, sf::Color(112, 224, 152));
    drawText(window, "MORNING RUSH", {44.0f, 54.0f}, 16, sf::Color(255, 235, 170));

    // 鏃堕棿
    std::ostringstream time;
    time << "TIME " << static_cast<int>(std::ceil(std::max(0.0f, game.getTimeLeft()))) << "s";
    drawText(window, time.str(), {742.0f, 36.0f}, 18,
             game.getTimeLeft() < 8.0f ? sf::Color(255, 120, 110) : sf::Color(230, 240, 245));

    const float pct = std::clamp(game.getDistance() / game.getTargetDistance(), 0.0f, 1.0f);
    drawPixelFrame(window, {230.0f, 42.0f}, {430.0f, 24.0f}, sf::Color(40, 52, 62), sf::Color(8, 8, 10));
    drawPixelBlock(window, {238.0f, 50.0f}, {414.0f * pct, 8.0f}, sf::Color(78, 210, 130));

    const float zoneA = std::clamp(0.34f * game.getTargetDistance() / game.getTargetDistance(), 0.0f, 1.0f);
    const float zoneB = std::clamp(0.78f * game.getTargetDistance() / game.getTargetDistance(), 0.0f, 1.0f);
    drawPixelBlock(window, {238.0f + 414.0f * zoneA, 48.0f}, {2.0f, 12.0f}, sf::Color(200, 200, 200, 120));
    drawPixelBlock(window, {238.0f + 414.0f * zoneB, 48.0f}, {2.0f, 12.0f}, sf::Color(200, 200, 200, 120));

    std::ostringstream info;
    info << "Hits " << game.getCollisionCount();
    if (game.getComboCount() > 1) info << "  Combo x" << game.getComboCount();
    if (game.isBurstRunning()) info << "  LUNGE";
    const float spdMul = std::abs(game.getRunSpeed()) / 120.0f;
    if (spdMul > 1.05f || spdMul < 0.95f) {
        info << "  Spd x" << std::fixed << std::setprecision(1) << spdMul;
    }
    drawText(window, info.str(), {44.0f, 72.0f}, 11, sf::Color(190, 210, 220));

    const float staminaPct = game.getMaxStamina() > 0.0f
        ? std::clamp(game.getStamina() / game.getMaxStamina(), 0.0f, 1.0f)
        : 0.0f;
    drawPixelBlock(window, {682.0f, 54.0f}, {112.0f, 8.0f}, sf::Color(35, 46, 58));
    drawPixelBlock(window, {684.0f, 56.0f}, {108.0f * staminaPct, 4.0f},
                   game.isBurstRunning() ? sf::Color(255, 210, 86) : sf::Color(92, 190, 220));
    drawText(window, "STAMINA", {682.0f, 36.0f}, 10, sf::Color(160, 180, 190));

    drawText(window, game.hasBicycle() ? "K READY" : "K LOCKED",
             {818.0f, 72.0f}, 10,
             game.hasBicycle() ? sf::Color(112, 224, 152) : sf::Color(190, 150, 140));
}

// 鈹€鈹€ runner 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€


void MorningRushRenderer::drawRunner(sf::RenderWindow& window) const {
    const float ground = 420.0f;
    const float lift = game.getRunnerLift();
    const float footY = game.getRunnerTopY() + game.getRunnerHeight();
    const bool introIdle = game.getPhase() == MorningRushPhase::Intro;
    const bool duck = !introIdle && game.isDucking();
    const bool slide = game.isSliding();
    const bool flash = game.getInvulnerableTimer() > 0.0f
        && static_cast<int>(game.getPulse() * 18.0f) % 2 == 0;
    const RushAction cue = (!introIdle && game.getActionCueTimer() > 0.0f) ? game.getActionCue() : RushAction::None;
    const RushAction visualAction = cue != RushAction::None ? cue : RushAction::None;
    const sf::Vector2f runnerFoot(game.getRunnerScreenX() + 34.0f, footY);

    if (!studentActionLoaded && !studentActionDirectionalLoaded) {
        const float h = slide ? 30.0f : (duck ? 40.0f : 74.0f);
        const float w = slide ? 80.0f : 54.0f;
        drawPixelFrame(window, {game.getRunnerScreenX(), footY - h}, {w, h},
                       sf::Color(42, 54, 68), sf::Color(8, 8, 10));
        drawActionEffect(window, visualAction, runnerFoot, lift);
        return;
    }

    constexpr int frameWidth = 360;
    constexpr int frameHeight = 260;
    enum class StudentRow { Idle = 0, Walk = 1, Jump = 2, Burst = 3, Crawl = 4, Hit = 5 };

    StudentRow row = StudentRow::Idle;
    int frameCount = 1;
    float fps = 1.0f;
    const float runSpeed = std::abs(game.getRunSpeed());
    const float runAnimFactor = std::clamp(runSpeed / 165.0f, 0.0f, 1.55f);
    sf::Vector2f position(game.getRunnerScreenX() + 42.0f + game.getBurstLungeOffset(),
                          footY + 7.0f);
    sf::Vector2f scale(0.46f, 0.46f);

    if (introIdle) {
        row = StudentRow::Idle;
        frameCount = 1;
        fps = 1.0f;
    } else if (game.isHitStunned()) {
        row = StudentRow::Hit;
        frameCount = 8;
        fps = 1.0f;
    } else if (duck || visualAction == RushAction::Squeeze) {
        row = StudentRow::Crawl;
        frameCount = 9;
        fps = 11.0f;
        position.y += 4.0f;
    } else if (visualAction == RushAction::BurstRun || game.isBurstRunning()) {
        row = StudentRow::Burst;
        frameCount = 8;
        fps = 15.0f;
        scale = {0.60f, 0.60f};
        position.x += game.isFacingRight() ? 16.0f : -16.0f;
    } else if (!game.isGrounded() || visualAction == RushAction::HurdleJump) {
        row = StudentRow::Jump;
        frameCount = 9;
        fps = 10.0f;
        position.y -= 5.0f;
    } else if (runSpeed > 2.0f && !game.isSlopeSliding()) {
        row = StudentRow::Walk;
        frameCount = 8;
        fps = 10.0f + runAnimFactor * 5.0f;
    }

    int frame = static_cast<int>(game.getPulse() * fps) % std::max(1, frameCount);
    if (row == StudentRow::Hit) {
        constexpr float hitDuration = 0.86f;
        const float progress = std::clamp(1.0f - game.getHitStunTimer() / hitDuration, 0.0f, 1.0f);
        frame = std::min(frameCount - 1, static_cast<int>(progress * frameCount));
    } else if (cue != RushAction::None) {
        constexpr float duration = 0.72f;
        const float progress = std::clamp(1.0f - game.getActionCueTimer() / duration, 0.0f, 1.0f);
        frame = std::min(frameCount - 1, static_cast<int>(progress * frameCount));
    }

    const sf::Texture& actionTexture = studentActionDirectionalLoaded
        ? (game.isFacingRight() ? studentActionRightTexture : studentActionLeftTexture)
        : studentActionTexture;
    sf::Sprite sprite(actionTexture);
    sprite.setTextureRect(sf::IntRect({frame * frameWidth, static_cast<int>(row) * frameHeight},
                                      {frameWidth, frameHeight}));
    sprite.setOrigin({frameWidth * 0.5f, frameHeight - 10.0f});
    sprite.setPosition(position);
    sprite.setScale(scale);
    sprite.setColor(flash && row != StudentRow::Hit
        ? sf::Color(255, 245, 190)
        : sf::Color::White);
    window.draw(sprite);

    if (slide) {
        drawPixelBlock(window, {game.getRunnerScreenX() - 14.0f, ground - 8.0f},
                       {98.0f, 6.0f}, sf::Color(20, 20, 24, 130));
    }

    drawActionEffect(window, visualAction, runnerFoot, lift);

    if (game.isWallContact()) {
        drawText(window, "WALL!", {game.getRunnerScreenX() - 8.0f, ground - lift - 150.0f},
                 12, sf::Color(255, 230, 120));
    }
}

void MorningRushRenderer::drawActionEffect(sf::RenderWindow& window, RushAction action,
                                           sf::Vector2f runnerFoot, float lift) const {
    (void)window;
    (void)action;
    (void)runnerFoot;
    (void)lift;
}

void MorningRushRenderer::drawObstacle(sf::RenderWindow& window, const MorningRushObstacle& ob) const {
    const float sx = game.worldToScreenX(ob.x);
    const float ground = 420.0f;

    sf::Color main = ob.hit
        ? (ob.destructible ? sf::Color(60, 60, 60, 60) : sf::Color(80, 80, 80, 120))
        : (ob.destructible ? sf::Color(180, 160, 80) : sf::Color(208, 74, 74));
    sf::Color dark = ob.hit
        ? sf::Color(40, 40, 40, 60)
        : (ob.destructible ? sf::Color(120, 100, 40) : sf::Color(26, 22, 28));

    if (ob.name.find("Pit") != std::string::npos) return;

    if (ob.kind == MorningRushObstacleKind::Overhead) {
        const float y = ground - ob.yOffset - ob.height;
        drawPixelBlock(window, {sx, y}, {ob.width, ob.height}, dark);
        drawPixelBlock(window, {sx + 6.0f, y + 6.0f}, {ob.width - 12.0f, ob.height - 12.0f}, main);
        drawPixelBlock(window, {sx + 12.0f, y + 16.0f}, {ob.width - 24.0f, 6.0f},
                       sf::Color(245, 220, 140, ob.hit ? 120 : 255));
        return;
    }

    const float y = ground - ob.height;
    const bool muted = ob.hit;
    const sf::Color softShadow(0, 0, 0, 80);
    const sf::Color ink = muted ? sf::Color(72, 72, 78, 120) : sf::Color(34, 34, 42);
    const sf::Color blue = muted ? sf::Color(70, 104, 150, 120) : sf::Color(70, 116, 178);
    const sf::Color green = muted ? sf::Color(78, 120, 92, 120) : sf::Color(86, 148, 110);
    const sf::Color red = muted ? sf::Color(140, 82, 92, 120) : sf::Color(190, 92, 104);
    const sf::Color yellow = muted ? sf::Color(170, 140, 86, 120) : sf::Color(238, 185, 78);
    const sf::Color wood = muted ? sf::Color(118, 100, 82, 130) : sf::Color(172, 112, 74);

    if (ob.name.find("BoxStack") != std::string::npos && greenZoneObjectsLoaded) {
        const auto textureSize = greenBoxTexture.getSize();
        const float scale = 2.0f;
        for (int row = 0; row < 2; ++row) {
            sf::Sprite box(greenBoxTexture);
            box.setPosition({sx, ground - (row + 1) * textureSize.y * scale});
            box.setScale({scale, scale});
            if (muted) box.setColor(sf::Color(170, 170, 170, 130));
            window.draw(box);
        }
        return;
    }

    const sf::Texture* obstacleTexture = nullptr;
    if (ob.name.find("WetFloorSign") != std::string::npos && wetFloorSignLoaded) {
        obstacleTexture = &wetFloorSignTexture;
    } else if (ob.name.find("TrafficCone") != std::string::npos && trafficConeLoaded) {
        obstacleTexture = &trafficConeTexture;
    } else if (ob.name.find("RecyclingBin") != std::string::npos && recyclingBinLoaded) {
        obstacleTexture = &recyclingBinTexture;
    } else if (ob.name.find("CampusPlanter") != std::string::npos && campusPlanterLoaded) {
        obstacleTexture = &campusPlanterTexture;
    } else if (ob.name.find("CrowdBarrier") != std::string::npos && crowdBarrierLoaded) {
        obstacleTexture = &crowdBarrierTexture;
    } else if (ob.name.find("Box") != std::string::npos && greenZoneObjectsLoaded) {
        obstacleTexture = &greenBoxTexture;
    } else if (ob.name.find("FountainTall") != std::string::npos && greenZoneObjectsLoaded) {
        obstacleTexture = &greenFountainTallTexture;
    } else if (ob.name.find("FountainLow") != std::string::npos && greenZoneObjectsLoaded) {
        obstacleTexture = &greenFountainLowTexture;
    } else if (ob.name.find("FenceLeft") != std::string::npos && greenZoneObjectsLoaded) {
        obstacleTexture = &greenFenceLeftTexture;
    } else if (ob.name.find("FenceRight") != std::string::npos && greenZoneObjectsLoaded) {
        obstacleTexture = &greenFenceRightTexture;
    }
    if (obstacleTexture != nullptr) {
        const sf::Vector2u textureSize = obstacleTexture->getSize();
        const float scale = std::min(ob.width / static_cast<float>(textureSize.x),
                                     ob.height / static_cast<float>(textureSize.y));
        const float renderedWidth = static_cast<float>(textureSize.x) * scale;
        const float renderedHeight = static_cast<float>(textureSize.y) * scale;
        sf::Sprite sprite(*obstacleTexture);
        sprite.setPosition({sx + (ob.width - renderedWidth) * 0.5f, ground - renderedHeight});
        sprite.setScale({scale, scale});
        if (muted) sprite.setColor(sf::Color(170, 170, 170, 130));
        window.draw(sprite);
        return;
    }

    if (ob.rule == MorningRushObstacleRule::Hurdle) {
        drawPixelBlock(window, {sx - 4.0f, ground - 5.0f}, {ob.width + 8.0f, 5.0f}, softShadow);
        if (ob.name.find("Desk") != std::string::npos) {
            drawPixelBlock(window, {sx + 2.0f, y + 8.0f}, {ob.width - 4.0f, 18.0f}, wood);
            drawPixelBlock(window, {sx + 10.0f, y + 26.0f}, {7.0f, ob.height - 26.0f}, ink);
            drawPixelBlock(window, {sx + ob.width - 17.0f, y + 26.0f}, {7.0f, ob.height - 26.0f}, ink);
        } else {
            drawPixelBlock(window, {sx + 4.0f, y + ob.height - 18.0f}, {ob.width - 8.0f, 18.0f}, green);
            for (float px = sx + 12.0f; px < sx + ob.width - 6.0f; px += 20.0f) {
                drawPixelBlock(window, {px, y + 6.0f}, {8.0f, 8.0f}, red);
            }
        }
        return;
    }

    if (ob.rule == MorningRushObstacleRule::Squeeze) {
        drawPixelBlock(window, {sx - 4.0f, ground - 6.0f}, {ob.width + 8.0f, 5.0f}, softShadow);
        for (int i = 0; i < 4; ++i) {
            const float px = sx + 4.0f + i * 22.0f;
            if (px > sx + ob.width - 12.0f) break;
            const float bob = std::sin(game.getPulse() * 5.0f + i) * 2.0f;
            const sf::Color shirt = (i % 3 == 0) ? blue : ((i % 3 == 1) ? green : red);
            drawPixelBlock(window, {px + 5.0f, y + 4.0f + bob}, {13.0f, 13.0f},
                           sf::Color(224, 174, 130, muted ? 140 : 255));
            drawPixelBlock(window, {px + 2.0f, y + 17.0f + bob}, {19.0f, 26.0f}, shirt);
            drawPixelBlock(window, {px + 5.0f, y + 43.0f + bob}, {5.0f, 11.0f}, ink);
            drawPixelBlock(window, {px + 15.0f, y + 43.0f + bob}, {5.0f, 11.0f}, ink);
        }
        return;
    }

    if (ob.rule == MorningRushObstacleRule::WallTurn) {
        drawPixelBlock(window, {sx - 4.0f, ground - 5.0f}, {ob.width + 8.0f, 5.0f}, softShadow);
        drawPixelBlock(window, {sx + 10.0f, y}, {ob.width - 20.0f, ob.height}, sf::Color(88, 106, 126, muted ? 130 : 235));
        for (float py = y + 8.0f; py < y + ob.height - 4.0f; py += 18.0f) {
            drawPixelBlock(window, {sx + 14.0f, py}, {ob.width - 28.0f, 3.0f},
                           sf::Color(206, 216, 226, muted ? 80 : 160));
        }
        return;
    }

    if (ob.rule == MorningRushObstacleRule::BurstRun) {
        drawPixelBlock(window, {sx - 4.0f, ground - 5.0f}, {ob.width + 8.0f, 5.0f}, softShadow);
        drawPixelBlock(window, {sx, y + ob.height - 16.0f}, {ob.width, 16.0f},
                       sf::Color(62, 76, 92, muted ? 110 : 220));
        for (float px = sx + 12.0f; px < sx + ob.width - 18.0f; px += 42.0f) {
            drawPixelBlock(window, {px, y + ob.height - 10.0f}, {18.0f, 3.0f},
                           sf::Color(178, 186, 190, muted ? 80 : 180));
        }
        return;
    }

    if (ob.name.find("Backpack") != std::string::npos) {
        drawPixelBlock(window, {sx - 4.0f, ground - 5.0f}, {ob.width + 8.0f, 5.0f}, softShadow);
        drawPixelBlock(window, {sx + 4.0f, y + 10.0f}, {28.0f, 28.0f}, blue);
        drawPixelBlock(window, {sx + 24.0f, y + 5.0f}, {28.0f, 33.0f}, green);
        drawPixelBlock(window, {sx + 12.0f, y + 18.0f}, {10.0f, 4.0f}, yellow);
        drawPixelBlock(window, {sx + 34.0f, y + 16.0f}, {10.0f, 4.0f}, yellow);
        return;
    }
    if (ob.name.find("Crowd") != std::string::npos || ob.name.find("Student") != std::string::npos) {
        for (int i = 0; i < 3; ++i) {
            const float px = sx + 8.0f + i * 20.0f;
            const float bob = std::sin(game.getPulse() * 5.0f + i) * 2.0f;
            drawPixelBlock(window, {px + 4.0f, y + 4.0f + bob}, {12.0f, 12.0f}, sf::Color(224, 174, 130, muted ? 140 : 255));
            drawPixelBlock(window, {px, y + 18.0f + bob}, {20.0f, 24.0f}, i == 0 ? blue : (i == 1 ? green : red));
            drawPixelBlock(window, {px + 2.0f, y + 42.0f + bob}, {6.0f, 10.0f}, ink);
            drawPixelBlock(window, {px + 12.0f, y + 42.0f + bob}, {6.0f, 10.0f}, ink);
        }
        drawPixelBlock(window, {sx - 4.0f, ground - 6.0f}, {ob.width + 8.0f, 5.0f}, softShadow);
        return;
    }
    if (ob.name.find("Spilled") != std::string::npos || ob.name.find("Papers") != std::string::npos) {
        const sf::Color spill = ob.name.find("Papers") != std::string::npos
            ? sf::Color(245, 238, 210, muted ? 125 : 235)
            : sf::Color(238, 230, 186, muted ? 120 : 235);
        drawPixelBlock(window, {sx, y + ob.height - 12.0f}, {ob.width, 12.0f}, spill);
        drawPixelBlock(window, {sx + 8.0f, y + ob.height - 18.0f}, {ob.width - 18.0f, 5.0f},
                       sf::Color(255, 250, 224, muted ? 120 : 230));
        if (ob.name.find("Papers") != std::string::npos) {
            drawPixelBlock(window, {sx + 14.0f, y + 2.0f}, {18.0f, 10.0f}, spill);
            drawPixelBlock(window, {sx + 44.0f, y + 4.0f}, {22.0f, 9.0f}, spill);
        }
        return;
    }
    if (ob.name.find("Bike") != std::string::npos) {
        drawPixelBlock(window, {sx + 8.0f, y + 26.0f}, {18.0f, 18.0f}, ink);
        drawPixelBlock(window, {sx + 46.0f, y + 26.0f}, {18.0f, 18.0f}, ink);
        drawPixelBlock(window, {sx + 22.0f, y + 18.0f}, {36.0f, 8.0f}, blue);
        drawPixelBlock(window, {sx + 36.0f, y + 8.0f}, {8.0f, 22.0f}, blue);
        drawPixelBlock(window, {sx + 48.0f, y + 4.0f}, {18.0f, 5.0f}, yellow);
        return;
    }
    if (ob.name.find("Bucket") != std::string::npos) {
        drawPixelBlock(window, {sx + 10.0f, y + 12.0f}, {30.0f, 24.0f}, yellow);
        drawPixelBlock(window, {sx + 14.0f, y + 4.0f}, {22.0f, 8.0f}, sf::Color(92, 180, 210, muted ? 130 : 230));
        drawPixelBlock(window, {sx + 34.0f, y - 6.0f}, {6.0f, 42.0f}, ink);
        return;
    }
    if (ob.name.find("Cart") != std::string::npos) {
        drawPixelBlock(window, {sx + 4.0f, y + 8.0f}, {58.0f, 28.0f}, wood);
        drawPixelBlock(window, {sx + 10.0f, y + 14.0f}, {42.0f, 5.0f}, blue);
        drawPixelBlock(window, {sx + 10.0f, y + 22.0f}, {34.0f, 5.0f}, yellow);
        drawPixelBlock(window, {sx + 10.0f, y + 36.0f}, {12.0f, 12.0f}, ink);
        drawPixelBlock(window, {sx + 48.0f, y + 36.0f}, {12.0f, 12.0f}, ink);
        return;
    }
    if (ob.name.find("Locker") != std::string::npos) {
        drawPixelBlock(window, {sx + 5.0f, y}, {ob.width - 10.0f, ob.height}, sf::Color(114, 132, 160, muted ? 130 : 235));
        drawPixelBlock(window, {sx + 11.0f, y + 10.0f}, {ob.width - 22.0f, 5.0f}, sf::Color(210, 220, 230, muted ? 90 : 190));
        drawPixelBlock(window, {sx + 11.0f, y + 26.0f}, {ob.width - 22.0f, 5.0f}, sf::Color(210, 220, 230, muted ? 90 : 190));
        drawPixelBlock(window, {sx + ob.width - 14.0f, y + 34.0f}, {5.0f, 7.0f}, yellow);
        return;
    }
    if (ob.name.find("Desk") != std::string::npos) {
        if (schoolTilesetLoaded) {
            drawSchoolTile(window, 0, 5, {sx, y - 4.0f}, 1.08f);
            drawSchoolTile(window, 1, 5, {sx + 42.0f, y - 4.0f}, 1.08f);
        } else {
            drawPixelBlock(window, {sx, y + 6.0f}, {ob.width, 22.0f}, wood);
            drawPixelBlock(window, {sx + 8.0f, y + 28.0f}, {8.0f, 20.0f}, ink);
            drawPixelBlock(window, {sx + ob.width - 16.0f, y + 28.0f}, {8.0f, 20.0f}, ink);
        }
        return;
    }
    if (game.canStandOnObstacle(ob) && adventureTerrainLoaded) {
        const int tileCount = std::max(1, static_cast<int>(std::ceil(ob.width / 48.0f)));
        for (int i = 0; i < tileCount; ++i) {
            const float x = sx + i * 48.0f;
            drawTile(window, adventureTerrainTexture, sf::IntRect({16, 0}, {16, 16}), {x, y}, 3.0f);
            if (ob.height > 42.0f) {
                drawTile(window, adventureTerrainTexture, sf::IntRect({16, 16}, {16, 16}), {x, y + 48.0f}, 3.0f);
            }
        }
        drawPixelBlock(window, {sx + 3.0f, y - 4.0f}, {ob.width - 6.0f, 5.0f},
                       sf::Color(255, 231, 111, ob.hit ? 90 : 230));
        return;
    }
    if (ob.destructible) {
        drawPixelBlock(window, {sx, y}, {ob.width, ob.height}, dark);
        drawPixelBlock(window, {sx + 4.0f, y + 4.0f}, {ob.width - 8.0f, ob.height - 8.0f}, main);
        // 瑁傜紳鏍囪
        drawPixelBlock(window, {sx + ob.width * 0.3f, y + ob.height * 0.4f}, {2.0f, ob.height * 0.4f},
                       sf::Color(60, 50, 30, ob.hit ? 60 : 200));
        drawPixelBlock(window, {sx + ob.width * 0.6f, y + ob.height * 0.3f}, {2.0f, ob.height * 0.5f},
                       sf::Color(60, 50, 30, ob.hit ? 60 : 200));
        return;
    }

    if (ob.name.find("Puddle") != std::string::npos) {
        drawPixelBlock(window, {sx, y + ob.height - 14.0f}, {ob.width, 14.0f},
                       sf::Color(34, 116, 190, ob.hit ? 120 : 255));
        drawPixelBlock(window, {sx + 10.0f, y + ob.height - 20.0f}, {ob.width - 20.0f, 6.0f},
                       sf::Color(120, 210, 240, ob.hit ? 120 : 255));
    } else if (ob.name.find("Bike") != std::string::npos) {
        drawPixelBlock(window, {sx + 8.0f, y + 26.0f}, {18.0f, 18.0f}, dark);
        drawPixelBlock(window, {sx + 46.0f, y + 26.0f}, {18.0f, 18.0f}, dark);
        drawPixelBlock(window, {sx + 22.0f, y + 18.0f}, {36.0f, 8.0f}, main);
        drawPixelBlock(window, {sx + 36.0f, y + 8.0f}, {8.0f, 22.0f}, main);
    } else if (ob.name.find("Stair") != std::string::npos || ob.name.find("Speed Bump") != std::string::npos) {
        drawPixelBlock(window, {sx, y}, {ob.width, ob.height}, dark);
        drawPixelBlock(window, {sx + 3.0f, y + 3.0f}, {ob.width - 6.0f, ob.height - 3.0f}, main);
        drawPixelBlock(window, {sx + 8.0f, y + 5.0f}, {ob.width - 16.0f, 4.0f},
                       sf::Color(255, 222, 120, ob.hit ? 110 : 230));
    } else {
        drawPixelBlock(window, {sx, y}, {ob.width, ob.height}, dark);
        drawPixelBlock(window, {sx + 6.0f, y + 6.0f}, {ob.width - 12.0f, ob.height - 6.0f}, main);
        drawPixelBlock(window, {sx + 12.0f, y + 14.0f}, {10.0f, 10.0f},
                       sf::Color(255, 235, 180, ob.hit ? 120 : 255));
        if (!ob.hit) {
            drawText(window, "!", {sx + ob.width * 0.5f - 3.0f, y + 4.0f}, 12, sf::Color(255, 245, 190));
        }
    }

    if (game.canStandOnObstacle(ob)) {
        drawPixelBlock(window, {sx + 3.0f, y - 3.0f}, {ob.width - 6.0f, 5.0f},
                       sf::Color(238, 220, 132, ob.hit ? 90 : 230));
        drawPixelBlock(window, {sx + 3.0f, y + 2.0f}, {ob.width - 6.0f, 2.0f},
                       sf::Color(90, 64, 40, ob.hit ? 80 : 180));
    }
}

// 鈹€鈹€ luck banner 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

void MorningRushRenderer::drawLuckBanner(sf::RenderWindow& window) const {
    const auto& event = game.getLuckEvent();
    if (event.displayTimer <= 0.0f) return;

    const sf::Color border = event.success ? sf::Color(96, 220, 130) : sf::Color(238, 190, 90);
    drawPixelFrame(window, {254.0f, 110.0f}, {452.0f, 72.0f}, sf::Color(24, 34, 46, 238), border);
    drawText(window, event.title, {278.0f, 126.0f}, 16, sf::Color(255, 235, 180));
    if (event.d20 > 0) {
        std::ostringstream roll;
        roll << "d20 " << event.d20 << " / DC " << event.dc;
        drawText(window, roll.str(), {588.0f, 126.0f}, 14,
                 event.success ? sf::Color(138, 242, 158) : sf::Color(255, 150, 120));
    }
    drawText(window, event.text, {278.0f, 154.0f}, 13, sf::Color(222, 232, 236));
}

// 鈹€鈹€ final 鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€鈹€

void MorningRushRenderer::drawFinal(sf::RenderWindow& window) const {
    drawPixelFrame(window, {190.0f, 154.0f}, {580.0f, 236.0f},
                   sf::Color(18, 31, 42, 244), sf::Color(238, 200, 110));

    std::string title = "ARRIVED";
    sf::Color titleColor(120, 240, 150);
    if (game.getOutcome() == MorningRushOutcome::GreatSuccess) title = "PERFECT ARRIVAL";
    if (game.getOutcome() == MorningRushOutcome::Failure) {
        title = "LATE ARRIVAL";  titleColor = sf::Color(255, 175, 110);
    }
    if (game.getOutcome() == MorningRushOutcome::CriticalFailure) {
        title = "BELL MISSED";  titleColor = sf::Color(255, 110, 110);
    }

    drawText(window, title, {236.0f, 190.0f}, 25, titleColor);
    drawText(window, game.getResultText(), {236.0f, 246.0f}, 14, sf::Color(232, 238, 230));
    std::ostringstream stats;
    stats << "Distance " << static_cast<int>(game.getDistance()) << "/"
          << static_cast<int>(game.getTargetDistance())
          << "   Collisions " << game.getCollisionCount()
          << "   Combo " << game.getComboCount();
    drawText(window, stats.str(), {236.0f, 298.0f}, 14, sf::Color(190, 212, 230));
    drawPixelFrame(window, {236.0f, 334.0f}, {284.0f, 42.0f}, sf::Color(36, 55, 60), sf::Color(8, 8, 10));
    drawText(window, "ENTER  return to class", {260.0f, 346.0f}, 14, sf::Color(255, 240, 210));
}

void MorningRushRenderer::drawText(sf::RenderWindow& window, const std::string& text,
                                   sf::Vector2f position, unsigned int size, sf::Color color) const {
    sf::Text label = cls::makeText(font, text, size);
    label.setFillColor(color);
    label.setPosition(position);
    window.draw(label);
}
