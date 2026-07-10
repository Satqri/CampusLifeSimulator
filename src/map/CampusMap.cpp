#include "map/CampusMap.h"
#include "utils/AssetPath.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"
#include "core/TimeSystem.h"
#include "entity/Player.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <tuple>
#include <utility>

namespace {

constexpr float kCampusWidth = 960.0f;
constexpr float kCampusHeight = 540.0f;
constexpr float kPi = 3.14159265358979323846f;

sf::Vector2f pointOnCircle(sf::Vector2f center, float radius, float degrees) {
    const float radians = degrees * kPi / 180.0f;
    return {center.x + std::cos(radians) * radius,
            center.y + std::sin(radians) * radius};
}

} // namespace

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
    loadBuildingTexture(5, "assets/image/buildings/building_store.png");
    mLampTextureLoaded = mLampTexture.loadFromFile(cls::resolveAssetPath("assets/image/ui/map/lamp_campus.png"));

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
                                  sf::Vector2f position, float scale) const {
    sf::Sprite sprite(outdoorTiles);
    sprite.setTextureRect(textureRect);
    sprite.setPosition(position);
    sprite.setScale({scale, scale});
    window.draw(sprite);
}

void CampusMap::drawEllipse(sf::RenderWindow& window, sf::Vector2f center, sf::Vector2f radii,
                             sf::Color color) const {
    sf::CircleShape ellipse(1.0f, 64);
    ellipse.setOrigin({1.0f, 1.0f});
    ellipse.setPosition(center);
    ellipse.setScale(radii);
    ellipse.setFillColor(color);
    window.draw(ellipse);
}

void CampusMap::drawGround(sf::RenderWindow& window) const {
    sf::RectangleShape bg({kCampusWidth, kCampusHeight});
    bg.setFillColor(sf::Color(112, 166, 92));
    window.draw(bg);

    auto drawPatch = [&window](std::initializer_list<sf::Vector2f> points, sf::Color color) {
        sf::ConvexShape patch(points.size());
        std::size_t index = 0;
        for (const auto& point : points) {
            patch.setPoint(index++, point);
        }
        patch.setFillColor(color);
        window.draw(patch);
    };

    drawPatch({{24.0f, 66.0f}, {246.0f, 50.0f}, {284.0f, 86.0f}, {268.0f, 202.0f},
               {210.0f, 220.0f}, {36.0f, 206.0f}, {18.0f, 154.0f}},
              sf::Color(86, 148, 84, 104));
    drawPatch({{648.0f, 52.0f}, {922.0f, 46.0f}, {940.0f, 190.0f}, {888.0f, 222.0f},
               {692.0f, 204.0f}, {642.0f, 154.0f}},
              sf::Color(78, 142, 88, 112));
    drawPatch({{40.0f, 330.0f}, {286.0f, 326.0f}, {318.0f, 420.0f}, {280.0f, 510.0f},
               {74.0f, 520.0f}, {18.0f, 472.0f}, {24.0f, 382.0f}},
              sf::Color(72, 140, 92, 108));
    drawPatch({{624.0f, 330.0f}, {930.0f, 324.0f}, {950.0f, 502.0f}, {812.0f, 526.0f},
               {646.0f, 500.0f}, {604.0f, 402.0f}},
              sf::Color(78, 146, 92, 108));

    for (int x = 8; x < static_cast<int>(kCampusWidth); x += 22) {
        for (int y = 50; y < static_cast<int>(kCampusHeight); y += 22) {
            const int seed = (x * 37 + y * 19) % 7;
            sf::RectangleShape fleck({seed % 2 == 0 ? 2.0f : 1.0f, seed % 3 == 0 ? 3.0f : 1.0f});
            fleck.setPosition({static_cast<float>(x + (seed * 5) % 13),
                               static_cast<float>(y + (seed * 7) % 11)});
            if (seed < 3) {
                fleck.setFillColor(sf::Color(62, 116, 70, 74));
            } else if (seed < 5) {
                fleck.setFillColor(sf::Color(152, 196, 104, 68));
            } else {
                fleck.setFillColor(sf::Color(188, 204, 116, 46));
            }
            window.draw(fleck);
        }
    }
}

void CampusMap::drawPathSegment(sf::RenderWindow& window, sf::Vector2f start, sf::Vector2f end,
                                float width, sf::Color color) const {
    const sf::Vector2f delta = end - start;
    const float length = std::hypot(delta.x, delta.y);
    if (length <= 0.01f) return;

    sf::RectangleShape segment({length, width});
    segment.setOrigin({0.0f, width * 0.5f});
    segment.setPosition(start);
    segment.setRotation(sf::degrees(std::atan2(delta.y, delta.x) * 180.0f / kPi));
    segment.setFillColor(color);
    window.draw(segment);
}

