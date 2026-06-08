#include "map/LibraryInterior.h"

LibraryInterior::LibraryInterior() {
    interactions = {
        {sf::FloatRect({72.0f, 92.0f}, {92.0f, 330.0f}),          "library_shelf_0", "Browse Shelf A",
         "Row upon row of reference books. Somewhere in these pages might be the answer you need."},
        {sf::FloatRect({72.0f + 220.0f, 92.0f}, {92.0f, 330.0f}), "library_shelf_1", "Browse Shelf B",
         "Literature and philosophy. A different kind of wisdom waits between these covers."},
        {sf::FloatRect({72.0f + 440.0f, 92.0f}, {92.0f, 330.0f}), "library_shelf_2", "Browse Shelf C",
         "Science and mathematics. Formulas and theories that could boost your exam performance."},
        {sf::FloatRect({72.0f + 660.0f, 92.0f}, {92.0f, 330.0f}), "library_shelf_3", "Browse Shelf D",
         "History and social studies. Understanding the past might help navigate the present."},
        {sf::FloatRect({390.0f, 260.0f}, {180.0f, 64.0f}),         "library_table",   "Read at Table",
         "A large reading table. A perfect spot to spread out your materials and focus."},
    };
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
