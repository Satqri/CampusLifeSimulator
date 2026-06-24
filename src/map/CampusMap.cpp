#include "map/CampusMap.h"
#include "utils/AssetPath.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"
#include "core/TimeSystem.h"
#include "entity/Player.h"
#include <filesystem>

static bool loadTextureFromCandidates(sf::Texture& texture, const std::string& relativePath) {
    return texture.loadFromFile(cls::resolveAssetPath(relativePath));
}

CampusMap::CampusMap() {
    outdoorTilesLoaded = loadTextureFromCandidates(
        outdoorTiles, "assets/image/tilesets/pixlab24_topdown_tileset.png");

    loadBuildingTexture(0, "assets/image/buildings/building_dormitory.png");
    loadBuildingTexture(1, "assets/image/buildings/building_gym.png");
    loadBuildingTexture(2, "assets/image/buildings/building_library.png");
    loadBuildingTexture(3, "assets/image/buildings/building_classroom.png");
    loadBuildingTexture(4, "assets/image/buildings/building_cafeteria.png");

    interactions = {
        InteractionPoint{sf::FloatRect({392.0f, 182.0f}, {176.0f, 152.0f}), "campus_square",
            "Campus Square", "interaction.campus_square.label",
            "The center square is lively. Clubs, roadshows, and chance encounters gather here.",
            "interaction.campus_square.description"},
        InteractionPoint{sf::FloatRect({612.0f, 236.0f}, {74.0f, 110.0f}), "campus_bulletin",
            "Bulletin Board", "interaction.campus_bulletin.label",
            "Posters and notices overlap on the board. Competitions and club news are updated often.",
            "interaction.campus_bulletin.description"}
    };

    // 添加建筑碰撞体（与 MapPortal area 一致）
    for (const auto& portal : getPortals()) {
        obstacles.push_back(portal.area);
    }
}

void CampusMap::drawPixlabSprite(sf::RenderWindow& window, const sf::IntRect& textureRect,
                                  sf::Vector2f position, float scale) {
    sf::Sprite sprite(outdoorTiles);
    sprite.setTextureRect(textureRect);
    sprite.setPosition(position);
    sprite.setScale({scale, scale});
    window.draw(sprite);
}

void CampusMap::drawBuilding(sf::RenderWindow& window, const MapPortal& portal,
                              const std::string& label, const sf::Sprite* buildingSprite) {
    sf::RectangleShape shadow({portal.area.size.x + 12.0f, portal.area.size.y + 12.0f});
    shadow.setPosition({portal.area.position.x + 6.0f, portal.area.position.y + 8.0f});
    shadow.setFillColor(sf::Color(20, 28, 24, 90));
    window.draw(shadow);

    if (buildingSprite) {
        const auto texSize = buildingSprite->getTexture().getSize();
        if (texSize.y > 0) {
            const float scale = portal.area.size.y / static_cast<float>(texSize.y);
            const float scaledW = texSize.x * scale;
            const float offsetX = (portal.area.size.x - scaledW) / 2.0f;
            sf::Sprite s(*buildingSprite);
            s.setScale({scale, scale});
            s.setPosition({portal.area.position.x + offsetX, portal.area.position.y});
            window.draw(s);
        }
    } else {
        sf::RectangleShape roofShape({portal.area.size.x + 18.0f, 24.0f});
        roofShape.setPosition({portal.area.position.x - 9.0f, portal.area.position.y - 18.0f});
        roofShape.setFillColor(sf::Color(146, 74, 60));
        window.draw(roofShape);

        sf::RectangleShape bodyShape(portal.area.size);
        bodyShape.setPosition(portal.area.position);
        bodyShape.setFillColor(sf::Color(176, 112, 72));
        bodyShape.setOutlineColor(sf::Color(78, 68, 48));
        bodyShape.setOutlineThickness(2.0f);
        window.draw(bodyShape);

        sf::RectangleShape door({34.0f, 32.0f});
        door.setPosition({portal.area.position.x + portal.area.size.x / 2.0f - 17.0f,
                          portal.area.position.y + portal.area.size.y - 32.0f});
        door.setFillColor(sf::Color(72, 48, 34));
        window.draw(door);
    }

    drawLabel(window, label, {portal.area.position.x + 14.0f, portal.area.position.y + 14.0f});
}

void CampusMap::drawLamp(sf::RenderWindow& window, sf::Vector2f position, bool glow) const {
    if (glow) {
        sf::CircleShape outer(42.0f);
        outer.setOrigin({42.0f, 42.0f});
        outer.setPosition(position);
        outer.setFillColor(sf::Color(255, 214, 118, 42));
        window.draw(outer);

        sf::CircleShape inner(22.0f);
        inner.setOrigin({22.0f, 22.0f});
        inner.setPosition(position);
        inner.setFillColor(sf::Color(255, 228, 148, 74));
        window.draw(inner);
    }

    sf::RectangleShape pole({5.0f, 30.0f});
    pole.setOrigin({2.5f, 30.0f});
    pole.setPosition({position.x, position.y + 22.0f});
    pole.setFillColor(sf::Color(54, 58, 54));
    window.draw(pole);

    sf::CircleShape bulb(6.0f);
    bulb.setOrigin({6.0f, 6.0f});
    bulb.setPosition(position);
    bulb.setFillColor(glow ? sf::Color(255, 226, 142) : sf::Color(178, 170, 128));
    window.draw(bulb);
}

