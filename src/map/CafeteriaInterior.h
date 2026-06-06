#ifndef CLS_MAP_CAFETERIAINTERIOR_H
#define CLS_MAP_CAFETERIAINTERIOR_H

#include "map/BuildingInterior.h"

class Student;

class CafeteriaInterior : public BuildingInterior {
public:
    CafeteriaInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Cafeteria; }

    void setStudent(Student* s) { student = s; }

private:
    Student* student = nullptr;
};

#endif