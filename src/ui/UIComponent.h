#ifndef CLS_UI_UICOMPONENT_H
#define CLS_UI_UICOMPONENT_H

#include <SFML/Graphics.hpp>

/**
 * @class UIComponent
 * @brief Base class for reusable SFML UI components.
 */
class UIComponent {
public:
    virtual ~UIComponent() = default;

    /**
     * @brief Update component state before drawing.
     * @param deltaTime Frame interval in seconds.
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Draw the component.
     * @param window Target render window.
     */
    virtual void render(sf::RenderWindow& window) = 0;
};

#endif // CLS_UI_UICOMPONENT_H
