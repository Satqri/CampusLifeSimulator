#ifndef CLS_MAP_CONVENIENCESTOREINTERIOR_H
#define CLS_MAP_CONVENIENCESTOREINTERIOR_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <memory>

/**
 * @class ConvenienceStoreInterior
 * @brief 便利店室内场景
 */
class ConvenienceStoreInterior : public BuildingInterior {
public:
    ConvenienceStoreInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Store; }

private:
    sf::Texture mShelfTexture;
    sf::Texture mCashRegisterTexture;
    std::unique_ptr<sf::Sprite> mShelfSprite;
    std::unique_ptr<sf::Sprite> mCashRegisterSprite;
};

#endif // CLS_MAP_CONVENIENCESTOREINTERIOR_H
