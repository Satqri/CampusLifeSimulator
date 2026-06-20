#include "map/ConvenienceStoreInterior.h"
#include "utils/AssetPath.h"

ConvenienceStoreInterior::ConvenienceStoreInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/store.json"));
    initObstaclesFromInteractions();
}

void ConvenienceStoreInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(124, 130, 138));

    sf::RectangleShape shelfA({156.0f, 40.0f});
    shelfA.setPosition({170.0f, 160.0f});
    shelfA.setFillColor(sf::Color(188, 170, 118));
    window.draw(shelfA);

    sf::RectangleShape shelfB({156.0f, 40.0f});
    shelfB.setPosition({380.0f, 160.0f});
    shelfB.setFillColor(sf::Color(188, 170, 118));
    window.draw(shelfB);

    sf::RectangleShape fridge({156.0f, 40.0f});
    fridge.setPosition({590.0f, 160.0f});
    fridge.setFillColor(sf::Color(168, 208, 228));
    window.draw(fridge);

    sf::RectangleShape hotWater({84.0f, 64.0f});
    hotWater.setPosition({180.0f, 356.0f});
    hotWater.setFillColor(sf::Color(214, 214, 220));
    window.draw(hotWater);

    sf::RectangleShape counter({210.0f, 70.0f});
    counter.setPosition({640.0f, 360.0f});
    counter.setFillColor(sf::Color(124, 86, 62));
    window.draw(counter);

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
