#include "map/LibraryInterior.h"
#include "utils/AssetPath.h"

#include <string>

namespace {
sf::IntRect rect(int x, int y, int w, int h) {
    return sf::IntRect({x, y}, {w, h});
}

sf::FloatRect hit(float x, float y, float w, float h) {
    return sf::FloatRect({x, y}, {w, h});
}
} // namespace

LibraryInterior::LibraryInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/library.json"));

    mAtlasTexture.loadFromFile(cls::resolveAssetPath(
        "assets/image/scenery/library/libassetpack-tiled.png"));

    buildLibraryLayout();
}

void LibraryInterior::addObject(sf::IntRect source, sf::Vector2f position, sf::Vector2f scale,
                                sf::FloatRect collision) {
    mObjectSprites.push_back(AtlasSprite{source, position, scale});
    obstacles.push_back(collision);
}

void LibraryInterior::drawAtlasSprite(sf::RenderWindow& window, const AtlasSprite& spriteInfo) const {
    if (mAtlasTexture.getSize().x == 0 || mAtlasTexture.getSize().y == 0) return;

    sf::Sprite sprite(mAtlasTexture);
    sprite.setTextureRect(spriteInfo.source);
    sprite.setPosition(spriteInfo.position);
    sprite.setScale(spriteInfo.scale);
    window.draw(sprite);
}

void LibraryInterior::drawPixelFloor(sf::RenderWindow& window) const {
    sf::RectangleShape floor({kRenderWidth, kRenderHeight - 164.0f});
    floor.setPosition({0.0f, 164.0f});
    floor.setFillColor(sf::Color(115, 66, 35));
    window.draw(floor);

    if (mAtlasTexture.getSize().x == 0) return;

    const AtlasSprite floorTile{rect(24, 216, 96, 48), {0.0f, 164.0f}, {1.0f, 1.0f}};
    for (float y = 164.0f; y < kRenderHeight; y += 48.0f) {
        for (float x = 0.0f; x < kRenderWidth; x += 96.0f) {
            drawAtlasSprite(window, AtlasSprite{floorTile.source, {x, y}, floorTile.scale});
        }
    }

    sf::RectangleShape shade({kRenderWidth, kRenderHeight - 164.0f});
    shade.setPosition({0.0f, 164.0f});
    shade.setFillColor(sf::Color(110, 58, 28, 70));
    window.draw(shade);
}

void LibraryInterior::drawBackWall(sf::RenderWindow& window) const {
    sf::RectangleShape wall({kRenderWidth, 184.0f});
    wall.setPosition({0.0f, 0.0f});
    wall.setFillColor(sf::Color(106, 58, 35));
    window.draw(wall);

    if (mAtlasTexture.getSize().x != 0) {
        drawAtlasSprite(window, AtlasSprite{rect(936, 0, 528, 160), {0.0f, 0.0f}, {1.82f, 1.0f}});
    }

    sf::RectangleShape baseboard({kRenderWidth, 18.0f});
    baseboard.setPosition({0.0f, 166.0f});
    baseboard.setFillColor(sf::Color(87, 42, 32));
    window.draw(baseboard);
}

