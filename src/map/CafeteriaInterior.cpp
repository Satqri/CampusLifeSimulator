#include "map/CafeteriaInterior.h"
#include "utils/AssetPath.h"
#include <array>
#include <filesystem>
#include <queue>
#include <string>
#include <vector>

namespace {
const sf::FloatRect kFloorArea({54.0f, 84.0f}, {846.0f, 402.0f});
const sf::FloatRect kCounterArea({75.0f, 80.0f}, {838.0f, 130.0f});
const sf::FloatRect kMainCounterArea({75.0f, 80.0f}, {596.0f, 130.0f});
const sf::FloatRect kSideCounterArea({728.0f, 80.0f}, {185.0f, 130.0f});

const std::array<sf::FloatRect, 5> kTableAreas = {
    sf::FloatRect({100.0f, 264.0f}, {122.0f, 132.0f}),
    sf::FloatRect({268.0f, 258.0f}, {132.0f, 140.0f}),
    sf::FloatRect({436.0f, 262.0f}, {132.0f, 136.0f}),
    sf::FloatRect({604.0f, 262.0f}, {132.0f, 136.0f}),
    sf::FloatRect({776.0f, 264.0f}, {122.0f, 132.0f}),
};

const sf::IntRect kCounterSource({280, 350}, {1560, 340});
const sf::IntRect kSideCounterSource({1980, 360}, {470, 330});

const std::array<sf::IntRect, 5> kTableSources = {
    sf::IntRect({360, 805}, {270, 350}),
    sf::IntRect({810, 800}, {340, 360}),
    sf::IntRect({1250, 805}, {340, 350}),
    sf::IntRect({1690, 805}, {340, 350}),
    sf::IntRect({2130, 805}, {310, 350}),
};

bool isSheetBackground(sf::Color color) {
    return color.r >= 168 && color.r <= 218
        && color.g >= 142 && color.g <= 192
        && color.b >= 120 && color.b <= 172
        && color.r >= color.g
        && color.g >= color.b;
}

void removeConnectedSheetBackground(sf::Image& image) {
    const auto size = image.getSize();
    if (size.x == 0 || size.y == 0) return;

    std::vector<bool> visited(static_cast<std::size_t>(size.x) * size.y, false);
    std::queue<sf::Vector2u> pending;

    auto indexOf = [size](sf::Vector2u point) {
        return static_cast<std::size_t>(point.y) * size.x + point.x;
    };

    auto enqueue = [&](sf::Vector2u point) {
        const auto index = indexOf(point);
        if (visited[index] || !isSheetBackground(image.getPixel(point))) return;
        visited[index] = true;
        pending.push(point);
    };

    for (unsigned x = 0; x < size.x; ++x) {
        enqueue({x, 0});
        enqueue({x, size.y - 1});
    }

    for (unsigned y = 0; y < size.y; ++y) {
        enqueue({0, y});
        enqueue({size.x - 1, y});
    }

    while (!pending.empty()) {
        const auto point = pending.front();
        pending.pop();

        auto color = image.getPixel(point);
        color.a = 0;
        image.setPixel(point, color);

        if (point.x > 0) enqueue({point.x - 1, point.y});
        if (point.x + 1 < size.x) enqueue({point.x + 1, point.y});
        if (point.y > 0) enqueue({point.x, point.y - 1});
        if (point.y + 1 < size.y) enqueue({point.x, point.y + 1});
    }
}

bool loadMaskedTexture(const sf::Image& sheet, sf::Texture& texture, sf::IntRect source) {
    sf::Image crop;
    crop.resize({static_cast<unsigned>(source.size.x), static_cast<unsigned>(source.size.y)}, sf::Color::Transparent);
    if (!crop.copy(sheet, {0, 0}, source)) return false;

    removeConnectedSheetBackground(crop);

    if (!texture.loadFromImage(crop)) return false;
    texture.setSmooth(false);
    return true;
}

void drawFittedSprite(sf::RenderWindow& window, const sf::Texture& texture, sf::FloatRect target) {
    const auto size = texture.getSize();
    if (size.x == 0 || size.y == 0) return;

    sf::Sprite sprite(texture);
    sprite.setScale({
        target.size.x / static_cast<float>(size.x),
        target.size.y / static_cast<float>(size.y)
    });
    sprite.setPosition(target.position);
    window.draw(sprite);
}
}

CafeteriaInterior::CafeteriaInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/cafeteria.json"));

    sf::Image cafeteriaSheet;
    if (cafeteriaSheet.loadFromFile(cls::resolveAssetPath("assets/image/scenery/cateen.png"))) {
        mCounterLoaded = loadMaskedTexture(cafeteriaSheet, mCounterTexture, kCounterSource);
        mSideCounterLoaded = loadMaskedTexture(cafeteriaSheet, mSideCounterTexture, kSideCounterSource);
        for (std::size_t i = 0; i < mTableTextures.size(); ++i)
            mTableLoaded[i] = loadMaskedTexture(cafeteriaSheet, mTableTextures[i], kTableSources[i]);
    }
    mFloorLoaded = mFloorTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/floor.png"));
    if (mFloorLoaded) mFloorTexture.setSmooth(false);

    updateInteractionArea("cafeteria_counter", kCounterArea);
    for (std::size_t i = 0; i < kTableAreas.size(); ++i)
        updateInteractionArea("cafeteria_table_" + std::to_string(i), kTableAreas[i]);

    initObstaclesFromInteractions();
}

void CafeteriaInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(150, 114, 72));
    if (mFloorLoaded)
        drawFittedSprite(window, mFloorTexture, kFloorArea);

    if (mCounterLoaded) {
        drawFittedSprite(window, mCounterTexture, kMainCounterArea);
    } else {
        sf::RectangleShape counter(kMainCounterArea.size);
        counter.setPosition(kMainCounterArea.position);
        counter.setFillColor(sf::Color(188, 188, 184));
        window.draw(counter);
    }

    if (mSideCounterLoaded) {
        drawFittedSprite(window, mSideCounterTexture, kSideCounterArea);
    } else {
        sf::RectangleShape sideCounter(kSideCounterArea.size);
        sideCounter.setPosition(kSideCounterArea.position);
        sideCounter.setFillColor(sf::Color(188, 188, 184));
        window.draw(sideCounter);
    }

    for (std::size_t i = 0; i < kTableAreas.size(); ++i) {
        if (mTableLoaded[i]) {
            drawFittedSprite(window, mTableTextures[i], kTableAreas[i]);
        } else {
            sf::RectangleShape table(kTableAreas[i].size);
            table.setPosition(kTableAreas[i].position);
            table.setFillColor(sf::Color(160, 106, 58));
            window.draw(table);
        }
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> CafeteriaInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Cafeteria, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
