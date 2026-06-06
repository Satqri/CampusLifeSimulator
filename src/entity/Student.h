#ifndef CLS_ENTITY_STUDENT_H
#define CLS_ENTITY_STUDENT_H

#include "entity/Character.h"
#include <SFML/Graphics.hpp>
#include <memory>

/**
 * @class Student
 * @brief 瀹胯垗涓殑瀛︾敓 NPC锛岀户鎵胯嚜 Character
 *
 * 鐜╁鍦ㄥ鑸嶄腑閬囧埌璇ュ鐢熷悗鍙寜涓?L 閿繘鍏ラ瀛愭瘮鐐规暟娓告垙銆?
 * 璇?NPC 涓嶇Щ鍔紝浠呬綔涓轰氦浜掑叆鍙ｅ拰娓告垙瀵规墜銆?
 */
class Student : public Character {
public:
    Student(float x, float y);

    void move(float directionX, float directionY, float deltaTime) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

    /** @brief 鍔犺浇绾圭悊 */
    bool loadTexture(const std::string& path);

    /** @brief 鑾峰彇鍖呭洿鐩掔敤浜庣鎾炴娴?*/
    sf::FloatRect getBounds() const;

    /** @brief 鏄惁闈犺繎鐜╁锛堣窛绂?< 璺濈闃堝€硷級 */
    bool isNearPlayer(const sf::Vector2f& playerPos, float threshold = 50.0f) const;

private:
    sf::Texture texture;
    sf::Sprite sprite;
    bool textureLoaded;
};

#endif // CLS_ENTITY_STUDENT_H
