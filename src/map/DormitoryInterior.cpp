#include "map/DormitoryInterior.h"
#include "utils/AssetPath.h"
#include <filesystem>

DormitoryInterior::DormitoryInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/dormitory.json"));

    // 优先加载用户自定义床图（bed_custom.png），否则回退到 bed.png
    const std::string bedCustomPath = cls::resolveAssetPath("assets/image/scenery/bed_custom.png");
    const std::string bedDefaultPath = cls::resolveAssetPath("assets/image/scenery/bed.png");
    if (std::filesystem::exists(bedCustomPath) && mBedTexture.loadFromFile(bedCustomPath)) {
        mBedSprite = std::make_unique<sf::Sprite>(mBedTexture);
    } else if (mBedTexture.loadFromFile(bedDefaultPath)) {
        mBedSprite = std::make_unique<sf::Sprite>(mBedTexture);
    }
    const std::string bgCustomPath = R"(C:\Users\sheng\Desktop\地板(1).png)";
    if (std::filesystem::exists(bgCustomPath) && mBackgroundTexture.loadFromFile(bgCustomPath)) {
        mBackgroundSprite = std::make_unique<sf::Sprite>(mBackgroundTexture);
    }

    if (mDeskPcTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/desk_and_computer.png")))
        mDeskPcSprite = std::make_unique<sf::Sprite>(mDeskPcTexture);
    if (mCarpetTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/dorm_carpet.png")))
        mCarpetSprite = std::make_unique<sf::Sprite>(mCarpetTexture);

    // 碰撞区适配精灵实际尺寸
    const sf::Vector2f bedPos = {410.0f, 175.0f}; // 床再往下移动20f
    if (mBedSprite) {
        const auto s = mBedTexture.getSize();
        const float baseScale = 70.0f / static_cast<float>(s.y);
        const float scale = baseScale * 2.0f; // 保持 2 倍缩放
        updateInteractionArea("dormitory_bed", sf::FloatRect(bedPos, {s.x * scale, s.y * scale}));
    }
    if (mCarpetSprite) {
        const auto s = mCarpetTexture.getSize();
        const float scale = 110.0f / static_cast<float>(s.y);
        const float scaledW = s.x * scale;
        const sf::Vector2f carpetPos = {bedPos.x - 210.0f, bedPos.y - 55.0f}; // 地毯再向左移动30f，并向上移动50f
        updateInteractionArea("dormitory_rug", sf::FloatRect(carpetPos, {scaledW, 110.0f}));
    }
    if (mDeskPcSprite) {
        const auto s = mDeskPcTexture.getSize();
        const float scale = 150.0f / static_cast<float>(s.x);
        updateInteractionArea("dormitory_desk", sf::FloatRect({636.0f, 112.0f}, {150.0f, s.y * scale}));
    }

    initObstaclesFromInteractions();
}

void DormitoryInterior::render(sf::RenderWindow& window) {
    if (mBackgroundSprite) {
        sf::Sprite bg(*mBackgroundSprite);
        const auto bgSize = mBackgroundTexture.getSize();
        if (bgSize.x > 0 && bgSize.y > 0) {
            const float scaleX = 876.0f / static_cast<float>(bgSize.x);
            const float scaleY = 424.0f / static_cast<float>(bgSize.y);
            bg.setScale({scaleX, scaleY});
        }
        bg.setPosition({42.0f, 72.0f});
        window.draw(bg);

        sf::RectangleShape roomFrame({876.0f, 424.0f});
        roomFrame.setPosition({42.0f, 72.0f});
        roomFrame.setFillColor(sf::Color(0, 0, 0, 0));
        roomFrame.setOutlineColor(sf::Color(42, 30, 22));
        roomFrame.setOutlineThickness(12.0f);
        window.draw(roomFrame);
    } else {
        drawRoomFrame(window, sf::Color(158, 110, 68));
    }

    // 床 + 枕头
    const sf::Vector2f bedPos = {410.0f, 175.0f}; // 床再往下移动20f
    if (mBedSprite) {
        const auto size = mBedTexture.getSize();
        const float baseScale = 70.0f / static_cast<float>(size.y);
        const float scale = baseScale * 2.0f; // 保持 2 倍缩放
        sf::Sprite s(*mBedSprite);
        s.setScale({scale, scale});
        s.setPosition(bedPos);
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
        const sf::Vector2f carpetPos = {bedPos.x - 210.0f, bedPos.y - 55.0f}; // 地毯再向左移动30f，并向上移动50f
        sf::Sprite s(*mCarpetSprite);
        s.setScale({scale, scale});
        s.setPosition(carpetPos);
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
