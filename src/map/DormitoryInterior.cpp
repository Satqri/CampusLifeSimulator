#include "map/DormitoryInterior.h"
#include "core/AssetPath.h"

DormitoryInterior::DormitoryInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/dormitory.json"));
    initObstaclesFromInteractions();
}

void DormitoryInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(158, 110, 68));

    sf::RectangleShape bed({180.0f, 70.0f});
    bed.setPosition({90.0f, 108.0f});
    bed.setFillColor(sf::Color(88, 148, 142));
    window.draw(bed);

    sf::RectangleShape pillow({44.0f, 58.0f});
    pillow.setPosition({98.0f, 114.0f});
    pillow.setFillColor(sf::Color(230, 218, 180));
    window.draw(pillow);

    sf::RectangleShape desk({210.0f, 66.0f});
    desk.setPosition({636.0f, 112.0f});
    desk.setFillColor(sf::Color(120, 78, 44));
    window.draw(desk);

    sf::RectangleShape monitor({86.0f, 42.0f});
    monitor.setPosition({686.0f, 190.0f});
    monitor.setFillColor(sf::Color(32, 48, 62));
    monitor.setOutlineColor(sf::Color(86, 118, 142));
    monitor.setOutlineThickness(3.0f);
    window.draw(monitor);

    sf::RectangleShape console({54.0f, 18.0f});
    console.setPosition({786.0f, 222.0f});
    console.setFillColor(sf::Color(48, 48, 58));
    window.draw(console);

    sf::RectangleShape rug({230.0f, 110.0f});
    rug.setPosition({365.0f, 285.0f});
    rug.setFillColor(sf::Color(72, 126, 116));
    window.draw(rug);

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> DormitoryInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Dormitory, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
