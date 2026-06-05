#ifndef CLS_MAP_CLASSROOMINTERIOR_H
#define CLS_MAP_CLASSROOMINTERIOR_H

#include "map/BuildingInterior.h"

/**
 * @class ClassroomInterior
 * @brief 教室室内场景
 */
class ClassroomInterior : public BuildingInterior {
public:
    ClassroomInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Classroom; }
};

#endif // CLS_MAP_CLASSROOMINTERIOR_H
