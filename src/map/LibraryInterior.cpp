#include "map/LibraryInterior.h"
#include "utils/AssetPath.h"

LibraryInterior::LibraryInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/library.json"));
    initObstaclesFromInteractions();
}

void LibraryInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(92, 78, 56));

    for (int i = 0; i < 4; ++i) {
        sf::RectangleShape shelf({92.0f, 330.0f});
        shelf.setPosition({72.0f + i * 220.0f, 92.0f});
        shelf.setFillColor(sf::Color(82, 58, 36));
        window.draw(shelf);
    }

    sf::RectangleShape table({180.0f, 64.0f});
    table.setPosition({390.0f, 260.0f});
    table.setFillColor(sf::Color(132, 92, 52));
    window.draw(table);

    for (int i = 0; i < 6; ++i) {
        sf::RectangleShape lamp({12.0f, 22.0f});
        lamp.setPosition({226.0f + i * 100.0f, 154.0f + (i % 2) * 180.0f});
        lamp.setFillColor(sf::Color(232, 202, 108));
        window.draw(lamp);
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
