#include "ui/ModalBox.h"
#include "core/TextUtils.h"
#include "map/MapPortal.h"

ModalBox::ModalBox(sf::Font& font) : mFont(font) {}

void ModalBox::setContent(const std::string& title, const std::string& body,
                          const std::string& footer) {
    mTitle = title;
    mBody = body;
    mFooter = footer;
}

void ModalBox::update(float) {}

void ModalBox::render(sf::RenderWindow& window) {
    sf::RectangleShape shade({kRenderWidth, kRenderHeight});
    shade.setFillColor(sf::Color(0, 0, 0, 105));
    window.draw(shade);

    sf::RectangleShape box({620.0f, 178.0f});
    box.setPosition({190.0f, 174.0f});
    box.setFillColor(sf::Color(14, 24, 31, 235));
    box.setOutlineColor(sf::Color(230, 210, 148, 180));
    box.setOutlineThickness(2.0f);
    window.draw(box);

    sf::Text heading = cls::makeText(mFont, mTitle, 22);
    heading.setFillColor(sf::Color(250, 238, 200));
    heading.setPosition({218.0f, 196.0f});
    window.draw(heading);

    sf::Text message = cls::makeText(mFont, mBody, 15);
    message.setFillColor(sf::Color(218, 230, 220));
    message.setPosition({218.0f, 238.0f});
    window.draw(message);

    sf::Text hint = cls::makeText(mFont, mFooter, 12);
    hint.setFillColor(sf::Color(172, 184, 178));
    hint.setPosition({218.0f, 320.0f});
    window.draw(hint);
}
