#ifndef CLS_MINIGAME_MORNINGRUSHRENDERER_H
#define CLS_MINIGAME_MORNINGRUSHRENDERER_H

#include "minigame/MorningRushGame.h"
#include "ui/UIComponent.h"
#include <SFML/Graphics.hpp>

class MorningRushRenderer : public UIComponent {
public:
    MorningRushRenderer(sf::Font& font, const MorningRushGame& game);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    void drawPixelFrame(sf::RenderWindow& window, sf::Vector2f position,
                        sf::Vector2f size, sf::Color fill, sf::Color outline) const;
    void drawPixelBlock(sf::RenderWindow& window, sf::Vector2f position,
                        sf::Vector2f size, sf::Color color) const;
    void drawTile(sf::RenderWindow& window, const sf::Texture& texture,
                  sf::IntRect rect, sf::Vector2f position, float scale) const;
    void drawScene(sf::RenderWindow& window) const;
    void drawBackground(sf::RenderWindow& window) const;
    void drawGround(sf::RenderWindow& window) const;
    void drawPlatforms(sf::RenderWindow& window) const;
    void drawCampusDecor(sf::RenderWindow& window) const;
    void drawFinishGate(sf::RenderWindow& window) const;
    void drawPickups(sf::RenderWindow& window) const;
    void drawHud(sf::RenderWindow& window) const;
    void drawRunner(sf::RenderWindow& window) const;
    void drawActionEffect(sf::RenderWindow& window, RushAction action,
                          sf::Vector2f runnerFoot, float lift) const;
    void drawObstacle(sf::RenderWindow& window, const MorningRushObstacle& obstacle) const;
    void drawSchoolTile(sf::RenderWindow& window, int col, int row,
                        sf::Vector2f position, float scale) const;
    void drawLuckBanner(sf::RenderWindow& window) const;
    void drawFinal(sf::RenderWindow& window) const;
    void drawText(sf::RenderWindow& window, const std::string& text,
                  sf::Vector2f position, unsigned int size, sf::Color color) const;

    sf::Font& font;
    const MorningRushGame& game;
    sf::Texture tilemapTexture;
    sf::Texture backgroundTexture;
    sf::Texture playerTexture;
    sf::Texture adventureBlueBackgroundTexture;
    sf::Texture adventureGreenBackgroundTexture;
    sf::Texture adventureTerrainTexture;
    sf::Texture adventureEndTexture;
    sf::Texture schoolTilesetTexture;
    sf::Texture studentActionTexture;
    sf::Texture studentActionLeftTexture;
    sf::Texture studentActionRightTexture;
    sf::Texture campusGateBackgroundTexture;
    sf::Texture campusPathBackgroundTexture;
    sf::Texture wetFloorSignTexture;
    sf::Texture trafficConeTexture;
    sf::Texture recyclingBinTexture;
    sf::Texture campusPlanterTexture;
    sf::Texture crowdBarrierTexture;
    sf::Texture bicyclePickupTexture;
    sf::Texture greenGroundTopTexture;
    sf::Texture greenGroundBodyTexture;
    sf::Texture greenPlatformTexture;
    sf::Texture greenBoxTexture;
    sf::Texture greenRampLeftTexture;
    sf::Texture greenRampRightTexture;
    sf::Texture greenHalfPipeTexture;
    sf::Texture greenFountainLowTexture;
    sf::Texture greenFountainTallTexture;
    sf::Texture greenFenceLeftTexture;
    sf::Texture greenFenceRightTexture;
    bool assetsLoaded = false;
    bool adventureBackgroundLoaded = false;
    bool adventureTerrainLoaded = false;
    bool adventureEndLoaded = false;
    bool schoolTilesetLoaded = false;
    bool studentActionLoaded = false;
    bool studentActionDirectionalLoaded = false;
    bool campusBackgroundLoaded = false;
    bool wetFloorSignLoaded = false;
    bool trafficConeLoaded = false;
    bool recyclingBinLoaded = false;
    bool campusPlanterLoaded = false;
    bool crowdBarrierLoaded = false;
    bool bicyclePickupLoaded = false;
    bool greenZoneTilesLoaded = false;
    bool greenZoneObjectsLoaded = false;
};

#endif
