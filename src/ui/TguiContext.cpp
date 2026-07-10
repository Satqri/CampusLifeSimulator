#include "ui/TguiContext.h"

TguiContext::TguiContext(sf::RenderWindow& window)
    : mGui(window)
{
}

bool TguiContext::handleEvent(const sf::Event& event) {
    return mGui.handleEvent(event);
}

void TguiContext::draw() {
    mGui.draw();
}
