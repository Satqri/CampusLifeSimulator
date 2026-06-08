#include "map/BuildingInterior.h"
#include "core/Localization.h"
#include "core/TextUtils.h"
#include "entity/Player.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

void BuildingInterior::clampPlayer(Player& player) const {
    float minX = kPlayerHalfSize;
    float maxX = kRenderWidth - kPlayerHalfSize;
    float minY = 42.0f + kPlayerHalfSize;
    float maxY = kRenderHeight - kPlayerHalfSize;

    if (getPlace() != CampusPlace::Campus) {
        minX = 54.0f + kPlayerHalfSize;
        maxX = 900.0f - kPlayerHalfSize;
        minY = 84.0f + kPlayerHalfSize;
        maxY = 524.0f - kPlayerHalfSize;
    }

    const sf::Vector2f pos = player.getPosition();
    const float clampedX = std::clamp(pos.x, minX, maxX);
    const float clampedY = std::clamp(pos.y, minY, maxY);
    if (clampedX != pos.x || clampedY != pos.y) {
        player.setPosition(clampedX, clampedY);
        player.stopMovement();
    }
}

void BuildingInterior::drawRoomFrame(sf::RenderWindow& window, sf::Color floorColor) const {
    sf::RectangleShape bg({kRenderWidth, kRenderHeight});
    bg.setFillColor(floorColor);
    window.draw(bg);

    sf::RectangleShape roomFrame({876.0f, 424.0f});
    roomFrame.setPosition({42.0f, 72.0f});
    roomFrame.setFillColor(sf::Color(0, 0, 0, 0));
    roomFrame.setOutlineColor(sf::Color(42, 30, 22));
    roomFrame.setOutlineThickness(12.0f);
    window.draw(roomFrame);

    for (int x = 54; x < 900; x += 42) {
        for (int y = 84; y < 486; y += 42) {
            sf::RectangleShape tile({40.0f, 40.0f});
            tile.setPosition({static_cast<float>(x), static_cast<float>(y)});
            tile.setFillColor(sf::Color(floorColor.r + 8, floorColor.g + 8, floorColor.b + 8, 70));
            window.draw(tile);
        }
    }
}

void BuildingInterior::drawExitPortal(sf::RenderWindow& window) const {
    sf::RectangleShape exit({140.0f, 42.0f});
    exit.setPosition({410.0f, 482.0f});
    exit.setFillColor(sf::Color(38, 130, 100, 210));
    window.draw(exit);
    if (font) {
        drawLabel(window, cls::text("map.exit_campus"), {428.0f, 494.0f});
    }
}

void BuildingInterior::drawPortalMarkers(sf::RenderWindow& window) const {
    for (const auto& portal : getPortals()) {
        sf::RectangleShape marker(portal.area.size);
        marker.setPosition(portal.area.position);
        marker.setFillColor(sf::Color(86, 255, 186, 45));
        marker.setOutlineColor(sf::Color(140, 255, 210, 160));
        marker.setOutlineThickness(2.0f);
        window.draw(marker);
    }
}

std::vector<InteractionPoint> BuildingInterior::loadInteractionsFromJson(const std::string& path) {
    std::vector<InteractionPoint> result;
    std::ifstream file(path);
    if (!file.is_open()) return result;
    nlohmann::json data;
    file >> data;
    for (const auto& ip : data["interactions"]) {
        InteractionPoint point;
        const auto& a = ip["area"];
        point.area = sf::FloatRect(
            sf::Vector2f(a.value("x", 0.0f), a.value("y", 0.0f)),
            sf::Vector2f(a.value("w", 0.0f), a.value("h", 0.0f))
        );
        point.actionId = ip.value("actionId", "");
        point.label = ip.value("label", "");
        point.description = ip.value("description", "");
        result.push_back(point);
    }
    return result;
}

void BuildingInterior::resolveCollisions(Player& player) const {
    if (obstacles.empty()) return;

    static constexpr float kHalf = kPlayerHalfSize;
    const sf::Vector2f pos = player.getPosition();

    for (const auto& ob : obstacles) {
        const float playerLeft   = pos.x - kHalf;
        const float playerRight  = pos.x + kHalf;
        const float playerTop    = pos.y - kHalf;
        const float playerBottom = pos.y + kHalf;

        const float obLeft   = ob.position.x;
        const float obRight  = ob.position.x + ob.size.x;
        const float obTop    = ob.position.y;
        const float obBottom = ob.position.y + ob.size.y;

        const bool collides = playerRight > obLeft && playerLeft < obRight
                           && playerBottom > obTop && playerTop < obBottom;
        if (!collides) continue;

        const float overlapLeft  = playerRight - obLeft;
        const float overlapRight = obRight - playerLeft;
        const float overlapTop   = playerBottom - obTop;
        const float overlapBot   = obBottom - playerTop;

        const float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBot});

        float newX = pos.x;
        float newY = pos.y;

        if (minOverlap == overlapLeft)       newX = obLeft - kHalf;
        else if (minOverlap == overlapRight) newX = obRight + kHalf;
        else if (minOverlap == overlapTop)   newY = obTop - kHalf;
        else                                 newY = obBottom + kHalf;

        player.setPosition(newX, newY);
        player.stopMovement();
    }
}

void BuildingInterior::drawLabel(sf::RenderWindow& window, const std::string& text,
                                  sf::Vector2f position, unsigned int size) const {
    if (!font) return;
    sf::Text label = cls::makeText(*font, text, size);
    label.setFillColor(sf::Color(244, 238, 206));
    label.setOutlineColor(sf::Color(30, 34, 30));
    label.setOutlineThickness(1.0f);
    label.setPosition(position);
    window.draw(label);
}
