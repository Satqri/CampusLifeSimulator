#ifndef CLS_UI_TGUICONTEXT_H
#define CLS_UI_TGUICONTEXT_H

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics.hpp>

class TguiContext {
public:
    explicit TguiContext(sf::RenderWindow& window);

    bool handleEvent(const sf::Event& event);
    void draw();

    tgui::Gui& gui() { return mGui; }
    const tgui::Gui& gui() const { return mGui; }

private:
    sf::RenderWindow& mWindow;
    sf::RenderTexture mRenderTex;
    tgui::Gui mGui;
};

#endif
