#ifndef CLS_MAP_BUILDINGINTERIOR_H
#define CLS_MAP_BUILDINGINTERIOR_H

#include "map/MapPortal.h"
#include "ui/UIComponent.h"
#include <SFML/Graphics.hpp>
#include <vector>

class Player;

/**
 * @class BuildingInterior
 * @brief 建筑地图抽象基类，继承 UIComponent
 *
 * 提供室内共享渲染辅助方法 + 交互点查询接口。
 * CampusMap（室外）也继承此类以复用 getPortals/getPlace 接口。
 */
class BuildingInterior : public UIComponent {
public:
    virtual ~BuildingInterior() = default;

    // ── UIComponent 接口 ──────────────────────────────────────
    void update(float /*deltaTime*/) override {}
    void render(sf::RenderWindow& window) override = 0;

    // ── 传送门 ─────────────────────────────────────────────────
    virtual std::vector<MapPortal> getPortals() const = 0;

    // ── 地点标识 ───────────────────────────────────────────────
    virtual CampusPlace getPlace() const = 0;

    // ── 交互点 ─────────────────────────────────────────────────
    const std::vector<InteractionPoint>& getInteractionPoints() const {
        return interactions;
    }

    /**
     * @brief 根据位置查找交互点
     * @param position 玩家位置
     * @return 匹配的交互点指针，无匹配返回 nullptr
     */
    const InteractionPoint* getInteractionAt(sf::Vector2f position) const {
        for (const auto& ip : interactions) {
            if (pointInRect(position, ip.area))
                return &ip;
        }
        return nullptr;
    }

    void setFont(const sf::Font* f) { font = f; }

    /**
     * @brief 将玩家限制在当前地图边界内
     *
     * 室外地图边界为全屏，室内地图边界为房间框架内。
     * 超出边界时停止玩家移动。
     */
    void clampPlayer(Player& player) const;

protected:
    const sf::Font* font = nullptr;
    std::vector<InteractionPoint> interactions;

    // ── 室内共享渲染辅助 ──────────────────────────────────────

    /** @brief 绘制房间边框 + 地板网格 */
    void drawRoomFrame(sf::RenderWindow& window, sf::Color floorColor) const;

    /** @brief 绘制底部绿色出口 */
    void drawExitPortal(sf::RenderWindow& window) const;

    /** @brief 绘制半透明传送门标记覆盖层 */
    void drawPortalMarkers(sf::RenderWindow& window) const;

    /** @brief 绘制文字标签 */
    void drawLabel(sf::RenderWindow& window, const std::string& text,
                   sf::Vector2f position, unsigned int size = 13) const;
};

#endif // CLS_MAP_BUILDINGINTERIOR_H
