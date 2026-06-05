#ifndef CLS_UI_SCENE_BACKGROUND_H
#define CLS_UI_SCENE_BACKGROUND_H

#include "ui/UIComponent.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include <string>

enum class SceneBackgroundType {
    Dormitory,
    Gym,
    Library,
    Classroom,
    Cafeteria
};

class SceneBackground : public UIComponent {
public:
    SceneBackground();

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window, SceneBackgroundType type, sf::Color tint = sf::Color(0, 0, 0, 0));

private:
    std::array<sf::Texture, 5> textures;
    std::array<std::unique_ptr<sf::Sprite>, 5> sprites;
    float elapsedTime = 0.0f;

    void load(SceneBackgroundType type, const std::string& relativePath);
    void renderFallback(sf::RenderWindow& window, SceneBackgroundType type) const;
    int index(SceneBackgroundType type) const;
};

#endif // CLS_UI_SCENE_BACKGROUND_H
