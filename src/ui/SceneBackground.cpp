#include "ui/SceneBackground.h"
#include "utils/AssetPath.h"

#include <cmath>
#include <filesystem>

SceneBackground::SceneBackground() {
    load(SceneBackgroundType::Dormitory, "assets/backgrounds/dormitory.png");
    load(SceneBackgroundType::Gym, "assets/backgrounds/gym.png");
    load(SceneBackgroundType::Library, "assets/backgrounds/library.png");
    load(SceneBackgroundType::Classroom, "assets/backgrounds/classroom.png");
    load(SceneBackgroundType::Cafeteria, "assets/backgrounds/cafeteria.png");
    load(SceneBackgroundType::Store, "assets/backgrounds/store.png");
}

void SceneBackground::update(float deltaTime) {
    elapsedTime += deltaTime;
}

void SceneBackground::render(sf::RenderWindow& window) {
    render(window, SceneBackgroundType::Dormitory);
}

void SceneBackground::render(sf::RenderWindow& window, SceneBackgroundType type, sf::Color tint) {
    const int i = index(type);
    if (sprites[i]) {
        const auto size = textures[i].getSize();
        if (size.x > 0 && size.y > 0) {
            sprites[i]->setScale({
                960.0f / static_cast<float>(size.x),
                540.0f / static_cast<float>(size.y)
            });
        }
        window.draw(*sprites[i]);
    } else {
        renderFallback(window, type);
    }

    const float breathe = (std::sin(elapsedTime * 0.75f) + 1.0f) * 0.5f;
    sf::RectangleShape shade({960.0f, 540.0f});
    shade.setFillColor(sf::Color(0, 12, 16, static_cast<std::uint8_t>(26 + breathe * 12)));
    window.draw(shade);

    if (tint.a > 0) {
        sf::RectangleShape overlay({960.0f, 540.0f});
        overlay.setFillColor(tint);
        window.draw(overlay);
    }
}

void SceneBackground::renderFallback(sf::RenderWindow& window, SceneBackgroundType type) const {
    sf::RectangleShape floor({960.0f, 540.0f});
    floor.setFillColor(type == SceneBackgroundType::Gym
        ? sf::Color(86, 96, 90)
        : type == SceneBackgroundType::Store
            ? sf::Color(78, 86, 94)
            : sf::Color(30, 40, 45));
    window.draw(floor);

    if (type != SceneBackgroundType::Gym && type != SceneBackgroundType::Store) return;

    sf::RectangleShape mat({720.0f, 300.0f});
    mat.setPosition({120.0f, 118.0f});
    mat.setFillColor(sf::Color(54, 70, 67));
    mat.setOutlineColor(sf::Color(138, 164, 150));
    mat.setOutlineThickness(4.0f);
    window.draw(mat);

    for (const sf::Vector2f pos : {sf::Vector2f{166.0f, 174.0f}, sf::Vector2f{596.0f, 174.0f}}) {
        sf::RectangleShape treadmill({150.0f, 56.0f});
        treadmill.setPosition(pos);
        treadmill.setFillColor(sf::Color(34, 42, 44));
        treadmill.setOutlineColor(sf::Color(150, 176, 166));
        treadmill.setOutlineThickness(3.0f);
        window.draw(treadmill);

        sf::RectangleShape belt({106.0f, 24.0f});
        belt.setPosition({pos.x + 22.0f, pos.y + 16.0f});
        belt.setFillColor(sf::Color(22, 25, 28));
        window.draw(belt);
    }

    for (const sf::Vector2f pos : {sf::Vector2f{178.0f, 326.0f}, sf::Vector2f{348.0f, 326.0f},
                                  sf::Vector2f{518.0f, 326.0f}, sf::Vector2f{688.0f, 326.0f}}) {
        sf::RectangleShape bar({92.0f, 10.0f});
        bar.setPosition({pos.x, pos.y + 16.0f});
        bar.setFillColor(sf::Color(196, 200, 188));
        window.draw(bar);

        sf::RectangleShape leftPlate({16.0f, 34.0f});
        leftPlate.setPosition({pos.x - 12.0f, pos.y + 4.0f});
        leftPlate.setFillColor(sf::Color(46, 52, 52));
        window.draw(leftPlate);

        sf::RectangleShape rightPlate({16.0f, 34.0f});
        rightPlate.setPosition({pos.x + 88.0f, pos.y + 4.0f});
        rightPlate.setFillColor(sf::Color(46, 52, 52));
        window.draw(rightPlate);
    }

    if (type != SceneBackgroundType::Store) return;

    sf::RectangleShape aisle({700.0f, 220.0f});
    aisle.setPosition({130.0f, 124.0f});
    aisle.setFillColor(sf::Color(102, 112, 122));
    aisle.setOutlineColor(sf::Color(168, 182, 194));
    aisle.setOutlineThickness(3.0f);
    window.draw(aisle);

    for (int i = 0; i < 3; ++i) {
        sf::RectangleShape shelf({156.0f, 40.0f});
        shelf.setPosition({170.0f + i * 210.0f, 160.0f});
        shelf.setFillColor(sf::Color(188, 170, 118));
        window.draw(shelf);
    }

    sf::RectangleShape counter({210.0f, 70.0f});
    counter.setPosition({640.0f, 360.0f});
    counter.setFillColor(sf::Color(124, 86, 62));
    window.draw(counter);

    sf::RectangleShape heater({84.0f, 64.0f});
    heater.setPosition({180.0f, 356.0f});
    heater.setFillColor(sf::Color(214, 214, 220));
    window.draw(heater);
}

void SceneBackground::load(SceneBackgroundType type, const std::string& relativePath) {
    const int i = index(type);
    const std::string resolvedPath = cls::resolveAssetPath(relativePath);
    if (!std::filesystem::exists(resolvedPath)) {
        return;
    }
    if (textures[i].loadFromFile(resolvedPath)) {
        sprites[i] = std::make_unique<sf::Sprite>(textures[i]);
    }
}

int SceneBackground::index(SceneBackgroundType type) const {
    switch (type) {
        case SceneBackgroundType::Dormitory: return 0;
        case SceneBackgroundType::Gym: return 1;
        case SceneBackgroundType::Library: return 2;
        case SceneBackgroundType::Classroom: return 3;
        case SceneBackgroundType::Cafeteria: return 4;
        case SceneBackgroundType::Store: return 5;
    }
    return 0;
}
