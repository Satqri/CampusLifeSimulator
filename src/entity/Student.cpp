#include "entity/Student.h"
#include "core/AssetPath.h"
#include <iostream>

Student::Student(float x, float y)
    : Character(x, y, Attributes(50, 50, 50, 50, 0), 0.0f)
    , texture()
    , sprite(texture)
    , textureLoaded(false)
{
    setName("Student");
    sprite.setPosition({x, y});
}

bool Student::loadTexture(const std::string& path) {
    std::string resolved = cls::resolveAssetPath(path);

    if (!texture.loadFromFile(resolved)) {
        std::cerr << "[Student] Failed to load texture: " << path
                  << " (resolved: " << resolved << ")" << std::endl;
        return false;
    }

    texture.setSmooth(true);
    sprite.setTexture(texture, true);

    // 璋冩暣鍒板悎閫傚ぇ灏?
    float targetWidth = 36.0f;
    float targetHeight = 36.0f;
    auto texSize = texture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float scaleX = targetWidth / static_cast<float>(texSize.x);
        float scaleY = targetHeight / static_cast<float>(texSize.y);
        float scale = std::min(scaleX, scaleY);
        sprite.setScale({scale, scale});
        // 灞呬腑閿氱偣
        sprite.setOrigin({static_cast<float>(texSize.x) / 2.0f,
                          static_cast<float>(texSize.y) / 2.0f});
        sprite.setPosition({posX, posY});
    }

    textureLoaded = true;
    std::cout << "[Student] Texture loaded successfully: " << resolved
              << " (" << texSize.x << "x" << texSize.y << ")" << std::endl;
    return true;
}

void Student::move(float /*directionX*/, float /*directionY*/, float /*deltaTime*/) {
    // 瀛︾敓 NPC 涓嶇Щ鍔?
}

void Student::update(float /*deltaTime*/) {
    // 瀛︾敓 NPC 鏃犺嚜涓昏涓?
}

void Student::render(sf::RenderWindow& window) {
    if (!visible) return;
    if (textureLoaded) {
        window.draw(sprite);
    } else {
        // 绾圭悊鏈姞杞芥椂鐨勫洖閫€锛堜笉搴斿嚭鐜帮紝闄ら潪璺緞閿欒锛?
        sf::RectangleShape fallback({32.0f, 32.0f});
        fallback.setPosition({posX - 16.0f, posY - 16.0f});
        fallback.setFillColor(sf::Color(100, 180, 220));
        window.draw(fallback);
    }
}

sf::FloatRect Student::getBounds() const {
    if (textureLoaded) {
        return sprite.getGlobalBounds();
    }
    return sf::FloatRect({posX - 16.0f, posY - 16.0f}, {32.0f, 32.0f});
}

bool Student::isNearPlayer(const sf::Vector2f& playerPos, float threshold) const {
    float dx = playerPos.x - posX;
    float dy = playerPos.y - posY;
    float dist = std::sqrt(dx * dx + dy * dy);
    return dist <= threshold;
}