void CampusMap::drawRoundedPath(sf::RenderWindow& window, std::initializer_list<sf::Vector2f> points,
                                float width, sf::Color color) const {
    if (points.size() == 0) return;

    auto previous = points.begin();
    for (auto current = previous + 1; current != points.end(); ++current) {
        drawPathSegment(window, *previous, *current, width, color);
        previous = current;
    }

    const float radius = width * 0.5f;
    for (const auto& point : points) {
        sf::CircleShape cap(radius, 48);
        cap.setOrigin({radius, radius});
        cap.setPosition(point);
        cap.setFillColor(color);
        window.draw(cap);
    }
}

void CampusMap::drawPathNetwork(sf::RenderWindow& window) const {
    auto drawPath = [this, &window](std::initializer_list<sf::Vector2f> points,
                                    float shadowWidth, float edgeWidth,
                                    float baseWidth, float topWidth) {
        drawRoundedPath(window, points, shadowWidth, sf::Color(30, 42, 30, 78));
        drawRoundedPath(window, points, edgeWidth, sf::Color(150, 124, 76));
        drawRoundedPath(window, points, baseWidth, sf::Color(226, 204, 142));
        drawRoundedPath(window, points, topWidth, sf::Color(204, 180, 112));
    };

    drawPath({{480.0f, 540.0f}, {480.0f, 410.0f}, {480.0f, 270.0f}, {480.0f, 0.0f}},
             88.0f, 76.0f, 64.0f, 50.0f);
    drawPath({{0.0f, 270.0f}, {214.0f, 270.0f}, {360.0f, 272.0f}, {480.0f, 270.0f},
              {610.0f, 272.0f}, {960.0f, 270.0f}},
             88.0f, 76.0f, 64.0f, 50.0f);

    drawPath({{480.0f, 270.0f}, {340.0f, 214.0f}, {290.0f, 160.0f}},
             58.0f, 48.0f, 40.0f, 30.0f);
    drawPath({{480.0f, 270.0f}, {626.0f, 210.0f}, {786.0f, 148.0f}},
             58.0f, 48.0f, 40.0f, 30.0f);
    drawPath({{480.0f, 270.0f}, {315.0f, 346.0f}, {180.0f, 410.0f}},
             58.0f, 48.0f, 40.0f, 30.0f);
    drawPath({{480.0f, 270.0f}, {650.0f, 350.0f}, {780.0f, 408.0f}},
             58.0f, 48.0f, 40.0f, 30.0f);
    drawPath({{480.0f, 370.0f}, {480.0f, 430.0f}, {480.0f, 492.0f}},
             52.0f, 44.0f, 36.0f, 28.0f);

    auto drawPebbles = [this, &window](sf::Vector2f start, sf::Vector2f end, int count) {
        const sf::Vector2f delta = end - start;
        const float length = std::hypot(delta.x, delta.y);
        if (length <= 0.01f) return;

        const sf::Vector2f normal{-delta.y / length, delta.x / length};
        for (int i = 0; i < count; ++i) {
            const float t = (static_cast<float>(i) + 0.55f) / (static_cast<float>(count) + 1.0f);
            const float offset = std::sin(static_cast<float>(i) * 2.37f + start.x * 0.013f) * 14.0f;
            const sf::Vector2f pos{
                start.x + delta.x * t + normal.x * offset,
                start.y + delta.y * t + normal.y * offset
            };
            drawEllipse(window, pos, {2.2f, 1.4f}, sf::Color(120, 104, 74, 82));
        }
    };

    drawPebbles({18.0f, 270.0f}, {930.0f, 270.0f}, 30);
    drawPebbles({480.0f, 28.0f}, {480.0f, 520.0f}, 22);
    drawPebbles({480.0f, 270.0f}, {786.0f, 148.0f}, 10);
    drawPebbles({480.0f, 270.0f}, {180.0f, 410.0f}, 10);
    drawPebbles({480.0f, 270.0f}, {780.0f, 408.0f}, 10);
    drawPebbles({480.0f, 270.0f}, {290.0f, 160.0f}, 8);
}

