#ifndef CLS_MAP_CAFETERIAINTERIOR_H
#define CLS_MAP_CAFETERIAINTERIOR_H

#include "map/BuildingInterior.h"

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
};

#endif // CLS_MAP_CAFETERIAINTERIOR_H
