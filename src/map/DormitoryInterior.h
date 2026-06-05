#ifndef CLS_MAP_DORMITORYINTERIOR_H
#define CLS_MAP_DORMITORYINTERIOR_H

#include "map/BuildingInterior.h"

/**
 * @class DormitoryInterior
 * @brief 宿舍室内场景
 */
class DormitoryInterior : public BuildingInterior {
public:
    DormitoryInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Dormitory; }
};

#endif // CLS_MAP_DORMITORYINTERIOR_H
