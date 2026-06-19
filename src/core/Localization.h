#ifndef CLS_CORE_LOCALIZATION_H
#define CLS_CORE_LOCALIZATION_H

#include <string>
#include <vector>

namespace cls {

/**
 * @enum Language
 * @brief 界面语言枚举
 */
enum class Language {
    English,
    Chinese
};

/**
 * @brief 设置当前语言
 * @param language 目标语言
 */
void setLanguage(Language language);

/**
 * @brief 从 JSON 文件加载指定语言文本表
 * @param language 目标语言
 * @param relativePath 相对资源路径或绝对路径
 * @return true 读取成功，false 读取失败
 */
bool loadLocaleFile(Language language, const std::string& relativePath);

/**
 * @brief 加载默认语言文件 assets/locales/en.json 与 assets/locales/zh.json
 *
 * 失败时仍会保留内置文本表作为兜底。
 */
void loadDefaultLocales();

/**
 * @brief 获取当前语言
 * @return 当前语言枚举值
 */
Language currentLanguage();

/**
 * @brief 获取本地化文本
 * @param key 文本键
 * @return 当前语言对应文本；若未定义则回退为 key 本身
 */
std::string text(const std::string& key);

/**
 * @brief 获取带变量替换的本地化文本
 * @param key 文本键
 * @param replacements 占位符替换列表，格式为 {"name", "value"}
 * @return 替换后的文本
 */
std::string format(const std::string& key,
                   const std::vector<std::pair<std::string, std::string>>& replacements);

/**
 * @brief 获取语言显示名称
 * @param language 语言枚举值
 * @return 语言名称文本
 */
std::string languageName(Language language);

} // namespace cls

#endif // CLS_CORE_LOCALIZATION_H
