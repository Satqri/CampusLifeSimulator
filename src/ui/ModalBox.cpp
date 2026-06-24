#include "ui/ModalBox.h"
#include "utils/TextUtils.h"
#include "utils/AssetPath.h"
#include "map/MapPortal.h"
#include <filesystem>

ModalBox::ModalBox(sf::Font& font) : mFont(font) {
    const std::string resolved = cls::resolveAssetPath("assets/image/ui/popups/event_notification.png");
    if (std::filesystem::exists(resolved) && mPopupTexture.loadFromFile(resolved)) {
        mPopupSprite = std::make_unique<sf::Sprite>(mPopupTexture);
    }
}

void ModalBox::setContent(const std::string& title, const std::string& body,
                          const std::string& footer) {
    mTitle = title;
    mBody = body;
    mFooter = footer;
}

void ModalBox::setFullscreenTexture(const sf::Texture* texture) {
    mFullscreenTexture = texture;
    mFullscreenSprite.reset();
    if (texture) {
        mFullscreenSprite = std::make_unique<sf::Sprite>(*texture);
    }
}

void ModalBox::update(float) {}

void ModalBox::render(sf::RenderWindow& window) {
    // 全屏背景（结局画面等）
    if (mFullscreenSprite) {
        const auto size = mFullscreenSprite->getTexture().getSize();
        mFullscreenSprite->setScale({
            kRenderWidth / static_cast<float>(size.x),
            kRenderHeight / static_cast<float>(size.y)
        });
        window.draw(*mFullscreenSprite);
    }

    // 遮罩
    sf::RectangleShape shade({kRenderWidth, kRenderHeight});
    shade.setFillColor(sf::Color(0, 0, 0, mFullscreenSprite ? 60 : 105));
    window.draw(shade);

    // 弹窗面板（纹理或纯色回退）
    if (mPopupSprite) {
        const auto size = mPopupTexture.getSize();
        const float scaleX = 620.0f / static_cast<float>(size.x);
        const float scaleY = 178.0f / static_cast<float>(size.y);
        const float scale = std::min(scaleX, scaleY);
        const float scaledW = size.x * scale;
        const float scaledH = size.y * scale;
        mPopupSprite->setScale({scale, scale});
        mPopupSprite->setPosition({190.0f + (620.0f - scaledW) / 2.0f, 174.0f + (178.0f - scaledH) / 2.0f});
        window.draw(*mPopupSprite);
    } else {
        sf::RectangleShape box({620.0f, 178.0f});
        box.setPosition({190.0f, 174.0f});
        box.setFillColor(sf::Color(14, 24, 31, 235));
        box.setOutlineColor(sf::Color(230, 210, 148, 180));
        box.setOutlineThickness(2.0f);
        window.draw(box);
    }

    // 文字内容
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
