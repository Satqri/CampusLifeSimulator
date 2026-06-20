#include "map/GymInterior.h"
#include "utils/AssetPath.h"
#include "core/Localization.h"

#include <string>

GymInterior::GymInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/gym.json"));
    initObstaclesFromInteractions();
}

void GymInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(84, 102, 92));

    sf::RectangleShape rubberZone({760.0f, 300.0f});
    rubberZone.setPosition({100.0f, 106.0f});
    rubberZone.setFillColor(sf::Color(58, 70, 68));
    rubberZone.setOutlineColor(sf::Color(44, 50, 48));
    rubberZone.setOutlineThickness(4.0f);
    window.draw(rubberZone);

    for (const sf::Vector2f pos : {sf::Vector2f{128.0f, 118.0f}, sf::Vector2f{610.0f, 118.0f}}) {
        sf::RectangleShape frame({190.0f, 76.0f});
        frame.setPosition(pos);
        frame.setFillColor(sf::Color(36, 44, 46));
        frame.setOutlineColor(sf::Color(150, 170, 158));
        frame.setOutlineThickness(3.0f);
        window.draw(frame);

        sf::RectangleShape belt({138.0f, 32.0f});
        belt.setPosition({pos.x + 26.0f, pos.y + 22.0f});
        belt.setFillColor(sf::Color(22, 24, 27));
        window.draw(belt);

        sf::RectangleShape console({38.0f, 18.0f});
        console.setPosition({pos.x + 76.0f, pos.y - 8.0f});
        console.setFillColor(sf::Color(96, 142, 132));
        window.draw(console);
    }

    for (int i = 0; i < 4; ++i) {
        const float x = 168.0f + i * 170.0f;
        const float y = 315.0f;

        sf::RectangleShape platform({108.0f, 58.0f});
        platform.setPosition({x, y});
        platform.setFillColor(sf::Color(72, 82, 78));
        platform.setOutlineColor(sf::Color(114, 132, 124));
        platform.setOutlineThickness(2.0f);
        window.draw(platform);

        sf::RectangleShape bar({86.0f, 10.0f});
        bar.setPosition({x + 11.0f, y + 26.0f});
        bar.setFillColor(sf::Color(198, 202, 190));
        window.draw(bar);

        sf::RectangleShape leftPlate({16.0f, 34.0f});
        leftPlate.setPosition({x + 4.0f, y + 14.0f});
        leftPlate.setFillColor(sf::Color(34, 38, 38));
        window.draw(leftPlate);

        sf::RectangleShape rightPlate({16.0f, 34.0f});
        rightPlate.setPosition({x + 88.0f, y + 14.0f});
        rightPlate.setFillColor(sf::Color(34, 38, 38));
        window.draw(rightPlate);
    }

    if (font) {
        drawLabel(window, cls::text("gym.treadmill"), {146.0f, 204.0f});
        drawLabel(window, cls::text("gym.treadmill"), {628.0f, 204.0f});
        drawLabel(window, cls::text("gym.barbells"), {168.0f, 386.0f});
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> GymInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Gym, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("scene.gym.exit")}
    };
}
