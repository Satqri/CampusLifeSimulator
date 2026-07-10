#ifndef CLS_MAP_CONVENIENCESTOREINTERIOR_H
#define CLS_MAP_CONVENIENCESTOREINTERIOR_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>

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
    sf::Texture mMockupTexture;
    bool mMockupLoaded = false;

    void drawStoreBackground(sf::RenderWindow& window) const;
    void drawMockup(sf::RenderWindow& window) const;
    void drawFallbackScene(sf::RenderWindow& window) const;
};

#endif // CLS_MAP_CONVENIENCESTOREINTERIOR_H
