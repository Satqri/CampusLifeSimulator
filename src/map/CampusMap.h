#ifndef CLS_MAP_CAMPUSMAP_H
#define CLS_MAP_CAMPUSMAP_H

#include "map/BuildingInterior.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <initializer_list>
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
                      const std::string& label, const sf::Sprite* buildingSprite = nullptr);

    sf::Texture outdoorTiles;
    bool outdoorTilesLoaded;
    const TimeSystem* timeSystem = nullptr;

    static constexpr int kBuildingCount = 6;
    std::array<sf::Texture, kBuildingCount> mBuildingTextures;
    std::array<std::unique_ptr<sf::Sprite>, kBuildingCount> mBuildingSprites;
    sf::Texture mLampTexture;
    bool mLampTextureLoaded = false;

    void loadBuildingTexture(int index, const std::string& relativePath);
    const sf::Sprite* getBuildingSprite(CampusPlace place) const;

    void drawGround(sf::RenderWindow& window) const;
    void drawPathSegment(sf::RenderWindow& window, sf::Vector2f start, sf::Vector2f end,
                         float width, sf::Color color) const;
    void drawRoundedPath(sf::RenderWindow& window, std::initializer_list<sf::Vector2f> points,
                         float width, sf::Color color) const;
    void drawPathNetwork(sf::RenderWindow& window) const;
    void drawPlaza(sf::RenderWindow& window) const;
    void drawTree(sf::RenderWindow& window, sf::Vector2f position, float scale) const;
    void drawFlowerBed(sf::RenderWindow& window, sf::Vector2f position, sf::Vector2f size) const;
    void drawBench(sf::RenderWindow& window, sf::Vector2f position) const;
    void drawCampusDetails(sf::RenderWindow& window) const;
    void drawMapTitle(sf::RenderWindow& window) const;
    void drawEllipse(sf::RenderWindow& window, sf::Vector2f center, sf::Vector2f radii,
                     sf::Color color) const;
    void drawPixlabSprite(sf::RenderWindow& window, const sf::IntRect& textureRect,
                          sf::Vector2f position, float scale = 1.0f) const;
    void drawLamp(sf::RenderWindow& window, sf::Vector2f position, bool glow) const;
    void drawTimeLighting(sf::RenderWindow& window) const;
};

#endif // CLS_MAP_CAMPUSMAP_H
