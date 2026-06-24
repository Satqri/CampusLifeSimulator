#include "map/ClassroomInterior.h"
#include "utils/AssetPath.h"
#include <filesystem>

ClassroomInterior::ClassroomInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/classroom.json"));

    if (mBlackboardTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/blackboard.png")))
        mBlackboardSprite = std::make_unique<sf::Sprite>(mBlackboardTexture);

    // 碰撞区适配精灵实际尺寸（黑板 250×150，居中）
    if (mBlackboardSprite) {
        const auto s = mBlackboardTexture.getSize();
        const float scale = 150.0f / static_cast<float>(s.y);
        const float scaledW = s.x * scale;
        const float offsetX = (520.0f - scaledW) / 2.0f;
        updateInteractionArea("classroom_board",
            sf::FloatRect({220.0f + offsetX, 82.0f}, {scaledW, 150.0f}));
    }

    initObstaclesFromInteractions();
}

void ClassroomInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(116, 126, 112));

    // 黑板
    if (mBlackboardSprite) {
        const auto size = mBlackboardTexture.getSize();
        const float scale = 150.0f / static_cast<float>(size.y);
        const float scaledW = size.x * scale;
        const float offsetX = (520.0f - scaledW) / 2.0f;
        sf::Sprite s(*mBlackboardSprite);
        s.setScale({scale, scale});
        s.setPosition({220.0f + offsetX, 82.0f});
        window.draw(s);
    } else {
        sf::RectangleShape board({520.0f, 58.0f});
        board.setPosition({220.0f, 82.0f});
        board.setFillColor(sf::Color(34, 78, 68));
        window.draw(board);
    }

    // 课桌 3x5
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 5; ++col) {
            sf::RectangleShape desk({72.0f, 34.0f});
            desk.setPosition({180.0f + col * 122.0f, 202.0f + row * 72.0f});
            desk.setFillColor(sf::Color(156, 108, 58));
            window.draw(desk);
        }
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> ClassroomInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Classroom, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
