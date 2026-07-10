#include "map/ClassroomInterior.h"
#include "utils/AssetPath.h"
#include <filesystem>
#include <queue>
#include <string>
#include <vector>

namespace {
const sf::FloatRect kBoardArea({70.0f, 70.0f}, {820.0f, 136.0f});
const sf::FloatRect kFloorArea({34.0f, 74.0f}, {892.0f, 444.0f});
const sf::IntRect kDeskSource({75, 80}, {420, 395});
const sf::Vector2f kDeskSize(92.0f, 70.0f);
const float kDeskStartX = 58.0f;
const float kDeskStartY = 250.0f;
const float kDeskStepX = 184.0f;
const float kDeskStepY = 80.0f;
const sf::FloatRect kExitArea({410.0f, 492.0f}, {140.0f, 42.0f});

sf::FloatRect deskArea(int row, int col) {
    return sf::FloatRect(
        {kDeskStartX + static_cast<float>(col) * kDeskStepX,
         kDeskStartY + static_cast<float>(row) * kDeskStepY},
        kDeskSize);
}

bool isDeskSheetBackground(sf::Color color) {
    return color.r >= 88 && color.r <= 160
        && color.g >= 98 && color.g <= 170
        && color.b >= 84 && color.b <= 150
        && color.g >= color.r - 8
        && color.g >= color.b;
}

void removeConnectedDeskBackground(sf::Image& image) {
    const auto size = image.getSize();
    if (size.x == 0 || size.y == 0) return;

    std::vector<bool> visited(static_cast<std::size_t>(size.x) * size.y, false);
    std::queue<sf::Vector2u> pending;

    auto indexOf = [size](sf::Vector2u point) {
        return static_cast<std::size_t>(point.y) * size.x + point.x;
    };

    auto enqueue = [&](sf::Vector2u point) {
        const auto index = indexOf(point);
        if (visited[index] || !isDeskSheetBackground(image.getPixel(point))) return;
        visited[index] = true;
        pending.push(point);
    };

    for (unsigned x = 0; x < size.x; ++x) {
        enqueue({x, 0});
        enqueue({x, size.y - 1});
    }
    for (unsigned y = 0; y < size.y; ++y) {
        enqueue({0, y});
        enqueue({size.x - 1, y});
    }

    while (!pending.empty()) {
        const auto point = pending.front();
        pending.pop();

        auto color = image.getPixel(point);
        color.a = 0;
        image.setPixel(point, color);

        if (point.x > 0) enqueue({point.x - 1, point.y});
        if (point.x + 1 < size.x) enqueue({point.x + 1, point.y});
        if (point.y > 0) enqueue({point.x, point.y - 1});
        if (point.y + 1 < size.y) enqueue({point.x, point.y + 1});
    }
}

bool loadDeskTexture(sf::Texture& texture) {
    sf::Image sheet;
    if (!sheet.loadFromFile(cls::resolveAssetPath("assets/image/scenery/desk.png"))) return false;

    sf::Image crop;
    crop.resize({static_cast<unsigned>(kDeskSource.size.x), static_cast<unsigned>(kDeskSource.size.y)}, sf::Color::Transparent);
    if (!crop.copy(sheet, {0, 0}, kDeskSource)) return false;

    removeConnectedDeskBackground(crop);

    if (!texture.loadFromImage(crop)) return false;
    texture.setSmooth(false);
    return true;
}

void drawFittedSprite(sf::RenderWindow& window, const sf::Texture& texture, sf::FloatRect target) {
    const auto size = texture.getSize();
    if (size.x == 0 || size.y == 0) return;

    sf::Sprite sprite(texture);
    sprite.setScale({
        target.size.x / static_cast<float>(size.x),
        target.size.y / static_cast<float>(size.y)
    });
    sprite.setPosition(target.position);
    window.draw(sprite);
}

void drawClassroomRoomFrame(sf::RenderWindow& window, sf::Color floorColor, const sf::Texture* floorTexture) {
    sf::RectangleShape bg({kRenderWidth, kRenderHeight});
    bg.setFillColor(floorColor);
    window.draw(bg);

    if (floorTexture) {
        drawFittedSprite(window, *floorTexture, kFloorArea);
    } else {
        for (int x = 36; x < 924; x += 42) {
            for (int y = 76; y < 520; y += 42) {
                sf::RectangleShape tile({40.0f, 40.0f});
                tile.setPosition({static_cast<float>(x), static_cast<float>(y)});
                tile.setFillColor(sf::Color(floorColor.r + 8, floorColor.g + 8, floorColor.b + 8, 70));
                window.draw(tile);
            }
        }
    }

    sf::RectangleShape roomFrame({912.0f, 464.0f});
    roomFrame.setPosition({24.0f, 64.0f});
    roomFrame.setFillColor(sf::Color(0, 0, 0, 0));
    roomFrame.setOutlineColor(sf::Color(42, 30, 22));
    roomFrame.setOutlineThickness(10.0f);
    window.draw(roomFrame);
}
}

