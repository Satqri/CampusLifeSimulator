#include "map/GymInterior.h"
#include "utils/AssetPath.h"
#include "core/Localization.h"
#include <algorithm>
#include <array>
#include <string>

GymInterior::GymInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/gym.json"));

    if (mTreadmillLeftTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/treadmill_left.png")))
        mTreadmillLeftSprite = std::make_unique<sf::Sprite>(mTreadmillLeftTexture);
    if (mTreadmillRightTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/treadmill_right.png")))
        mTreadmillRightSprite = std::make_unique<sf::Sprite>(mTreadmillRightTexture);

    for (int i = 0; i < 4; ++i) {
        const std::string path = cls::resolveAssetPath("assets/image/scenery/dumbbell_" + std::to_string(i) + ".png");
        if (mDumbbellTextures[i].loadFromFile(path))
            mDumbbellSprites[i] = std::make_unique<sf::Sprite>(mDumbbellTextures[i]);
    }

    if (mFrontDeskTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/front_desk.png")))
        mFrontDeskSprite = std::make_unique<sf::Sprite>(mFrontDeskTexture);

    initObstaclesFromInteractions();
}

void GymInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(84, 102, 92));

    sf::RectangleShape rubberZone({760.0f, 300.0f});
    rubberZone.setPosition({100.0f, 106.0f});
    rubberZone.setFillColor(sf::Color(58, 70, 68));
    rubberZone.setOutlineColor(sf::Color(44, 50, 48));
    rubberZone.setOutlineThickness(4.0f);
    window.draw(rubberZone);

    // 跑步机 x2
    const std::array<std::unique_ptr<sf::Sprite>*, 2> treadmillSprites = {&mTreadmillLeftSprite, &mTreadmillRightSprite};
    const std::array<sf::Vector2f, 2> treadmillPositions = {sf::Vector2f{150.0f, 118.0f}, sf::Vector2f{610.0f, 118.0f}};
    const std::array<sf::Vector2f, 2> treadmillSizes = {sf::Vector2f{150.0f, 180.0f}, sf::Vector2f{150.0f, 180.0f}};
    for (int i = 0; i < 2; ++i) {
        const auto& spritePtr = *treadmillSprites[i];
        const auto boxPos = treadmillPositions[i];
        const auto boxSize = treadmillSizes[i];
        if (spritePtr) {
            const auto size = (i == 0 ? mTreadmillLeftTexture : mTreadmillRightTexture).getSize();
            const float maxWidth = boxSize.x;
            const float maxHeight = boxSize.y;
            const float scale = std::min(maxWidth / static_cast<float>(size.x), maxHeight / static_cast<float>(size.y));
            sf::Sprite s(*spritePtr);
            s.setScale({scale, scale});
            const auto scaledSize = sf::Vector2f(size.x * scale, size.y * scale);
            s.setPosition({boxPos.x + (boxSize.x - scaledSize.x) / 2.0f, boxPos.y + (boxSize.y - scaledSize.y) / 2.0f});
            window.draw(s);
        } else {
            sf::RectangleShape frame(boxSize);
            frame.setPosition(boxPos);
            frame.setFillColor(sf::Color(36, 44, 46));
            frame.setOutlineColor(sf::Color(150, 170, 158));
            frame.setOutlineThickness(3.0f);
            window.draw(frame);

            sf::RectangleShape belt({boxSize.x - 62.0f, 40.0f});
            belt.setPosition({boxPos.x + 30.0f, boxPos.y + 26.0f});
            belt.setFillColor(sf::Color(22, 24, 27));
            window.draw(belt);

            sf::RectangleShape console({38.0f, 18.0f});
            console.setPosition({boxPos.x + 76.0f, boxPos.y - 8.0f});
            console.setFillColor(sf::Color(96, 142, 132));
            window.draw(console);
        }
    }

    // 前台
    const sf::Vector2f frontDeskPos{392.0f, 70.0f};
    const sf::Vector2f frontDeskSize{180.0f, 120.0f};
    if (mFrontDeskSprite) {
        const auto size = mFrontDeskTexture.getSize();
        const float scale = std::min(frontDeskSize.x / static_cast<float>(size.x), frontDeskSize.y / static_cast<float>(size.y));
        sf::Sprite s(*mFrontDeskSprite);
        s.setScale({scale, scale});
        const auto scaledSize = sf::Vector2f(size.x * scale, size.y * scale);
        s.setPosition({frontDeskPos.x + (frontDeskSize.x - scaledSize.x) / 2.0f,
                      frontDeskPos.y + (frontDeskSize.y - scaledSize.y) / 2.0f});
        window.draw(s);
    }

    // 杠铃站 x4
    for (int i = 0; i < 4; ++i) {
        const float x = 168.0f + i * 170.0f;
        const float y = 315.0f;

        if (mDumbbellSprites[i]) {
            const auto size = mDumbbellTextures[i].getSize();
            const float maxWidth = 108.0f;
            const float maxHeight = 58.0f;
            const float scale = std::min(maxWidth / static_cast<float>(size.x), maxHeight / static_cast<float>(size.y));
            sf::Sprite s(*mDumbbellSprites[i]);
            s.setScale({scale, scale});
            const auto scaledSize = sf::Vector2f(size.x * scale, size.y * scale);
            s.setPosition({x + (maxWidth - scaledSize.x) / 2.0f, y + (maxHeight - scaledSize.y) / 2.0f});
            window.draw(s);
        } else {
            sf::RectangleShape platform({108.0f, 58.0f});
            platform.setPosition({x, y});
            platform.setFillColor(sf::Color(72, 82, 78));
            platform.setOutlineColor(sf::Color(114, 132, 124));
            platform.setOutlineThickness(2.0f);
            window.draw(platform);

            sf::RectangleShape bar({86.0f, 10.0f});
            bar.setPosition({x + 11.0f, y + 26.0f});
            bar.setFillColor(sf::Color(198, 202, 190));
            window.draw(bar);

            sf::RectangleShape leftPlate({16.0f, 34.0f});
            leftPlate.setPosition({x + 4.0f, y + 14.0f});
            leftPlate.setFillColor(sf::Color(34, 38, 38));
            window.draw(leftPlate);

            sf::RectangleShape rightPlate({16.0f, 34.0f});
            rightPlate.setPosition({x + 88.0f, y + 14.0f});
            rightPlate.setFillColor(sf::Color(34, 38, 38));
            window.draw(rightPlate);
        }
    }

    if (font) {
        drawLabel(window, cls::text("gym.treadmill"), {168.0f, 204.0f});
        drawLabel(window, cls::text("gym.treadmill"), {628.0f, 204.0f});
        drawLabel(window, cls::text("gym.barbells"), {168.0f, 386.0f});
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> GymInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Gym, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("scene.gym.exit")}
    };
}
