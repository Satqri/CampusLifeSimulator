#include "map/LibraryInterior.h"
#include "utils/AssetPath.h"
#include <filesystem>
#include <string>

LibraryInterior::LibraryInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/library.json"));

    if (mBookshelfTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/bookshelf.png")))
        mBookshelfSprite = std::make_unique<sf::Sprite>(mBookshelfTexture);

    // 碰撞区适配精灵实际尺寸（书架精灵 92×93，仅顶部一个）
    if (mBookshelfSprite) {
        const auto s = mBookshelfTexture.getSize();
        const float h = s.y * 92.0f / static_cast<float>(s.x);
        for (int i = 0; i < 4; ++i)
            updateInteractionArea("library_shelf_" + std::to_string(i),
                sf::FloatRect({72.0f + i * 220.0f, 92.0f}, {92.0f, h}));
    }

    initObstaclesFromInteractions();
}

void LibraryInterior::render(sf::RenderWindow& window) {
    drawRoomFrame(window, sf::Color(92, 78, 56));

    // 书架 x4
    for (int i = 0; i < 4; ++i) {
        if (mBookshelfSprite) {
            const auto size = mBookshelfTexture.getSize();
            const float scale = 92.0f / static_cast<float>(size.x);
            sf::Sprite s(*mBookshelfSprite);
            s.setScale({scale, scale});
            s.setPosition({72.0f + i * 220.0f, 92.0f});
            window.draw(s);
        } else {
            sf::RectangleShape shelf({92.0f, 330.0f});
            shelf.setPosition({72.0f + i * 220.0f, 92.0f});
            shelf.setFillColor(sf::Color(82, 58, 36));
            window.draw(shelf);
        }
    }

    // 桌子
    sf::RectangleShape table({180.0f, 64.0f});
    table.setPosition({390.0f, 260.0f});
    table.setFillColor(sf::Color(132, 92, 52));
    window.draw(table);

    // 台灯 x6
    for (int i = 0; i < 6; ++i) {
        sf::RectangleShape lamp({12.0f, 22.0f});
        lamp.setPosition({226.0f + i * 100.0f, 154.0f + (i % 2) * 180.0f});
        lamp.setFillColor(sf::Color(232, 202, 108));
        window.draw(lamp);
    }

    drawExitPortal(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> LibraryInterior::getPortals() const {
    return {
        MapPortal{sf::FloatRect({410.0f, 482.0f}, {140.0f, 42.0f}), CampusPlace::Campus,
                  SceneBackgroundType::Library, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
