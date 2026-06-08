#ifndef CLS_CORE_TEXTUTILS_H
#define CLS_CORE_TEXTUTILS_H

#include <SFML/Graphics.hpp>
#include <string>

namespace cls {

/**
 * @brief 将 UTF-8 std::string 转换为 SFML 字符串
 * @param text UTF-8 文本
 * @return 可安全显示中文的 SFML 字符串
 */
inline sf::String toSfString(const std::string& text) {
    return sf::String::fromUtf8(text.begin(), text.end());
}

/**
 * @brief 便捷创建支持 UTF-8 的文本对象
 * @param font 字体
 * @param text UTF-8 文本
 * @param size 字号
 * @return 构造完成的文本对象
 */
inline sf::Text makeText(const sf::Font& font, const std::string& text, unsigned int size) {
    return sf::Text(font, toSfString(text), size);
}

} // namespace cls

#endif // CLS_CORE_TEXTUTILS_H
