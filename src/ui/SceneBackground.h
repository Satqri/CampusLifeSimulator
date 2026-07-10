#ifndef CLS_UI_SCENE_BACKGROUND_H
#define CLS_UI_SCENE_BACKGROUND_H

#include "ui/UIComponent.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

enum class SceneBackgroundType {
    Dormitory,
    Gym,
    Library,
    Classroom,
    Cafeteria,
    Store
};

class SceneBackground : public UIComponent {
public:
    SceneBackground();

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window, SceneBackgroundType type, sf::Color tint = sf::Color(0, 0, 0, 0));

private:
    std::array<sf::Texture, 6> textures;
    std::array<bool, 6> textureLoaded{};
    float elapsedTime = 0.0f;

    void load(SceneBackgroundType type, const std::string& relativePath);
    void renderFallback(sf::RenderWindow& window, SceneBackgroundType type) const;
    int index(SceneBackgroundType type) const;
};

#endif // CLS_UI_SCENE_BACKGROUND_H
