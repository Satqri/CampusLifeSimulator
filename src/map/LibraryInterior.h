#ifndef CLS_MAP_LIBRARYINTERIOR_H
#define CLS_MAP_LIBRARYINTERIOR_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <vector>

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
    struct AtlasSprite {
        sf::IntRect source;
        sf::Vector2f position;
        sf::Vector2f scale{1.0f, 1.0f};
    };

    void addObject(sf::IntRect source, sf::Vector2f position, sf::Vector2f scale,
                   sf::FloatRect collision);
    void drawAtlasSprite(sf::RenderWindow& window, const AtlasSprite& sprite) const;
    void drawPixelFloor(sf::RenderWindow& window) const;
    void drawBackWall(sf::RenderWindow& window) const;
    void buildLibraryLayout();

    sf::Texture mAtlasTexture;
    std::vector<AtlasSprite> mDecorSprites;
    std::vector<AtlasSprite> mObjectSprites;
};

#endif // CLS_MAP_LIBRARYINTERIOR_H
