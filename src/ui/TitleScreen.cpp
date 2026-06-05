#include "ui/TitleScreen.h"

#include <algorithm>
#include <array>
#include <cmath>

TitleScreen::TitleScreen(sf::Font& fontRef, const std::string& backgroundPath)
    : font(fontRef),
      startButton({360.0f, 378.0f}, {240.0f, 72.0f}),
      helpButton({860.0f, 440.0f}, {68.0f, 68.0f}) {
    const std::array<std::string, 4> candidates = {
        backgroundPath,
        "../../../" + backgroundPath,
        "../../../../" + backgroundPath,
        "D:/Campus_2D/CampusLifeSimulator/" + backgroundPath
    };
    for (const auto& path : candidates) {
        if (backgroundTexture.loadFromFile(path)) {
            backgroundSprite = std::make_unique<sf::Sprite>(backgroundTexture);
            break;
        }
    }
}

void TitleScreen::update(float deltaTime) {
    elapsedTime += deltaTime;
}

void TitleScreen::render(sf::RenderWindow& window) {
    if (backgroundSprite) {
        const auto size = backgroundTexture.getSize();
        if (size.x > 0 && size.y > 0) {
            backgroundSprite->setScale({
                960.0f / static_cast<float>(size.x),
                540.0f / static_cast<float>(size.y)
            });
        }
        window.draw(*backgroundSprite);
    } else {
        sf::RectangleShape fallback({960.0f, 540.0f});
        fallback.setFillColor(sf::Color(12, 92, 76));
        window.draw(fallback);
    }

    sf::RectangleShape shade({960.0f, 540.0f});
    shade.setFillColor(sf::Color(0, 20, 18, 70));
    window.draw(shade);
    drawAmbientEffects(window);

    sf::Text title(font, "Campus Life", 58);
    title.setFillColor(sf::Color(246, 255, 235));
    title.setOutlineColor(sf::Color(18, 48, 48));
    title.setOutlineThickness(3.0f);
    title.setPosition({278.0f, 188.0f});
    window.draw(title);

    sf::Text subtitle(font, "Semester begins here", 18);
    subtitle.setFillColor(sf::Color(225, 246, 215));
    subtitle.setPosition({385.0f, 270.0f});
    window.draw(subtitle);

    drawButton(window, startButton, "New Semester", sf::Color(20, 154, 115, 238),
               sf::Color(232, 219, 160));
    drawButton(window, helpButton, "?", sf::Color(188, 164, 105, 235),
               sf::Color(246, 232, 178));

    sf::Text version(font, "v0.2.0", 13);
    version.setFillColor(sf::Color(230, 244, 218, 210));
    version.setPosition({28.0f, 500.0f});
    window.draw(version);
}

void TitleScreen::drawAmbientEffects(sf::RenderWindow& window) const {
    const std::array<sf::Vector2f, 11> seeds = {{
        {58.0f, 294.0f}, {162.0f, 122.0f}, {312.0f, 90.0f},
        {486.0f, 178.0f}, {604.0f, 76.0f}, {708.0f, 150.0f},
        {826.0f, 296.0f}, {886.0f, 96.0f}, {244.0f, 410.0f},
        {520.0f, 360.0f}, {752.0f, 414.0f}
    }};

    for (std::size_t i = 0; i < seeds.size(); ++i) {
        const float phase = elapsedTime * (0.55f + static_cast<float>(i % 4) * 0.08f)
            + static_cast<float>(i) * 1.37f;
        const float driftX = std::sin(phase) * (4.0f + static_cast<float>(i % 3) * 2.0f);
        const float driftY = std::cos(phase * 0.8f) * (3.0f + static_cast<float>(i % 2) * 2.0f);
        const float pulse = (std::sin(phase * 1.35f) + 1.0f) * 0.5f;
        const float radius = 3.0f + static_cast<float>(i % 4) * 1.5f + pulse * 2.0f;

        sf::CircleShape glow(radius * 3.0f);
        glow.setOrigin({radius * 3.0f, radius * 3.0f});
        glow.setPosition({seeds[i].x + driftX, seeds[i].y + driftY});
        glow.setFillColor(sf::Color(88, 255, 195, static_cast<std::uint8_t>(18 + pulse * 18)));
        window.draw(glow);

        sf::CircleShape core(radius);
        core.setOrigin({radius, radius});
        core.setPosition({seeds[i].x + driftX, seeds[i].y + driftY});
        core.setFillColor(sf::Color(157, 255, 210, static_cast<std::uint8_t>(80 + pulse * 70)));
        window.draw(core);
    }

    const float breathe = (std::sin(elapsedTime * 0.9f) + 1.0f) * 0.5f;
    sf::RectangleShape vignette({960.0f, 540.0f});
    vignette.setFillColor(sf::Color(0, 18, 18, static_cast<std::uint8_t>(18 + breathe * 16)));
    window.draw(vignette);
}

TitleAction TitleScreen::handleClick(sf::Vector2f mousePosition) const {
    if (contains(startButton, mousePosition)) {
        return TitleAction::Start;
    }
    if (contains(helpButton, mousePosition)) {
        return TitleAction::Help;
    }
    return TitleAction::None;
}

void TitleScreen::drawButton(sf::RenderWindow& window, const sf::FloatRect& bounds,
                             const std::string& label, const sf::Color& fill,
                             const sf::Color& outline) const {
    sf::RectangleShape button(bounds.size);
    button.setPosition(bounds.position);
    button.setFillColor(fill);
    button.setOutlineColor(outline);
    button.setOutlineThickness(2.0f);

    if (bounds.size.x > 100.0f) {
        const float pulse = (std::sin(elapsedTime * 2.4f) + 1.0f) * 0.5f;
        for (int i = 3; i >= 1; --i) {
            const float expand = static_cast<float>(i) * (4.0f + pulse * 1.8f);
            sf::RectangleShape glow({bounds.size.x + expand * 2.0f, bounds.size.y + expand * 2.0f});
            glow.setPosition({bounds.position.x - expand, bounds.position.y - expand});
            glow.setFillColor(sf::Color(102, 255, 204, static_cast<std::uint8_t>(18 + i * 8 + pulse * 14)));
            window.draw(glow);
        }
    }

    window.draw(button);

    sf::RectangleShape footer({bounds.size.x, std::min(18.0f, bounds.size.y * 0.28f)});
    footer.setPosition({bounds.position.x, bounds.position.y + bounds.size.y - footer.getSize().y});
    footer.setFillColor(sf::Color(182, 152, 98, 210));
    window.draw(footer);

    const unsigned int size = bounds.size.x > 100.0f ? 21U : 30U;
    sf::Text text(font, label, size);
    text.setFillColor(sf::Color(248, 241, 214));
    const auto textBounds = text.getLocalBounds();
    text.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                    textBounds.position.y + textBounds.size.y / 2.0f});
    text.setPosition({bounds.position.x + bounds.size.x / 2.0f,
                      bounds.position.y + bounds.size.y / 2.0f - 3.0f});
    window.draw(text);
}

bool TitleScreen::contains(const sf::FloatRect& bounds, sf::Vector2f point) const {
    return point.x >= bounds.position.x
        && point.x <= bounds.position.x + bounds.size.x
        && point.y >= bounds.position.y
        && point.y <= bounds.position.y + bounds.size.y;
}