void CampusMap::drawTimeLighting(sf::RenderWindow& window) const {
    if (!timeSystem) return;

    sf::Color overlay(0, 0, 0, 0);
    switch (timeSystem->currentPhase()) {
        case TimePhase::EarlyMorning:
        case TimePhase::Afternoon:
            overlay = sf::Color(0, 0, 0, 0);
            break;
        case TimePhase::Noon:
            overlay = sf::Color(255, 244, 196, 26);
            break;
        case TimePhase::Evening:
            overlay = sf::Color(48, 32, 70, 82);
            break;
        case TimePhase::Night:
            overlay = sf::Color(0, 8, 24, 168);
            break;
    }

    if (overlay.a > 0) {
        sf::RectangleShape shade({960.0f, 540.0f});
        shade.setFillColor(overlay);
        window.draw(shade);
    }
}

void CampusMap::render(sf::RenderWindow& window) {
    sf::RectangleShape bg({960.0f, 540.0f});
    bg.setFillColor(sf::Color(139, 180, 74));
    window.draw(bg);

    // 主路（纵向）
    sf::RectangleShape mainPath({116.0f, 540.0f});
    mainPath.setPosition({422.0f, 0.0f});
    mainPath.setFillColor(sf::Color(214, 190, 118));
    window.draw(mainPath);

    // 横路
    sf::RectangleShape crossPath({960.0f, 92.0f});
    crossPath.setPosition({0.0f, 224.0f});
    crossPath.setFillColor(sf::Color(214, 190, 118));
    window.draw(crossPath);

    // 广场
    sf::CircleShape plaza(76.0f);
    plaza.setPosition({404.0f, 194.0f});
    plaza.setFillColor(sf::Color(188, 178, 122));
    window.draw(plaza);

    // 地面细节
    for (int x = 0; x < 960; x += 32) {
        for (int y = 42; y < 540; y += 32) {
            const bool onRoad = (x > 410 && x < 550) || (y > 210 && y < 330);
            if (onRoad) {
                sf::CircleShape pebble(1.4f);
                pebble.setPosition({static_cast<float>(x + (y % 11)), static_cast<float>(y + (x % 7))});
                pebble.setFillColor(sf::Color(166, 145, 92, 95));
                window.draw(pebble);
            } else if ((x + y) % 96 == 0) {
                if (outdoorTilesLoaded) {
                    drawPixlabSprite(window, sf::IntRect({370, 48}, {84, 76}),
                                     {static_cast<float>(x - 16), static_cast<float>(y - 26)}, 0.42f);
                } else {
                    sf::CircleShape bush(10.0f);
                    bush.setPosition({static_cast<float>(x + 10), static_cast<float>(y + 12)});
                    bush.setFillColor(sf::Color(91, 145, 76));
                    window.draw(bush);
                }
            }
        }
    }

    // 建筑
    const auto portals = getPortals();
    drawBuilding(window, portals[0], cls::text("map.dorm.short"), getBuildingSprite(CampusPlace::Dormitory));
    drawBuilding(window, portals[1], cls::text("map.gym"),         getBuildingSprite(CampusPlace::Gym));
    drawBuilding(window, portals[2], cls::text("map.library"),     getBuildingSprite(CampusPlace::Library));
    drawBuilding(window, portals[3], cls::text("map.classroom"),   getBuildingSprite(CampusPlace::Classroom));
    drawBuilding(window, portals[4], cls::text("map.cafeteria"),   getBuildingSprite(CampusPlace::Cafeteria));
    drawBuilding(window, portals[5], cls::text("map.store"),       nullptr);
    drawTimeLighting(window);

    const bool lampGlow = timeSystem
        && (timeSystem->currentPhase() == TimePhase::Evening || timeSystem->currentPhase() == TimePhase::Night);
    for (const sf::Vector2f lampPos : {
             sf::Vector2f{404.0f, 92.0f}, sf::Vector2f{556.0f, 92.0f},
             sf::Vector2f{394.0f, 220.0f}, sf::Vector2f{566.0f, 220.0f},
             sf::Vector2f{392.0f, 332.0f}, sf::Vector2f{568.0f, 332.0f},
             sf::Vector2f{246.0f, 182.0f}, sf::Vector2f{680.0f, 182.0f},
             sf::Vector2f{282.0f, 438.0f}, sf::Vector2f{676.0f, 438.0f}}) {
        drawLamp(window, lampPos, lampGlow);
    }

    // 喷泉
    sf::CircleShape fountain(24.0f);
    fountain.setPosition({456.0f, 250.0f});
    fountain.setFillColor(sf::Color(82, 156, 176));
    fountain.setOutlineColor(sf::Color(224, 220, 178));
    fountain.setOutlineThickness(4.0f);
    window.draw(fountain);

    // 长椅
    for (const sf::Vector2f benchPos : {sf::Vector2f{322.0f, 236.0f}, sf::Vector2f{584.0f, 236.0f},
                                        sf::Vector2f{322.0f, 300.0f}, sf::Vector2f{584.0f, 300.0f}}) {
        sf::RectangleShape bench({54.0f, 14.0f});
        bench.setPosition(benchPos);
        bench.setFillColor(sf::Color(132, 72, 42));
        window.draw(bench);
    }

    sf::RectangleShape bulletin({32.0f, 84.0f});
    bulletin.setPosition({632.0f, 246.0f});
    bulletin.setFillColor(sf::Color(108, 78, 50));
    bulletin.setOutlineColor(sf::Color(238, 228, 186));
    bulletin.setOutlineThickness(2.0f);
    window.draw(bulletin);

    sf::RectangleShape poster({22.0f, 58.0f});
    poster.setPosition({637.0f, 254.0f});
    poster.setFillColor(sf::Color(242, 228, 176));
    window.draw(poster);

    drawPortalMarkers(window);

    // 地点标题 + 提示
    sf::Text title = cls::makeText(*font, placeName(getPlace()), 22);
    title.setFillColor(sf::Color(245, 238, 205));
    title.setOutlineColor(sf::Color(20, 30, 30));
    title.setOutlineThickness(2.0f);
    title.setPosition({18.0f, 50.0f});
    window.draw(title);

    sf::Text hint = cls::makeText(*font, cls::text("campus.move_hint"), 13);
    hint.setFillColor(sf::Color(235, 235, 210));
    hint.setPosition({18.0f, 510.0f});
    window.draw(hint);
}

