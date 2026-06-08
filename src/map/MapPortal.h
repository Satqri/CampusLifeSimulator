#ifndef CLS_MAP_MAPPORTAL_H
#define CLS_MAP_MAPPORTAL_H

#include "ui/SceneBackground.h"
#include "core/Localization.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <algorithm>

/// 窗口分辨率
constexpr unsigned int kWindowWidth = 1280;
constexpr unsigned int kWindowHeight = 720;
/// 渲染分辨率（像素风格放大前）
constexpr float kRenderWidth = 960.0f;
constexpr float kRenderHeight = 540.0f;
/// 玩家半尺寸（用于边界 clamp）
constexpr float kPlayerHalfSize = 8.0f;

/**
 * @enum CampusPlace
 * @brief 校园地点枚举
 */
enum class CampusPlace {
    Campus,
    Dormitory,
    Gym,
    Library,
    Classroom,
    Cafeteria
};

/**
 * @struct MapPortal
 * @brief 场景传送门 — 玩家走入区域按 Enter 触发场景切换
 */
struct MapPortal {
    sf::FloatRect area;
    CampusPlace target;
    SceneBackgroundType transitionBackground;
    sf::Vector2f spawnPosition;
    std::string title;
    std::string subtitle;
};

/**
 * @struct InteractionPoint
 * @brief 建筑内的家具交互点 — 玩家靠近按 Enter 触发事件
 */
struct InteractionPoint {
    sf::FloatRect area;
    std::string actionId;
    std::string label;
    std::string description;
};

/**
 * @brief AABB 碰撞检测
 */
inline bool pointInRect(sf::Vector2f point, const sf::FloatRect& rect) {
    return point.x >= rect.position.x
        && point.x <= rect.position.x + rect.size.x
        && point.y >= rect.position.y
        && point.y <= rect.position.y + rect.size.y;
}

/**
 * @brief 获取地点显示名称
 */
inline std::string placeName(CampusPlace place) {
    switch (place) {
        case CampusPlace::Campus:    return cls::text("map.campus");
        case CampusPlace::Dormitory: return cls::text("map.dormitory");
        case CampusPlace::Gym:       return cls::text("map.gym");
        case CampusPlace::Library:   return cls::text("map.library");
        case CampusPlace::Classroom: return cls::text("map.classroom");
        case CampusPlace::Cafeteria: return cls::text("map.cafeteria");
    }
    return cls::text("map.campus");
}

#endif // CLS_MAP_MAPPORTAL_H
