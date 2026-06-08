#include "core/WindowScaler.h"
#include "map/MapPortal.h"

namespace cls {

void applyWindowSize(sf::RenderWindow& window, sf::View& view,
                     unsigned int width, unsigned int height) {
    window.setSize({width, height});
    view = sf::View(sf::FloatRect({0.0f, 0.0f}, {kRenderWidth, kRenderHeight}));
    window.setView(view);
}

sf::Vector2f mapPixelToGameCoords(const sf::RenderWindow& window, sf::Vector2i pixel) {
    return window.mapPixelToCoords(pixel);
}

} // namespace cls
