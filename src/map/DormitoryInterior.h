#ifndef CLS_MAP_DORMITORYINTERIOR_H
#define CLS_MAP_DORMITORYINTERIOR_H

#include "map/BuildingInterior.h"

class Student;

class DormitoryInterior : public BuildingInterior {
public:
    DormitoryInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Dormitory; }

    void setStudent(Student* s) { student = s; }

private:
    Student* student = nullptr;
};

#endif