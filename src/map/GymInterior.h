#ifndef CLS_MAP_GYMINTERIOR_H
#define CLS_MAP_GYMINTERIOR_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <memory>

class GymInterior : public BuildingInterior {
public:
    GymInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Gym; }

private:
    sf::Texture mTreadmillTexture;
    sf::Texture mDumbbellTexture;
    std::unique_ptr<sf::Sprite> mTreadmillSprite;
    std::unique_ptr<sf::Sprite> mDumbbellSprite;
};

#endif // CLS_MAP_GYMINTERIOR_H
