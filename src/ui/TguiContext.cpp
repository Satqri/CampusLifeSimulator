#include "ui/TguiContext.h"
#include "map/MapPortal.h"

TguiContext::TguiContext(sf::RenderWindow& window)
    : mWindow(window)
    , mRenderTex({static_cast<unsigned>(kRenderWidth), static_cast<unsigned>(kRenderHeight)})
    , mGui(window)
{
    mRenderTex.setSmooth(true);
    mGui.setTarget(mRenderTex);
}

bool TguiContext::handleEvent(const sf::Event& event) {
    // 鼠标事件: 窗口像素 → 游戏逻辑坐标 (960×540)
    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f p = mWindow.mapPixelToCoords(mp->position);
        return mGui.handleEvent(sf::Event::MouseButtonPressed{
            mp->button, {static_cast<int>(p.x), static_cast<int>(p.y)}});
    }
    if (const auto* mr = event.getIf<sf::Event::MouseButtonReleased>()) {
        sf::Vector2f p = mWindow.mapPixelToCoords(mr->position);
        return mGui.handleEvent(sf::Event::MouseButtonReleased{
            mr->button, {static_cast<int>(p.x), static_cast<int>(p.y)}});
    }
    if (const auto* mm = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f p = mWindow.mapPixelToCoords(mm->position);
        return mGui.handleEvent(sf::Event::MouseMoved{
            {static_cast<int>(p.x), static_cast<int>(p.y)}});
    }
    return mGui.handleEvent(event);
}

void TguiContext::draw() {
    mRenderTex.clear(sf::Color::Transparent);
    mGui.draw();
    mRenderTex.display();
    sf::Sprite blitSprite(mRenderTex.getTexture());
    mWindow.draw(blitSprite);
}