void LibraryInterior::buildLibraryLayout() {
    obstacles.clear();
    mDecorSprites.clear();
    mObjectSprites.clear();
    obstacles.reserve(40);
    mDecorSprites.reserve(16);
    mObjectSprites.reserve(32);

    // Wall edge and room boundary collisions.
    obstacles.push_back(hit(42.0f, 72.0f, 876.0f, 18.0f));
    obstacles.push_back(hit(42.0f, 72.0f, 18.0f, 424.0f));
    obstacles.push_back(hit(900.0f, 72.0f, 18.0f, 424.0f));

    // Left runner carpet, stacked like the reference image, with potted plants on both sides.
    mDecorSprites.push_back(AtlasSprite{rect(745, 192, 71, 153), {66.0f, 58.0f}, {1.0f, 1.0f}});
    mDecorSprites.push_back(AtlasSprite{rect(745, 24, 71, 153), {66.0f, 198.0f}, {1.0f, 1.0f}});
    mDecorSprites.push_back(AtlasSprite{rect(745, 24, 71, 153), {66.0f, 354.0f}, {1.0f, 1.0f}});
    addObject(rect(30, 49, 36, 71), {76.0f, 180.0f}, {0.92f, 0.92f}, hit(82.0f, 204.0f, 30.0f, 40.0f));
    addObject(rect(30, 49, 36, 71), {127.0f, 180.0f}, {0.92f, 0.92f}, hit(133.0f, 204.0f, 30.0f, 40.0f));
    addObject(rect(30, 49, 36, 71), {76.0f, 310.0f}, {0.92f, 0.92f}, hit(82.0f, 334.0f, 30.0f, 40.0f));
    addObject(rect(30, 49, 36, 71), {127.0f, 310.0f}, {0.92f, 0.92f}, hit(133.0f, 334.0f, 30.0f, 40.0f));
    addObject(rect(30, 49, 36, 71), {76.0f, 438.0f}, {0.92f, 0.92f}, hit(82.0f, 462.0f, 30.0f, 40.0f));
    addObject(rect(30, 49, 36, 71), {127.0f, 438.0f}, {0.92f, 0.92f}, hit(133.0f, 462.0f, 30.0f, 40.0f));

    // Top row: shelves, clock, and chest.
    addObject(rect(268, 365, 88, 139), {190.0f, 80.0f}, {1.12f, 1.12f}, hit(198.0f, 116.0f, 88.0f, 118.0f));
    updateInteractionArea("library_shelf_0", hit(198.0f, 116.0f, 88.0f, 118.0f));

    addObject(rect(79, 24, 58, 120), {389.0f, 78.0f}, {1.28f, 1.28f}, hit(410.0f, 108.0f, 68.0f, 128.0f));

    addObject(rect(388, 373, 88, 131), {604.0f, 84.0f}, {1.12f, 1.12f}, hit(612.0f, 116.0f, 88.0f, 118.0f));
    updateInteractionArea("library_shelf_1", hit(612.0f, 116.0f, 88.0f, 118.0f));

    addObject(rect(148, 372, 88, 132), {720.0f, 82.0f}, {1.14f, 1.14f}, hit(728.0f, 116.0f, 86.0f, 116.0f));
    updateInteractionArea("library_shelf_2", hit(728.0f, 116.0f, 86.0f, 116.0f));
    updateInteractionArea("library_shelf_3", hit(728.0f, 116.0f, 86.0f, 116.0f));

    addObject(rect(58, 150, 54, 42), {654.0f, 245.0f}, {0.90f, 0.90f}, hit(660.0f, 263.0f, 48.0f, 28.0f));

    // Study desk cluster.
    addObject(rect(362, 216, 164, 96), {242.0f, 260.0f}, {1.55f, 1.55f}, hit(266.0f, 304.0f, 224.0f, 92.0f));
    updateInteractionArea("library_table", hit(266.0f, 304.0f, 224.0f, 92.0f));
    mDecorSprites.push_back(AtlasSprite{rect(290, 24, 45, 24), {338.0f, 283.0f}, {1.0f, 1.0f}});
    mDecorSprites.push_back(AtlasSprite{rect(296, 75, 33, 21), {430.0f, 276.0f}, {1.1f, 1.1f}});
    mDecorSprites.push_back(AtlasSprite{rect(288, 115, 48, 29), {548.0f, 270.0f}, {0.95f, 0.95f}});
    mDecorSprites.push_back(AtlasSprite{rect(624, 125, 24, 43), {314.0f, 246.0f}, {1.0f, 1.0f}});

    addObject(rect(168, 144, 48, 48), {210.0f, 334.0f}, {0.82f, 0.82f}, hit(220.0f, 352.0f, 32.0f, 28.0f));
    addObject(rect(240, 96, 30, 48), {514.0f, 328.0f}, {0.95f, 0.95f}, hit(526.0f, 354.0f, 28.0f, 32.0f));
    addObject(rect(177, 30, 33, 36), {244.0f, 314.0f}, {0.90f, 0.90f}, hit(252.0f, 324.0f, 22.0f, 20.0f));

    // Rugs and seating area.
    mDecorSprites.push_back(AtlasSprite{rect(456, 24, 168, 72), {640.0f, 292.0f}, {1.28f, 1.16f}});
    mDecorSprites.push_back(AtlasSprite{rect(465, 127, 126, 59), {332.0f, 405.0f}, {1.38f, 1.15f}});

    addObject(rect(672, 39, 48, 57), {622.0f, 386.0f}, {1.05f, 1.05f}, hit(630.0f, 402.0f, 30.0f, 48.0f));
    addObject(rect(672, 100, 48, 68), {736.0f, 386.0f}, {1.05f, 1.05f}, hit(744.0f, 402.0f, 30.0f, 48.0f));
    addObject(rect(541, 229, 191, 107), {620.0f, 448.0f}, {0.95f, 0.95f}, hit(630.0f, 472.0f, 158.0f, 46.0f));

    // Globe, loose boxes, books and plants.
    addObject(rect(146, 228, 92, 108), {334.0f, 390.0f}, {1.05f, 1.05f}, hit(348.0f, 420.0f, 70.0f, 64.0f));
    addObject(rect(177, 30, 33, 36), {172.0f, 288.0f}, {1.00f, 1.00f}, hit(180.0f, 300.0f, 24.0f, 22.0f));
    addObject(rect(168, 129, 57, 57), {184.0f, 372.0f}, {0.96f, 0.96f}, hit(194.0f, 390.0f, 36.0f, 30.0f));
    addObject(rect(177, 30, 33, 36), {310.0f, 474.0f}, {0.92f, 0.92f}, hit(318.0f, 486.0f, 24.0f, 20.0f));
    addObject(rect(168, 88, 58, 30), {148.0f, 476.0f}, {1.05f, 1.05f}, hit(150.0f, 484.0f, 58.0f, 20.0f));

    // Small decorative clutter with collision for readable object presence.
    addObject(rect(240, 27, 31, 45), {540.0f, 382.0f}, {0.75f, 0.75f}, hit(548.0f, 392.0f, 24.0f, 18.0f));
    addObject(rect(360, 26, 24, 22), {560.0f, 430.0f}, {0.75f, 0.75f}, hit(564.0f, 434.0f, 18.0f, 12.0f));
    addObject(rect(30, 24, 36, 42), {836.0f, 288.0f}, {0.75f, 0.75f}, hit(846.0f, 300.0f, 16.0f, 18.0f));
}

void LibraryInterior::render(sf::RenderWindow& window) {
    sf::RectangleShape bg({kRenderWidth, kRenderHeight});
    bg.setFillColor(sf::Color(42, 30, 24));
    window.draw(bg);

    drawBackWall(window);
    drawPixelFloor(window);

    for (const auto& sprite : mDecorSprites) {
        drawAtlasSprite(window, sprite);
    }

    for (const auto& sprite : mObjectSprites) {
        drawAtlasSprite(window, sprite);
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> LibraryInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Library, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
