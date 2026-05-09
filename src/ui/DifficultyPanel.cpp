#include "ui/DifficultyPanel.h"

DifficultyPanel::DifficultyPanel(sf::Font& fontRef)
    : font(fontRef),
      backButton({34.0f, 34.0f}, {54.0f, 54.0f}),
      cards{{
          Card{sf::FloatRect({118.0f, 144.0f}, {170.0f, 218.0f}), "Easy",
              "More SAN and energy.\nExplore systems\nwith less pressure.", Difficulty::Easy,
              sf::Color(34, 170, 128)},
          Card{sf::FloatRect({395.0f, 144.0f}, {170.0f, 218.0f}), "Normal",
              "Balanced campus life.\nRecommended\nfirst run.", Difficulty::Normal,
              sf::Color(24, 154, 115)},
          Card{sf::FloatRect({672.0f, 144.0f}, {170.0f, 218.0f}), "Hard",
              "Lower SAN buffer.\nStress appears\nearlier.", Difficulty::Hard,
              sf::Color(206, 120, 80)}
      }} {
}

void DifficultyPanel::update(float) {
}

void DifficultyPanel::render(sf::RenderWindow& window) {
    sf::RectangleShape background({960.0f, 540.0f});
    background.setFillColor(sf::Color(58, 56, 43));
    window.draw(background);

    for (int i = 0; i < 120; ++i) {
        sf::CircleShape speck(1.0f + static_cast<float>(i % 3) * 0.45f);
        speck.setPosition({
            static_cast<float>((i * 67) % 940),
            330.0f + static_cast<float>((i * 31) % 180)
        });
        speck.setFillColor(sf::Color(25, 31, 23, 130));
        window.draw(speck);
    }

    sf::RectangleShape back(backButton.size);
    back.setPosition(backButton.position);
    back.setFillColor(sf::Color(21, 158, 119));
    window.draw(back);

    sf::Text backArrow(font, "<", 28);
    backArrow.setFillColor(sf::Color(244, 239, 208));
    backArrow.setPosition({52.0f, 42.0f});
    window.draw(backArrow);

    sf::Text title(font, "Choose Difficulty", 28);
    title.setFillColor(sf::Color(245, 235, 205));
    title.setPosition({354.0f, 42.0f});
    window.draw(title);

    sf::Text subtitle(font, "(you can tune it later)", 18);
    subtitle.setFillColor(sf::Color(224, 213, 188));
    subtitle.setPosition({374.0f, 78.0f});
    window.draw(subtitle);

    sf::RectangleShape leftRule({270.0f, 2.0f});
    leftRule.setPosition({118.0f, 116.0f});
    leftRule.setFillColor(sf::Color(160, 151, 124));
    window.draw(leftRule);

    sf::RectangleShape rightRule({270.0f, 2.0f});
    rightRule.setPosition({572.0f, 116.0f});
    rightRule.setFillColor(sf::Color(160, 151, 124));
    window.draw(rightRule);

    for (const auto& card : cards) {
        drawCard(window, card);
    }

    sf::Text note(font, "Keyboard: 1 Easy   2 Normal   3 Hard   Esc Back", 16);
    note.setFillColor(sf::Color(225, 217, 198));
    note.setPosition({300.0f, 470.0f});
    window.draw(note);
}

DifficultyAction DifficultyPanel::handleClick(sf::Vector2f mousePosition) const {
    if (contains(backButton, mousePosition)) {
        return {DifficultyActionType::Back, Difficulty::Normal};
    }
    for (const auto& card : cards) {
        if (contains(card.bounds, mousePosition)) {
            return {DifficultyActionType::Select, card.difficulty};
        }
    }
    return {};
}

void DifficultyPanel::drawCard(sf::RenderWindow& window, const Card& card) const {
    sf::RectangleShape art({card.bounds.size.x, 104.0f});
    art.setPosition(card.bounds.position);
    art.setFillColor(card.accent);
    window.draw(art);

    sf::RectangleShape nameStrip({card.bounds.size.x, 32.0f});
    nameStrip.setPosition({card.bounds.position.x, card.bounds.position.y + 104.0f});
    nameStrip.setFillColor(sf::Color(188, 164, 105));
    window.draw(nameStrip);

    sf::RectangleShape outline(card.bounds.size);
    outline.setPosition(card.bounds.position);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color(224, 209, 160));
    outline.setOutlineThickness(2.0f);
    window.draw(outline);

    sf::CircleShape head(22.0f);
    head.setPosition({card.bounds.position.x + 64.0f, card.bounds.position.y + 22.0f});
    head.setFillColor(sf::Color(245, 232, 188));
    window.draw(head);

    sf::RectangleShape body({42.0f, 34.0f});
    body.setPosition({card.bounds.position.x + 65.0f, card.bounds.position.y + 62.0f});
    body.setFillColor(sf::Color(89, 72, 48));
    window.draw(body);

    sf::Text title(font, card.title, 18);
    title.setFillColor(sf::Color(245, 237, 208));
    const auto titleBounds = title.getLocalBounds();
    title.setOrigin({titleBounds.position.x + titleBounds.size.x / 2.0f,
                     titleBounds.position.y + titleBounds.size.y / 2.0f});
    title.setPosition({card.bounds.position.x + card.bounds.size.x / 2.0f,
                       card.bounds.position.y + 120.0f});
    window.draw(title);

    sf::Text desc(font, card.description, 13);
    desc.setFillColor(sf::Color(241, 233, 210));
    desc.setPosition({card.bounds.position.x + 14.0f, card.bounds.position.y + 150.0f});
    window.draw(desc);
}

bool DifficultyPanel::contains(const sf::FloatRect& bounds, sf::Vector2f point) const {
    return point.x >= bounds.position.x
        && point.x <= bounds.position.x + bounds.size.x
        && point.y >= bounds.position.y
        && point.y <= bounds.position.y + bounds.size.y;
}
