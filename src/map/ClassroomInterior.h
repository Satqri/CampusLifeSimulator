#ifndef CLS_MAP_CLASSROOMINTERIOR_H
#define CLS_MAP_CLASSROOMINTERIOR_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <memory>

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

private:
    sf::Texture mBlackboardTexture;
    std::unique_ptr<sf::Sprite> mBlackboardSprite;
};

#endif // CLS_MAP_CLASSROOMINTERIOR_H
