#include "map/ConvenienceStoreInterior.h"
#include "utils/AssetPath.h"
#include <filesystem>

ConvenienceStoreInterior::ConvenienceStoreInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/store.json"));

    if (mShelfTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/shelf.png")))
        mShelfSprite = std::make_unique<sf::Sprite>(mShelfTexture);
    if (mCashRegisterTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/cash_register.png")))
        mCashRegisterSprite = std::make_unique<sf::Sprite>(mCashRegisterTexture);

    // 碰撞区适配精灵实际尺寸
    if (mShelfSprite) {
        const auto s = mShelfTexture.getSize();
        const float h = s.y * 156.0f / static_cast<float>(s.x);
        updateInteractionArea("store_shelf", sf::FloatRect({170.0f, 160.0f}, {156.0f, h}));
        updateInteractionArea("store_drink_fridge", sf::FloatRect({590.0f, 160.0f}, {156.0f, h}));
    }
    if (mCashRegisterSprite) {
        const auto s = mCashRegisterTexture.getSize();
        const float w = s.x * 70.0f / static_cast<float>(s.y);
        updateInteractionArea("store_counter", sf::FloatRect({640.0f, 360.0f}, {w, 70.0f}));
    }

    initObstaclesFromInteractions();
}

void ConvenienceStoreInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(124, 130, 138));

    // 货架 A、B、冰箱
    for (const float x : {170.0f, 380.0f, 590.0f}) {
        if (mShelfSprite) {
            const auto size = mShelfTexture.getSize();
            const float scale = 156.0f / static_cast<float>(size.x);
            sf::Sprite s(*mShelfSprite);
            s.setScale({scale, scale});
            s.setPosition({x, 160.0f});
            window.draw(s);
        } else {
            sf::RectangleShape shelf({156.0f, 40.0f});
            shelf.setPosition({x, 160.0f});
            shelf.setFillColor(x == 590.0f ? sf::Color(168, 208, 228) : sf::Color(188, 170, 118));
            window.draw(shelf);
        }
    }

    // 热水机
    sf::RectangleShape hotWater({84.0f, 64.0f});
    hotWater.setPosition({180.0f, 356.0f});
    hotWater.setFillColor(sf::Color(214, 214, 220));
    window.draw(hotWater);

    // 收银台
    if (mCashRegisterSprite) {
        const auto size = mCashRegisterTexture.getSize();
        const float scale = 70.0f / static_cast<float>(size.y);
        sf::Sprite s(*mCashRegisterSprite);
        s.setScale({scale, scale});
        s.setPosition({640.0f, 360.0f});
        window.draw(s);
    } else {
        sf::RectangleShape counter({210.0f, 70.0f});
        counter.setPosition({640.0f, 360.0f});
        counter.setFillColor(sf::Color(124, 86, 62));
        window.draw(counter);
    }

    drawExitPortal(window);
    drawObstacleOutlines(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> ConvenienceStoreInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Store, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
