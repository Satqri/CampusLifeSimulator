#ifndef CLS_MAP_CAMPUSMAP_H
#define CLS_MAP_CAMPUSMAP_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <memory>

class Player;
class TimeSystem;

/**
 * @class CampusMap
 * @brief 校园室外地图 — 绿地、道路、建筑入口
 */
class CampusMap : public BuildingInterior {
public:
    CampusMap();

    void render(sf::RenderWindow& window) override;
    std::vector<MapPortal> getPortals() const override;
    CampusPlace getPlace() const override { return CampusPlace::Campus; }
    void setTimeSystem(const TimeSystem* time) { timeSystem = time; }

    /** @brief 渲染玩家（校园地图自行管理玩家渲染） */
    void renderPlayer(sf::RenderWindow& window, Player& player);

private:
    void drawBuilding(sf::RenderWindow& window, const MapPortal& portal,
                      const std::string& label, sf::Color body, sf::Color roof);

    sf::Texture outdoorTiles;
    bool outdoorTilesLoaded;
    const TimeSystem* timeSystem = nullptr;

    void drawPixlabSprite(sf::RenderWindow& window, const sf::IntRect& textureRect,
                          sf::Vector2f position, float scale = 1.0f);
    void drawLamp(sf::RenderWindow& window, sf::Vector2f position, bool glow) const;
    void drawTimeLighting(sf::RenderWindow& window) const;
};

#endif // CLS_MAP_CAMPUSMAP_H