ClassroomInterior::ClassroomInterior() {
    interactions = loadInteractionsFromJson(
        cls::resolveAssetPath("assets/config/interiors/classroom.json"));

    if (mBlackboardTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/blackboard.png")))
        mBlackboardSprite = std::make_unique<sf::Sprite>(mBlackboardTexture);
    mDeskLoaded = loadDeskTexture(mDeskTexture);
    mFloorLoaded = mFloorTexture.loadFromFile(cls::resolveAssetPath("assets/image/scenery/floor2.png"));
    if (mFloorLoaded) mFloorTexture.setSmooth(false);

    updateInteractionArea("classroom_board", kBoardArea);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 5; ++col) {
            updateInteractionArea("classroom_desk_" + std::to_string(row) + "_" + std::to_string(col),
                deskArea(row, col));
        }
    }

    initObstaclesFromInteractions();
}

void ClassroomInterior::render(sf::RenderWindow& window) {
    drawClassroomRoomFrame(window, sf::Color(116, 126, 112),
        mFloorLoaded ? &mFloorTexture : nullptr);

    if (mBlackboardSprite) {
        const auto size = mBlackboardTexture.getSize();
        const float scaleX = kBoardArea.size.x / static_cast<float>(size.x);
        const float scaleY = kBoardArea.size.y / static_cast<float>(size.y);
        sf::Sprite sprite(*mBlackboardSprite);
        sprite.setScale({scaleX, scaleY});
        sprite.setPosition(kBoardArea.position);
        window.draw(sprite);
    } else {
        sf::RectangleShape board(kBoardArea.size);
        board.setPosition(kBoardArea.position);
        board.setFillColor(sf::Color(34, 78, 68));
        window.draw(board);
    }

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 5; ++col) {
            const auto area = deskArea(row, col);
            if (mDeskLoaded) {
                drawFittedSprite(window, mDeskTexture, area);
            } else {
                sf::RectangleShape desk(area.size);
                desk.setPosition(area.position);
                desk.setFillColor(sf::Color(156, 108, 58));
                window.draw(desk);
            }
        }
    }

    sf::RectangleShape exit(kExitArea.size);
    exit.setPosition(kExitArea.position);
    exit.setFillColor(sf::Color(38, 130, 100, 210));
    window.draw(exit);
    if (font) {
        drawLabel(window, cls::text("map.exit_campus"), {428.0f, 504.0f});
    }
    drawObstacleOutlines(window);
    drawPortalMarkers(window);
}

std::vector<MapPortal> ClassroomInterior::getPortals() const {
    return {
        MapPortal{kExitArea, CampusPlace::Campus,
                  SceneBackgroundType::Classroom, {480.0f, 448.0f},
                  cls::text("notice.campus_square"), cls::text("notice.campus_square.subtitle")}
    };
}
