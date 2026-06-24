#include "map/CafeteriaInterior.h"
#include "utils/AssetPath.h"
#include <filesystem>

CafeteriaInterior::CafeteriaInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/cafeteria.json"));

    if (mCounterTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/shelf.png")))
        mCounterSprite = std::make_unique<sf::Sprite>(mCounterTexture);

    // 碰撞区适配精灵实际尺寸（柜台 354×70，居中）
    if (mCounterSprite) {
        const auto s = mCounterTexture.getSize();
        const float scale = 70.0f / static_cast<float>(s.y);
        const float scaledW = s.x * scale;
        const float offsetX = (760.0f - scaledW) / 2.0f;
        updateInteractionArea("cafeteria_counter",
            sf::FloatRect({100.0f + offsetX, 94.0f}, {scaledW, 70.0f}));
    }

    initObstaclesFromInteractions();
}

void CafeteriaInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(150, 114, 72));

    // 柜台
    if (mCounterSprite) {
        const auto size = mCounterTexture.getSize();
        const float scale = 70.0f / static_cast<float>(size.y);
        const float scaledW = size.x * scale;
        const float offsetX = (760.0f - scaledW) / 2.0f;
        sf::Sprite s(*mCounterSprite);
        s.setScale({scale, scale});
        s.setPosition({100.0f + offsetX, 94.0f});
        window.draw(s);
    } else {
        sf::RectangleShape counter({760.0f, 70.0f});
        counter.setPosition({100.0f, 94.0f});
        counter.setFillColor(sf::Color(176, 104, 58));
        window.draw(counter);
    }

    // 餐盘 x7
    for (int i = 0; i < 7; ++i) {
        sf::CircleShape tray(12.0f);
        tray.setPosition({132.0f + i * 100.0f, 116.0f});
        tray.setFillColor(sf::Color(232, 184, 88));
        window.draw(tray);
    }

    // 餐桌 x5
    for (int i = 0; i < 5; ++i) {
        sf::RectangleShape table({84.0f, 58.0f});
        table.setPosition({130.0f + i * 160.0f, 278.0f});
        table.setFillColor(sf::Color(120, 78, 44));
        window.draw(table);
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> CafeteriaInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Cafeteria, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
