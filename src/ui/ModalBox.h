#ifndef CLS_UI_MODALBOX_H
#define CLS_UI_MODALBOX_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include "ui/UIComponent.h"

/**
 * @class ModalBox
 * @brief 通用模态弹窗 — 半透明遮罩 + 居中面板 + 标题/正文/底部提示
 */
class ModalBox : public UIComponent {
public:
    explicit ModalBox(sf::Font& font);

    /**
     * @brief 设置弹窗内容，下次 render 时显示
     */
    void setContent(const std::string& title, const std::string& body,
                    const std::string& footer);

    /** @brief 设置全屏背景纹理（用于结局画面等） */
    void setFullscreenTexture(const sf::Texture* texture);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    sf::Font& mFont;
    std::string mTitle;
    std::string mBody;
    std::string mFooter;

    sf::Texture mPopupTexture;
    std::unique_ptr<sf::Sprite> mPopupSprite;
    const sf::Texture* mFullscreenTexture = nullptr;
    std::unique_ptr<sf::Sprite> mFullscreenSprite;
};

#endif
