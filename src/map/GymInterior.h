#ifndef CLS_MAP_GYMINTERIOR_H
#define CLS_MAP_GYMINTERIOR_H

#include "map/BuildingInterior.h"

class GymInterior : public BuildingInterior {
public:
    GymInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Gym; }
};

#endif // CLS_MAP_GYMINTERIOR_H