void CampusMap::renderPlayer(sf::RenderWindow& window, Player& player) {
    player.render(window);
}

void CampusMap::loadBuildingTexture(int index, const std::string& relativePath) {
    const std::string resolved = cls::resolveAssetPath(relativePath);
    if (!std::filesystem::exists(resolved)) return;
    if (mBuildingTextures[index].loadFromFile(resolved)) {
        mBuildingSprites[index] = std::make_unique<sf::Sprite>(mBuildingTextures[index]);
    }
}

const sf::Sprite* CampusMap::getBuildingSprite(CampusPlace place) const {
    switch (place) {
        case CampusPlace::Dormitory: return mBuildingSprites[0].get();
        case CampusPlace::Gym:       return mBuildingSprites[1].get();
        case CampusPlace::Library:   return mBuildingSprites[2].get();
        case CampusPlace::Classroom: return mBuildingSprites[3].get();
        case CampusPlace::Cafeteria: return mBuildingSprites[4].get();
        default: return nullptr;
    }
}

std::vector<MapPortal> CampusMap::getPortals() const {
    return {
        MapPortal{sf::FloatRect({80.0f, 86.0f}, {150.0f, 92.0f}), CampusPlace::Dormitory, SceneBackgroundType::Dormitory,
            {480.0f, 448.0f}, cls::text("map.dormitory"), cls::text("scene.dormitory.subtitle")},
        MapPortal{sf::FloatRect({260.0f, 100.0f}, {110.0f, 76.0f}), CampusPlace::Gym, SceneBackgroundType::Gym,
            {480.0f, 448.0f}, cls::text("map.gym"), cls::text("scene.gym.subtitle")},
        MapPortal{sf::FloatRect({702.0f, 82.0f}, {168.0f, 96.0f}), CampusPlace::Library, SceneBackgroundType::Library,
            {480.0f, 448.0f}, cls::text("map.library"), cls::text("scene.library.subtitle")},
        MapPortal{sf::FloatRect({92.0f, 352.0f}, {176.0f, 104.0f}), CampusPlace::Classroom, SceneBackgroundType::Classroom,
            {480.0f, 448.0f}, cls::text("map.classroom"), cls::text("scene.classroom.subtitle")},
        MapPortal{sf::FloatRect({690.0f, 350.0f}, {182.0f, 106.0f}), CampusPlace::Cafeteria, SceneBackgroundType::Cafeteria,
            {480.0f, 448.0f}, cls::text("map.cafeteria"), cls::text("scene.cafeteria.subtitle")},
        MapPortal{sf::FloatRect({414.0f, 420.0f}, {132.0f, 72.0f}), CampusPlace::Store, SceneBackgroundType::Store,
            {480.0f, 448.0f}, cls::text("map.store"), cls::text("scene.store.subtitle")}
    };
}
