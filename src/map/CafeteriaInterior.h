#ifndef CLS_MAP_CAFETERIAINTERIOR_H
#define CLS_MAP_CAFETERIAINTERIOR_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <array>

/**
 * @class CafeteriaInterior
 * @brief 食堂室内场景
 */
class CafeteriaInterior : public BuildingInterior {
public:
    CafeteriaInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Cafeteria; }

private:
    sf::Texture mCounterTexture;
    sf::Texture mSideCounterTexture;
    sf::Texture mFloorTexture;
    std::array<sf::Texture, 5> mTableTextures;
    bool mCounterLoaded{false};
    bool mSideCounterLoaded{false};
    bool mFloorLoaded{false};
    std::array<bool, 5> mTableLoaded{};
};

#endif // CLS_MAP_CAFETERIAINTERIOR_H
