#include "map/CafeteriaInterior.h"
#include "core/AssetPath.h"

CafeteriaInterior::CafeteriaInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/cafeteria.json"));
    initObstaclesFromInteractions();
}

void CafeteriaInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(150, 114, 72));

    sf::RectangleShape counter({760.0f, 70.0f});
    counter.setPosition({100.0f, 94.0f});
    counter.setFillColor(sf::Color(176, 104, 58));
    window.draw(counter);

    for (int i = 0; i < 7; ++i) {
        sf::CircleShape tray(12.0f);
        tray.setPosition({132.0f + i * 100.0f, 116.0f});
        tray.setFillColor(sf::Color(232, 184, 88));
        window.draw(tray);
    }

    for (int i = 0; i < 5; ++i) {
        sf::RectangleShape table({84.0f, 58.0f});
        table.setPosition({130.0f + i * 160.0f, 278.0f});
        table.setFillColor(sf::Color(120, 78, 44));
        window.draw(table);
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
