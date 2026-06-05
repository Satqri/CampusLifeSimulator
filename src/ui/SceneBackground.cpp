#include "ui/SceneBackground.h"
#include "core/AssetPath.h"

#include <cmath>

SceneBackground::SceneBackground() {
    load(SceneBackgroundType::Dormitory, "assets/backgrounds/dormitory.png");
    load(SceneBackgroundType::Library, "assets/backgrounds/library.png");
    load(SceneBackgroundType::Classroom, "assets/backgrounds/classroom.png");
    load(SceneBackgroundType::Cafeteria, "assets/backgrounds/cafeteria.png");
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
        sf::RectangleShape fallback({960.0f, 540.0f});
        fallback.setFillColor(sf::Color(30, 40, 45));
        window.draw(fallback);
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

void SceneBackground::load(SceneBackgroundType type, const std::string& relativePath) {
    const int i = index(type);
    const std::string resolvedPath = cls::resolveAssetPath(relativePath);
    if (textures[i].loadFromFile(resolvedPath)) {
        sprites[i] = std::make_unique<sf::Sprite>(textures[i]);
    }
}

int SceneBackground::index(SceneBackgroundType type) const {
    switch (type) {
        case SceneBackgroundType::Dormitory: return 0;
        case SceneBackgroundType::Library: return 1;
        case SceneBackgroundType::Classroom: return 2;
        case SceneBackgroundType::Cafeteria: return 3;
    }
    return 0;
}
