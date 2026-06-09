#ifndef CLS_CORE_MEALCONFIG_H
#define CLS_CORE_MEALCONFIG_H

#include "core/Types.h"
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

/**
 * @struct MealOption
 * @brief 餐食选项 — 名称/价格/属性奖励/描述
 */
struct MealOption {
    std::string name;
    int cost;
    Attributes reward;
    std::string description;
};

/**
 * @brief 从 JSON 文件加载餐食配置
 * @param path JSON 文件路径
 * @return 餐食选项列表
 */
inline std::vector<MealOption> loadMealConfig(const std::string& path) {
    std::vector<MealOption> meals;
    std::ifstream file(path);
    if (!file.is_open()) return meals;
    nlohmann::json data;
    file >> data;
    for (const auto& m : data["meals"]) {
        MealOption opt;
        opt.name = m.value("name", "");
        opt.cost = m.value("cost", 0);
        const auto& r = m["reward"];
        opt.reward = Attributes{
            .energy = r.value("energy", 0),
            .gold = r.value("gold", 0),
            .san = r.value("san", 0),
            .academic = r.value("academic", 0),
            .social = r.value("social", 0)
        };
        opt.description = m.value("description", "");
        meals.push_back(opt);
    }
    return meals;
}

#endif
