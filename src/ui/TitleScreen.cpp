#include "ui/TitleScreen.h"
#include "utils/AssetPath.h"
#include "core/Localization.h"
#include "utils/TextUtils.h"

#include <algorithm>
#include <array>
#include <cmath>

TitleScreen::TitleScreen(sf::Font& fontRef, const std::string& backgroundPath)
    : font(fontRef)
    , buttons{{
        Button{sf::FloatRect({330.0f, 360.0f}, {300.0f, 88.0f}), "title.start", TitleAction::Start,
               sf::Color(20, 154, 115, 238), sf::Color(232, 219, 160)},
        Button{sf::FloatRect({772.0f, 92.0f}, {132.0f, 52.0f}), "title.settings", TitleAction::Settings,
               sf::Color(46, 126, 180, 238), sf::Color(214, 227, 255)},
        Button{sf::FloatRect({804.0f, 446.0f}, {100.0f, 50.0f}), "title.help", TitleAction::Help,
               sf::Color(188, 164, 105, 235), sf::Color(246, 232, 178)}
      }}
{
    const std::string resolvedPath = cls::resolveAssetPath(backgroundPath);
    if (backgroundTexture.loadFromFile(resolvedPath)) {
        backgroundSprite = std::make_unique<sf::Sprite>(backgroundTexture);
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

    sf::Text title = cls::makeText(font, cls::text("title.name"), 58);
    title.setFillColor(sf::Color(246, 255, 235));
    title.setOutlineColor(sf::Color(18, 48, 48));
    title.setOutlineThickness(3.0f);
    title.setPosition({278.0f, 188.0f});
    window.draw(title);

    sf::Text subtitle = cls::makeText(font, cls::text("title.subtitle"), 18);
    subtitle.setFillColor(sf::Color(225, 246, 215));
    subtitle.setPosition({355.0f, 270.0f});
    window.draw(subtitle);

    drawButton(window, buttons[0], selectedIndex == 0);
    drawButton(window, buttons[1], selectedIndex == 1);
    drawButton(window, buttons[2], selectedIndex == 2);

    sf::Text navHint = cls::makeText(font, cls::text("title.nav"), 15);
    navHint.setFillColor(sf::Color(235, 244, 218, 220));
    navHint.setPosition({244.0f, 468.0f});
    window.draw(navHint);

    sf::Text version = cls::makeText(font, "v0.2.0", 13);
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

TitleAction TitleScreen::handleClick(sf::Vector2f mousePosition) {
    for (std::size_t i = 0; i < buttons.size(); ++i) {
        if (contains(buttons[i].bounds, mousePosition)) {
            selectedIndex = i;
            return buttons[i].action;
        }
    }
    return TitleAction::None;
}

void TitleScreen::moveSelection(int delta) {
    if (buttons.empty()) return;
    const int total = static_cast<int>(buttons.size());
    int next = static_cast<int>(selectedIndex) + delta;
    while (next < 0) next += total;
    next %= total;
    selectedIndex = static_cast<std::size_t>(next);
}

void TitleScreen::setSelection(std::size_t index) {
    if (index < buttons.size()) {
        selectedIndex = index;
    }
}

TitleAction TitleScreen::confirmSelection() const {
    return buttons[selectedIndex].action;
}

void TitleScreen::drawButton(sf::RenderWindow& window, const Button& button, bool selected) const {
    sf::RectangleShape base(button.bounds.size);
    base.setPosition(button.bounds.position);
    base.setFillColor(button.fill);
    base.setOutlineColor(button.outline);
    base.setOutlineThickness(selected ? 4.0f : 2.0f);

    const float pulse = (std::sin(elapsedTime * (selected ? 3.0f : 1.8f)) + 1.0f) * 0.5f;
    if (selected) {
        for (int i = 4; i >= 1; --i) {
            const float expand = static_cast<float>(i) * (3.8f + pulse * 2.1f);
            sf::RectangleShape glow({button.bounds.size.x + expand * 2.0f, button.bounds.size.y + expand * 2.0f});
            glow.setPosition({button.bounds.position.x - expand, button.bounds.position.y - expand});
            glow.setFillColor(sf::Color(255, 247, 160, static_cast<std::uint8_t>(16 + i * 10 + pulse * 30)));
            window.draw(glow);
        }
    }

    window.draw(base);

    sf::RectangleShape footer({button.bounds.size.x, std::min(18.0f, button.bounds.size.y * 0.28f)});
    footer.setPosition({button.bounds.position.x, button.bounds.position.y + button.bounds.size.y - footer.getSize().y});
    footer.setFillColor(selected ? sf::Color(255, 224, 130, 235) : sf::Color(182, 152, 98, 210));
    window.draw(footer);

    const unsigned int textSize = button.bounds.size.x >= 280.0f ? 28U : 18U;
    sf::Text text = cls::makeText(font, cls::text(button.labelKey), textSize);
    text.setFillColor(selected ? sf::Color(255, 252, 225) : sf::Color(248, 241, 214));
    const auto textBounds = text.getLocalBounds();
    text.setOrigin({textBounds.position.x + textBounds.size.x / 2.0f,
                    textBounds.position.y + textBounds.size.y / 2.0f});
    text.setPosition({button.bounds.position.x + button.bounds.size.x / 2.0f,
                      button.bounds.position.y + button.bounds.size.y / 2.0f - 3.0f});
    window.draw(text);
}

bool TitleScreen::contains(const sf::FloatRect& bounds, sf::Vector2f point) const {
    return point.x >= bounds.position.x
        && point.x <= bounds.position.x + bounds.size.x
        && point.y >= bounds.position.y
        && point.y <= bounds.position.y + bounds.size.y;
}