void CampusMap::drawPlaza(sf::RenderWindow& window) const {
    const sf::Vector2f center{480.0f, 270.0f};
    drawEllipse(window, {center.x, center.y + 8.0f}, {116.0f, 112.0f}, sf::Color(40, 42, 30, 86));

    const std::array<std::pair<float, sf::Color>, 4> rings = {{
        {104.0f, sf::Color(230, 210, 148)},
        {88.0f,  sf::Color(187, 176, 122)},
        {68.0f,  sf::Color(220, 203, 146)},
        {48.0f,  sf::Color(194, 178, 116)}
    }};
    for (const auto& [radius, color] : rings) {
        sf::CircleShape ring(radius, 96);
        ring.setOrigin({radius, radius});
        ring.setPosition(center);
        ring.setFillColor(color);
        ring.setOutlineColor(sf::Color(116, 96, 70, 88));
        ring.setOutlineThickness(2.0f);
        window.draw(ring);
    }

    for (int i = 0; i < 16; ++i) {
        drawPathSegment(window, center, pointOnCircle(center, 96.0f, static_cast<float>(i) * 22.5f),
                        1.2f, sf::Color(116, 96, 70, 70));
    }

    sf::CircleShape fountainBase(30.0f, 64);
    fountainBase.setOrigin({30.0f, 30.0f});
    fountainBase.setPosition(center);
    fountainBase.setFillColor(sf::Color(226, 220, 178));
    fountainBase.setOutlineColor(sf::Color(72, 116, 130));
    fountainBase.setOutlineThickness(3.0f);
    window.draw(fountainBase);

    sf::CircleShape water(21.0f, 64);
    water.setOrigin({21.0f, 21.0f});
    water.setPosition(center);
    water.setFillColor(sf::Color(74, 154, 178));
    window.draw(water);

    sf::CircleShape waterLight(9.0f, 32);
    waterLight.setOrigin({9.0f, 9.0f});
    waterLight.setPosition({center.x + 2.0f, center.y - 2.0f});
    waterLight.setFillColor(sf::Color(136, 208, 224, 190));
    window.draw(waterLight);
}

void CampusMap::drawTree(sf::RenderWindow& window, sf::Vector2f position, float scale) const {
    sf::RectangleShape trunk({6.0f * scale, 20.0f * scale});
    trunk.setOrigin({3.0f * scale, 0.0f});
    trunk.setPosition({position.x, position.y + 8.0f * scale});
    trunk.setFillColor(sf::Color(86, 58, 38));
    window.draw(trunk);

    const std::array<std::tuple<sf::Vector2f, float, sf::Color>, 4> leaves = {{
        {sf::Vector2f{-8.0f, -3.0f}, 17.0f, sf::Color(48, 116, 70)},
        {sf::Vector2f{8.0f, -4.0f}, 16.0f, sf::Color(62, 136, 76)},
        {sf::Vector2f{0.0f, -15.0f}, 18.0f, sf::Color(74, 154, 84)},
        {sf::Vector2f{0.0f, 2.0f}, 16.0f, sf::Color(54, 126, 72)}
    }};
    for (const auto& [offset, radius, color] : leaves) {
        sf::CircleShape leaf(radius * scale, 28);
        leaf.setOrigin({radius * scale, radius * scale});
        leaf.setPosition({position.x + offset.x * scale, position.y + offset.y * scale});
        leaf.setFillColor(color);
        window.draw(leaf);
    }

    sf::CircleShape highlight(6.0f * scale, 18);
    highlight.setOrigin({6.0f * scale, 6.0f * scale});
    highlight.setPosition({position.x - 3.0f * scale, position.y - 21.0f * scale});
    highlight.setFillColor(sf::Color(142, 190, 96, 120));
    window.draw(highlight);
}

void CampusMap::drawFlowerBed(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size) const {
    sf::RectangleShape bed(size);
    bed.setPosition(position);
    bed.setFillColor(sf::Color(58, 122, 70));
    bed.setOutlineColor(sf::Color(230, 208, 132, 185));
    bed.setOutlineThickness(2.0f);
    window.draw(bed);

    for (int i = 0; i < 4; ++i) {
        const float x = position.x + 8.0f + static_cast<float>(i) * (size.x - 16.0f) / 3.0f;
        const float y = position.y + size.y * 0.48f + (i % 2 == 0 ? -2.0f : 2.0f);
        sf::CircleShape flower(2.4f, 12);
        flower.setOrigin({2.4f, 2.4f});
        flower.setPosition({x, y});
        flower.setFillColor(i % 3 == 0 ? sf::Color(236, 116, 96)
                            : (i % 3 == 1 ? sf::Color(248, 208, 94)
                                           : sf::Color(216, 128, 176)));
        window.draw(flower);
    }
}

