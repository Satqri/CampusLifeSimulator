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
    sf::Texture mFloorTexture;
    sf::Texture mTreadmillLeftTexture;
    sf::Texture mTreadmillRightTexture;
    sf::Texture mDumbbellTextures[4];
    sf::Texture mFrontDeskTexture;
    std::unique_ptr<sf::Sprite> mFloorSprite;
    std::unique_ptr<sf::Sprite> mTreadmillLeftSprite;
    std::unique_ptr<sf::Sprite> mTreadmillRightSprite;
    std::unique_ptr<sf::Sprite> mDumbbellSprites[4];
    std::unique_ptr<sf::Sprite> mFrontDeskSprite;
};

#endif // CLS_MAP_GYMINTERIOR_H
