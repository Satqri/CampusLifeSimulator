#ifndef SLS_STATE_GAMESTATE_H
#define SLS_STATE_GAMESTATE_H

#include <SFML/Graphics.hpp>

class Game;

/**
 * @class GameState
 * @brief 游戏状态抽象基类
 *
 * 继承关系: 根类，被 ExplorationState/EventDialogState/CombatState/MainQuestState 继承
 * 属性: game (指向主 Game 对象的指针)
 * 行为: handleInput()/update()/render() 纯虚接口
 * 派生关系: 派生 4 种具体状态类
 */
class GameState {
public:
    explicit GameState(Game* game) : game(game) {}
    virtual ~GameState() = default;

    /**
     * @brief 处理输入事件
     * @param event SFML 事件
     *
     * 纯虚函数，由具体状态实现各自的输入处理
     */
    virtual void handleInput(const sf::Event& event) = 0;

    /**
     * @brief 更新状态逻辑
     * @param deltaTime 帧间隔时间（秒）
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief 渲染状态 UI
     * @param window SFML 渲染窗口引用
     */
    virtual void render(sf::RenderWindow& window) = 0;

protected:
    Game* game; ///< 指向主 Game 对象的指针
};

#endif // SLS_STATE_GAMESTATE_H
