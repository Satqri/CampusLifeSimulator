#include "map/DormitoryInterior.h"
#include "utils/AssetPath.h"
#include <filesystem>

DormitoryInterior::DormitoryInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/dormitory.json"));

    if (mBedTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/bed.png")))
        mBedSprite = std::make_unique<sf::Sprite>(mBedTexture);
    if (mDeskPcTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/desk_and_computer.png")))
        mDeskPcSprite = std::make_unique<sf::Sprite>(mDeskPcTexture);
    if (mCarpetTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/carpet.png")))
        mCarpetSprite = std::make_unique<sf::Sprite>(mCarpetTexture);

    // 碰撞区适配精灵实际尺寸
    if (mBedSprite) {
        const auto s = mBedTexture.getSize();
        const float scale = 70.0f / static_cast<float>(s.y);
        updateInteractionArea("dormitory_bed", sf::FloatRect({90.0f, 108.0f}, {s.x * scale, 70.0f}));
    }
    if (mDeskPcSprite) {
        const auto s = mDeskPcTexture.getSize();
        const float scale = 150.0f / static_cast<float>(s.x);
        updateInteractionArea("dormitory_desk", sf::FloatRect({636.0f, 112.0f}, {150.0f, s.y * scale}));
    }
    if (mCarpetSprite) {
        const auto s = mCarpetTexture.getSize();
        const float scale = 110.0f / static_cast<float>(s.y);
        const float scaledW = s.x * scale;
        const float offsetX = (230.0f - scaledW) / 2.0f;
        updateInteractionArea("dormitory_rug", sf::FloatRect({365.0f + offsetX, 285.0f}, {scaledW, 110.0f}));
    }

    initObstaclesFromInteractions();
}

void DormitoryInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(158, 110, 68));

    // 床 + 枕头
    if (mBedSprite) {
        const auto size = mBedTexture.getSize();
        const float scale = 70.0f / static_cast<float>(size.y);
        sf::Sprite s(*mBedSprite);
        s.setScale({scale, scale});
        s.setPosition({90.0f, 108.0f});
        window.draw(s);
    } else {
        sf::RectangleShape bed({180.0f, 70.0f});
        bed.setPosition({90.0f, 108.0f});
        bed.setFillColor(sf::Color(88, 148, 142));
        window.draw(bed);

        sf::RectangleShape pillow({44.0f, 58.0f});
        pillow.setPosition({98.0f, 114.0f});
        pillow.setFillColor(sf::Color(230, 218, 180));
        window.draw(pillow);
    }

    // 书桌 + 显示器 + 主机
    if (mDeskPcSprite) {
        const auto size = mDeskPcTexture.getSize();
        const float scale = 150.0f / static_cast<float>(size.x);
        sf::Sprite s(*mDeskPcSprite);
        s.setScale({scale, scale});
        s.setPosition({636.0f, 112.0f});
        window.draw(s);
    } else {
        sf::RectangleShape desk({210.0f, 66.0f});
        desk.setPosition({636.0f, 112.0f});
        desk.setFillColor(sf::Color(120, 78, 44));
        window.draw(desk);

        sf::RectangleShape monitor({86.0f, 42.0f});
        monitor.setPosition({686.0f, 190.0f});
        monitor.setFillColor(sf::Color(32, 48, 62));
        monitor.setOutlineColor(sf::Color(86, 118, 142));
        monitor.setOutlineThickness(3.0f);
        window.draw(monitor);

        sf::RectangleShape console({54.0f, 18.0f});
        console.setPosition({786.0f, 222.0f});
        console.setFillColor(sf::Color(48, 48, 58));
        window.draw(console);
    }

    // 地毯
    if (mCarpetSprite) {
        const auto size = mCarpetTexture.getSize();
        const float scale = 110.0f / static_cast<float>(size.y);
        const float scaledW = size.x * scale;
        const float offsetX = (230.0f - scaledW) / 2.0f;
        sf::Sprite s(*mCarpetSprite);
        s.setScale({scale, scale});
        s.setPosition({365.0f + offsetX, 285.0f});
        window.draw(s);
    } else {
        sf::RectangleShape rug({230.0f, 110.0f});
        rug.setPosition({365.0f, 285.0f});
        rug.setFillColor(sf::Color(72, 126, 116));
        window.draw(rug);
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> DormitoryInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Dormitory, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
