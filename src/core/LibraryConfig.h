#ifndef CLS_CORE_LIBRARYCONFIG_H
#define CLS_CORE_LIBRARYCONFIG_H

#include "core/Types.h"
#include "core/Localization.h"
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

/**
 * @struct LibraryBook
 * @brief 图书馆藏书 — 名称/技能/阅读属性变化
 */
struct LibraryBook {
    std::string name;
    std::string nameKey;
    std::string skill;
    std::string skillKey;
    Attributes delta;

    std::string displayName() const {
        return nameKey.empty() ? name : cls::text(nameKey);
    }

    std::string displaySkill() const {
        return skillKey.empty() ? skill : cls::text(skillKey);
    }
};

/**
 * @brief 从 JSON 加载图书馆藏书配置
 * @param path JSON 文件路径
 */
inline std::vector<LibraryBook> loadLibraryConfig(const std::string& path) {
    std::vector<LibraryBook> books;
    std::ifstream file(path);
    if (!file.is_open()) return books;
    nlohmann::json data;
    file >> data;
    for (const auto& b : data["books"]) {
        LibraryBook book;
        book.name = b.value("name", "");
        book.nameKey = b.value("name_key", "");
        book.skill = b.value("skill", "");
        book.skillKey = b.value("skill_key", "");
        const auto& d = b["delta"];
        book.delta = Attributes{
            .energy = d.value("energy", 0),
            .gold = d.value("gold", 0),
            .san = d.value("san", 0),
            .academic = d.value("academic", 0),
            .social = d.value("social", 0)
        };
        books.push_back(book);
    }
    return books;
}

#endif
