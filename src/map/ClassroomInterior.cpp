#include "map/ClassroomInterior.h"
#include <string>

ClassroomInterior::ClassroomInterior() {
    // 黑板
    interactions.push_back(InteractionPoint{
        sf::FloatRect({220.0f, 82.0f}, {520.0f, 58.0f}), "classroom_board", "Look at Board",
        "The blackboard is covered with today's lecture notes. Reviewing them might give you an edge."});

    // 课桌 3 行 × 5 列
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 5; ++col) {
            interactions.push_back(InteractionPoint{
                sf::FloatRect({180.0f + col * 122.0f, 202.0f + row * 72.0f}, {72.0f, 34.0f}),
                "classroom_desk_" + std::to_string(row) + "_" + std::to_string(col),
                "Sit at Desk",
                "A wooden desk. Sitting here and focusing on the lesson could sharpen your academic skills."
            });
        }
    }
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
                  "Campus Square", "The main paths open again; choose where the day goes next."}
    };
}