void CampusMap::drawBench(sf::RenderWindow& window, sf::Vector2f position) const {
    sf::RectangleShape shadow({60.0f, 8.0f});
    shadow.setPosition({position.x + 2.0f, position.y + 12.0f});
    shadow.setFillColor(sf::Color(22, 24, 20, 66));
    window.draw(shadow);

    sf::RectangleShape seat({58.0f, 13.0f});
    seat.setPosition(position);
    seat.setFillColor(sf::Color(126, 74, 48));
    window.draw(seat);

    sf::RectangleShape stripe({50.0f, 2.0f});
    stripe.setPosition({position.x + 4.0f, position.y + 5.0f});
    stripe.setFillColor(sf::Color(180, 122, 72, 150));
    window.draw(stripe);

    for (float legX : {8.0f, 48.0f}) {
        sf::RectangleShape leg({5.0f, 9.0f});
        leg.setPosition({position.x + legX, position.y + 12.0f});
        leg.setFillColor(sf::Color(70, 54, 38));
        window.draw(leg);
    }
}

void CampusMap::drawCampusDetails(sf::RenderWindow& window) const {
    const std::array<sf::Vector2f, 22> trees = {{
        {44.0f, 92.0f}, {58.0f, 154.0f}, {52.0f, 450.0f}, {836.0f, 52.0f},
        {908.0f, 90.0f}, {902.0f, 450.0f}, {80.0f, 506.0f}, {900.0f, 510.0f},
        {346.0f, 76.0f}, {620.0f, 78.0f}, {330.0f, 460.0f}, {626.0f, 462.0f},
        {154.0f, 236.0f}, {790.0f, 232.0f}, {116.0f, 286.0f}, {850.0f, 308.0f},
        {22.0f, 356.0f}, {934.0f, 364.0f}, {236.0f, 64.0f}, {718.0f, 62.0f},
        {250.0f, 500.0f}, {708.0f, 500.0f}
    }};
    for (std::size_t i = 0; i < trees.size(); ++i) {
        drawTree(window, trees[i], 0.82f + static_cast<float>(i % 3) * 0.12f);
    }

    drawFlowerBed(window, {388.0f, 196.0f}, {44.0f, 18.0f});
    drawFlowerBed(window, {528.0f, 196.0f}, {44.0f, 18.0f});
    drawFlowerBed(window, {386.0f, 326.0f}, {46.0f, 18.0f});
    drawFlowerBed(window, {526.0f, 326.0f}, {46.0f, 18.0f});

    for (const sf::Vector2f benchPos : {sf::Vector2f{320.0f, 230.0f}, sf::Vector2f{584.0f, 230.0f},
                                        sf::Vector2f{320.0f, 302.0f}, sf::Vector2f{584.0f, 302.0f}}) {
        drawBench(window, benchPos);
    }

    sf::RectangleShape boardShadow({58.0f, 108.0f});
    boardShadow.setPosition({622.0f, 236.0f});
    boardShadow.setFillColor(sf::Color(24, 24, 18, 72));
    window.draw(boardShadow);

    sf::RectangleShape bulletin({50.0f, 110.0f});
    bulletin.setPosition({624.0f, 230.0f});
    bulletin.setFillColor(sf::Color(94, 67, 44));
    bulletin.setOutlineColor(sf::Color(238, 224, 178, 220));
    bulletin.setOutlineThickness(3.0f);
    window.draw(bulletin);

    sf::RectangleShape poster({32.0f, 78.0f});
    poster.setPosition({633.0f, 241.0f});
    poster.setFillColor(sf::Color(246, 230, 176));
    window.draw(poster);
    for (int i = 0; i < 3; ++i) {
        sf::RectangleShape strip({22.0f + static_cast<float>(i % 2) * 4.0f, 6.0f});
        strip.setPosition({638.0f, 251.0f + static_cast<float>(i) * 16.0f});
        strip.setFillColor(i == 0 ? sf::Color(210, 90, 78, 180)
                          : (i == 1 ? sf::Color(80, 132, 182, 160)
                                    : sf::Color(88, 152, 98, 160)));
        window.draw(strip);
    }

}

