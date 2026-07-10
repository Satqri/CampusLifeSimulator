#include "map/ConvenienceStoreInterior.h"
#include "utils/AssetPath.h"

#include <array>

namespace {
constexpr float kRoomLeft = 42.0f;
constexpr float kRoomTop = 72.0f;
constexpr float kRoomWidth = 876.0f;
constexpr float kRoomHeight = 424.0f;
constexpr float kMockupPixels = 320.0f;
constexpr float kMockupScale = kRoomHeight / kMockupPixels;
constexpr float kMockupSize = kMockupPixels * kMockupScale;
constexpr float kMockupLeft = (kRenderWidth - kMockupSize) * 0.5f;
constexpr float kMockupTop = kRoomTop;
constexpr float kMockupRight = kMockupLeft + kMockupSize;
constexpr float kRoomBottom = kRoomTop + kRoomHeight;
constexpr float kWalkableFloorTop = kMockupTop + 126.0f * kMockupScale;
constexpr float kPlayerClampBottom = 524.0f;

sf::FloatRect mapMockupRect(float x, float y, float width, float height) {
    return sf::FloatRect(
        {kMockupLeft + x * kMockupScale, kMockupTop + y * kMockupScale},
        {width * kMockupScale, height * kMockupScale}
    );
}

void drawRect(sf::RenderWindow& window, sf::FloatRect rect, sf::Color color) {
    sf::RectangleShape shape(rect.size);
    shape.setPosition(rect.position);
    shape.setFillColor(color);
    window.draw(shape);
}

const sf::FloatRect kCounterArea = mapMockupRect(4.0f, 112.0f, 60.0f, 34.0f);
const sf::FloatRect kShelfArea = mapMockupRect(73.0f, 173.0f, 37.0f, 36.0f);
const sf::FloatRect kDrinkFridgeArea = mapMockupRect(178.0f, 94.0f, 87.0f, 54.0f);
const sf::FloatRect kHotWaterArea = mapMockupRect(136.0f, 197.0f, 49.0f, 70.0f);
const sf::FloatRect kNightShiftPosterArea = mapMockupRect(100.0f, 78.0f, 28.0f, 44.0f);

const std::array<sf::FloatRect, 11> kSolidObstacles = {
    mapMockupRect(4.0f, 116.0f, 58.0f, 27.0f),
    mapMockupRect(145.0f, 116.0f, 18.0f, 28.0f),
    mapMockupRect(178.0f, 99.0f, 28.0f, 44.0f),
    mapMockupRect(236.0f, 99.0f, 28.0f, 44.0f),
    mapMockupRect(73.0f, 177.0f, 35.0f, 28.0f),
    mapMockupRect(137.0f, 199.0f, 45.0f, 64.0f),
    mapMockupRect(49.0f, 244.0f, 60.0f, 18.0f),
    mapMockupRect(222.0f, 183.0f, 47.0f, 24.0f),
    mapMockupRect(216.0f, 248.0f, 40.0f, 43.0f),
    mapMockupRect(262.0f, 248.0f, 35.0f, 43.0f),
    mapMockupRect(8.0f, 230.0f, 14.0f, 22.0f)
};
} // namespace

ConvenienceStoreInterior::ConvenienceStoreInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/store.json"));

    mMockupLoaded = mMockupTexture.loadFromFile(
        cls::resolveAssetPath("assets/image/backgrounds/store_mockup.png"));
    if (mMockupLoaded) {
        mMockupTexture.setSmooth(false);
    }

    updateInteractionArea("store_counter", kCounterArea);
    updateInteractionArea("store_shelf", kShelfArea);
    updateInteractionArea("store_drink_fridge", kDrinkFridgeArea);
    updateInteractionArea("store_hot_water", kHotWaterArea);
    updateInteractionArea("store_night_shift", kNightShiftPosterArea);

    obstacles.clear();
    obstacles.push_back(sf::FloatRect({kMockupLeft, kRoomTop}, {kMockupSize, kWalkableFloorTop - kRoomTop}));
    obstacles.push_back(sf::FloatRect({kRoomLeft, kRoomTop}, {kMockupLeft - kRoomLeft, kPlayerClampBottom - kRoomTop}));
    obstacles.push_back(sf::FloatRect({kMockupRight, kRoomTop},
                                      {kRoomLeft + kRoomWidth - kMockupRight, kPlayerClampBottom - kRoomTop}));
    for (const auto& obstacle : kSolidObstacles) {
        obstacles.push_back(obstacle);
    }
}

void ConvenienceStoreInterior::drawStoreBackground(sf::RenderWindow& window) const {
    drawRect(window, sf::FloatRect({0.0f, 0.0f}, {kRenderWidth, kRenderHeight}),
             sf::Color(18, 19, 27));
}

void ConvenienceStoreInterior::drawMockup(sf::RenderWindow& window) const {
    if (!mMockupLoaded) return;

    sf::Sprite sprite(mMockupTexture);
    sprite.setPosition({kMockupLeft, kMockupTop});
    sprite.setScale({kMockupScale, kMockupScale});
    window.draw(sprite);

    drawRect(window, kNightShiftPosterArea, sf::Color(238, 205, 116));
    drawRect(window, mapMockupRect(103.0f, 82.0f, 22.0f, 7.0f), sf::Color(174, 54, 48));
    drawRect(window, mapMockupRect(105.0f, 94.0f, 18.0f, 3.0f), sf::Color(91, 61, 47));
    drawRect(window, mapMockupRect(105.0f, 101.0f, 14.0f, 3.0f), sf::Color(91, 61, 47));
    drawRect(window, mapMockupRect(105.0f, 108.0f, 17.0f, 3.0f), sf::Color(91, 61, 47));
}

void ConvenienceStoreInterior::drawFallbackScene(sf::RenderWindow& window) const {
    drawRect(window, sf::FloatRect({kMockupLeft, kMockupTop}, {kMockupSize, kWalkableFloorTop - kMockupTop}),
             sf::Color(42, 54, 108));
    drawRect(window, sf::FloatRect({kMockupLeft, kWalkableFloorTop}, {kMockupSize, kRoomBottom - kWalkableFloorTop}),
             sf::Color(82, 43, 35));
    drawRect(window, kCounterArea, sf::Color(124, 74, 52));
    drawRect(window, kShelfArea, sf::Color(192, 136, 48));
    drawRect(window, kDrinkFridgeArea, sf::Color(116, 204, 225));
    drawRect(window, kHotWaterArea, sf::Color(142, 148, 148));
    drawRect(window, kNightShiftPosterArea, sf::Color(238, 205, 116));
}

void ConvenienceStoreInterior::render(sf::RenderWindow& window) {
    drawStoreBackground(window);
    if (mMockupLoaded) {
        drawMockup(window);
    } else {
        drawFallbackScene(window);
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> ConvenienceStoreInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Store, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
