#ifndef CLS_CORE_WINDOWSCALER_H
#define CLS_CORE_WINDOWSCALER_H

#include <SFML/Graphics.hpp>

namespace cls {

/**
 * @brief 应用窗口尺寸并保持内部渲染坐标系不变
 * @param window 目标窗口
 * @param view 游戏视图
 * @param width 窗口宽度
 * @param height 窗口高度
 */
void applyWindowSize(sf::RenderWindow& window, sf::View& view,
                     unsigned int width, unsigned int height);

/**
 * @brief 将窗口像素坐标映射为游戏坐标
 * @param window 目标窗口
 * @param pixel 鼠标像素坐标
 * @return 对应的世界坐标
 */
sf::Vector2f mapPixelToGameCoords(const sf::RenderWindow& window, sf::Vector2i pixel);

} // namespace cls

#endif // CLS_CORE_WINDOWSCALER_H