void CampusMap::drawBuilding(sf::RenderWindow& window, const MapPortal& portal,
                              const std::string& label, const sf::Sprite* buildingSprite) {
    (void)label;
    const auto& area = portal.area;

    if (buildingSprite) {
        const auto texSize = buildingSprite->getTexture().getSize();
        if (texSize.y > 0) {
            const float scale = area.size.y / static_cast<float>(texSize.y);
            const float scaledW = texSize.x * scale;
            const float offsetX = (area.size.x - scaledW) / 2.0f;
            sf::Sprite s(*buildingSprite);
            s.setScale({scale, scale});
            s.setPosition({area.position.x + offsetX, area.position.y});
            window.draw(s);
        }
    } else {
        sf::ConvexShape roof(3);
        roof.setPoint(0, {area.position.x - 12.0f, area.position.y + 18.0f});
        roof.setPoint(1, {area.position.x + area.size.x * 0.5f, area.position.y - 12.0f});
        roof.setPoint(2, {area.position.x + area.size.x + 12.0f, area.position.y + 18.0f});
        roof.setFillColor(sf::Color(136, 70, 56));
        roof.setOutlineColor(sf::Color(70, 55, 42));
        roof.setOutlineThickness(2.0f);
        window.draw(roof);

        sf::RectangleShape bodyShape({area.size.x, area.size.y - 18.0f});
        bodyShape.setPosition({area.position.x, area.position.y + 18.0f});
        bodyShape.setFillColor(sf::Color(180, 108, 64));
        bodyShape.setOutlineColor(sf::Color(78, 68, 48));
        bodyShape.setOutlineThickness(2.0f);
        window.draw(bodyShape);

        sf::RectangleShape door({34.0f, 38.0f});
        door.setPosition({area.position.x + area.size.x * 0.5f - 17.0f,
                          area.position.y + area.size.y - 38.0f});
        door.setFillColor(sf::Color(72, 48, 34));
        window.draw(door);

        for (float windowX : {20.0f, area.size.x - 40.0f}) {
            sf::RectangleShape storeWindow({22.0f, 18.0f});
            storeWindow.setPosition({area.position.x + windowX, area.position.y + 34.0f});
            storeWindow.setFillColor(sf::Color(106, 172, 190, 235));
            storeWindow.setOutlineColor(sf::Color(238, 222, 166, 180));
            storeWindow.setOutlineThickness(1.0f);
            window.draw(storeWindow);
        }
    }

}

void CampusMap::drawMapTitle(sf::RenderWindow& window) const {
    if (!font) return;

    sf::RectangleShape titlePlate({236.0f, 42.0f});
    titlePlate.setPosition({14.0f, 50.0f});
    titlePlate.setFillColor(sf::Color(14, 34, 36, 186));
    titlePlate.setOutlineColor(sf::Color(224, 205, 143, 92));
    titlePlate.setOutlineThickness(1.0f);
    window.draw(titlePlate);

    sf::Text title = cls::makeText(*font, placeName(getPlace()), 22);
    title.setFillColor(sf::Color(255, 239, 194));
    title.setPosition({30.0f, 57.0f});
    window.draw(title);
}

void CampusMap::drawLamp(sf::RenderWindow& window, sf::Vector2f position, bool glow) const {
    if (glow) {
        drawEllipse(window, position, {34.0f, 24.0f}, sf::Color(255, 214, 118, 42));
        drawEllipse(window, position, {18.0f, 13.0f}, sf::Color(255, 228, 148, 74));
    }

    if (mLampTextureLoaded) {
        sf::Sprite lamp(mLampTexture);
        const auto texSize = mLampTexture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            const float scale = 0.54f;
            lamp.setScale({scale, scale});
            lamp.setPosition({position.x - static_cast<float>(texSize.x) * scale * 0.5f,
                              position.y - 16.0f});
            window.draw(lamp);
            return;
        }
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
    drawGround(window);
    drawPathNetwork(window);
    drawPlaza(window);
    drawCampusDetails(window);

    const auto portals = getPortals();
    drawBuilding(window, portals[0], cls::text("map.dorm.short"), getBuildingSprite(CampusPlace::Dormitory));
    drawBuilding(window, portals[1], cls::text("map.gym"),         getBuildingSprite(CampusPlace::Gym));
    drawBuilding(window, portals[2], cls::text("map.library"),     getBuildingSprite(CampusPlace::Library));
    drawBuilding(window, portals[3], cls::text("map.classroom"),   getBuildingSprite(CampusPlace::Classroom));
    drawBuilding(window, portals[4], cls::text("map.cafeteria"),   getBuildingSprite(CampusPlace::Cafeteria));
    drawBuilding(window, portals[5], cls::text("map.store"),       getBuildingSprite(CampusPlace::Store));

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

    drawMapTitle(window);
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
        case CampusPlace::Store:     return mBuildingSprites[5].get();
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
