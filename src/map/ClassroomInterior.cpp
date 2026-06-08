#include "map/ClassroomInterior.h"
#include "core/AssetPath.h"

ClassroomInterior::ClassroomInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/classroom.json"));
}

void ClassroomInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(116, 126, 112));

    sf::RectangleShape board({520.0f, 58.0f});
    board.setPosition({220.0f, 82.0f});
    board.setFillColor(sf::Color(34, 78, 68));
    window.draw(board);

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 5; ++col) {
            sf::RectangleShape desk({72.0f, 34.0f});
            desk.setPosition({180.0f + col * 122.0f, 202.0f + row * 72.0f});
            desk.setFillColor(sf::Color(156, 108, 58));
            window.draw(desk);
        }
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> ClassroomInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Classroom, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
