#ifndef CLS_MAP_DORMITORYINTERIOR_H
#define CLS_MAP_DORMITORYINTERIOR_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <memory>

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

private:
    sf::Texture mBackgroundTexture;
    std::unique_ptr<sf::Sprite> mBackgroundSprite;
    sf::Texture mBedTexture;
    sf::Texture mDeskPcTexture;
    sf::Texture mCarpetTexture;
    std::unique_ptr<sf::Sprite> mBedSprite;
    std::unique_ptr<sf::Sprite> mDeskPcSprite;
    std::unique_ptr<sf::Sprite> mCarpetSprite;
};

#endif // CLS_MAP_DORMITORYINTERIOR_H
