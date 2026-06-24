#ifndef CLS_MAP_BUILDINGINTERIOR_H
#define CLS_MAP_BUILDINGINTERIOR_H

#include "map/MapPortal.h"
#include "ui/UIComponent.h"
#include <SFML/Graphics.hpp>
#include <string>
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
     * @brief 根据位置查找交互点（检测范围向外扩展 10px，允许从家具边缘交互）
     */
    const InteractionPoint* getInteractionAt(sf::Vector2f position) const {
        static constexpr float kMargin = 16.0f;
        for (const auto& ip : interactions) {
            const sf::FloatRect expanded(
                ip.area.position - sf::Vector2f(kMargin, kMargin),
                ip.area.size + sf::Vector2f(kMargin * 2, kMargin * 2));
            if (pointInRect(position, expanded))
                return &ip;
        }
        return nullptr;
    }

    /** @brief 把交互点区域复制为障碍物 */
    void initObstaclesFromInteractions() {
        obstacles.clear();
        for (const auto& ip : interactions) {
            obstacles.push_back(ip.area);
        }
    }

    /** @brief 按 actionId 更新交互点区域（用于精灵尺寸适配） */
    void updateInteractionArea(const std::string& actionId, sf::FloatRect newArea) {
        for (auto& ip : interactions) {
            if (ip.actionId == actionId) {
                ip.area = newArea;
                return;
            }
        }
    }

    void setFont(const sf::Font* f) { font = f; }

    /**
     * @brief 将玩家限制在当前地图边界内
     *
     * 室外地图边界为全屏，室内地图边界为房间框架内。
     * 超出边界时停止玩家移动。
     */
    void clampPlayer(Player& player) const;

    /**
     * @brief 从 JSON 文件加载交互点列表
     * @param path JSON 文件路径
     * @return 交互点列表
     */
    static std::vector<InteractionPoint> loadInteractionsFromJson(const std::string& path);

    // ── 碰撞 ──────────────────────────────────────────────────

    /**
     * @brief 将玩家推离所有障碍物
     * @param player 玩家对象
     */
    void resolveCollisions(Player& player) const;

    /**
     * @brief 获取当前地图的障碍物列表（子类可追加）
     */
    std::vector<sf::FloatRect>& getObstacles() { return obstacles; }

protected:
    const sf::Font* font = nullptr;
    std::vector<InteractionPoint> interactions;
    std::vector<sf::FloatRect> obstacles;

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
