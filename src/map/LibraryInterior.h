#ifndef CLS_MAP_LIBRARYINTERIOR_H
#define CLS_MAP_LIBRARYINTERIOR_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <memory>

/**
 * @class LibraryInterior
 * @brief 图书馆室内场景
 */
class LibraryInterior : public BuildingInterior {
public:
    LibraryInterior();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Library; }

private:
    sf::Texture mBookshelfTexture;
    std::unique_ptr<sf::Sprite> mBookshelfSprite;
};

#endif // CLS_MAP_LIBRARYINTERIOR_H
