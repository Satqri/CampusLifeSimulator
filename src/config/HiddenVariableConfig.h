#ifndef CLS_CORE_HIDDENVARIABLECONFIG_H
#define CLS_CORE_HIDDENVARIABLECONFIG_H

#include <nlohmann/json.hpp>
#include <algorithm>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

/**
 * @brief 隐藏变量类型 — 存储不影响六维属性的叙事累积状态
 *
 * 支持 int / bool / string 三种值类型，用于结局判定和条件分支。
 * 合并规则由 assets/config/hidden_variables.json 中的 merge 字段定义：
 * accumulate（数值累加）、assign（覆盖）。
 *
 * @see assets/config/hidden_variables.json 完整变量定义
 */
using HiddenMap = nlohmann::json;

/**
 * @brief 隐藏变量缩放类别
 */
enum class HiddenVarScaling { None, Benefit, Burden };

/**
 * @struct VariableDef
 * @brief 单个隐藏变量的元数据定义
 */
struct VariableDef {
    enum class Type { Int, Bool, String };
    Type type = Type::Int;
    int initialInt = 0;
    bool initialBool = false;
    std::string initialString;
    int clampMin = 0;
    int clampMax = 0;
    bool hasClamp = false;
    bool isAssignment = false;
    HiddenVarScaling scaling = HiddenVarScaling::None;
};

/**
 * @brief 通配符模式规则
 */
enum class PatternKind { PrefixSuffix, Suffix };

struct PatternRule {
    PatternKind kind;
    std::string prefix;
    std::string suffix;
    int clampMin = 0;
    int clampMax = 0;
    bool hasClamp = false;
    bool isAssignment = false;
    HiddenVarScaling scaling = HiddenVarScaling::None;
};

/**
 * @class HiddenVariableConfig
 * @brief 从 JSON 加载隐藏变量元数据，提供 clamp/merge/scaling 查询
 */
class HiddenVariableConfig {
public:
    bool loadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        nlohmann::json root;
        file >> root;

        if (root.contains("variables")) {
            for (auto it = root["variables"].begin(); it != root["variables"].end(); ++it) {
                VariableDef def;
                parseVariableDef(it.value(), def);
                mVars[it.key()] = std::move(def);
            }
        }

        if (root.contains("patterns")) {
            for (const auto& p : root["patterns"]) {
                PatternRule rule;
                std::string kindStr = p.value("pattern", "suffix");
                rule.kind = (kindStr == "prefix_suffix") ? PatternKind::PrefixSuffix : PatternKind::Suffix;
                rule.prefix = p.value("prefix", "");
                rule.suffix = p.value("suffix", "");

                if (p.contains("min") && p.contains("max")) {
                    rule.clampMin = p["min"];
                    rule.clampMax = p["max"];
                    rule.hasClamp = true;
                }
                rule.isAssignment = (p.value("merge", "accumulate") == "assign");

                std::string sc = p.value("scaling", "none");
                if (sc == "benefit")      rule.scaling = HiddenVarScaling::Benefit;
                else if (sc == "burden")  rule.scaling = HiddenVarScaling::Burden;

                mPatterns.push_back(std::move(rule));
            }
        }
        return true;
    }

    const VariableDef* find(const std::string& key) const {
        auto it = mVars.find(key);
        return (it != mVars.end()) ? &it->second : nullptr;
    }

    int clamp(const std::string& key, int value) const {
        if (auto* def = find(key)) {
            if (def->hasClamp) return std::clamp(value, def->clampMin, def->clampMax);
            return value;
        }
        for (const auto& rule : mPatterns) {
            if (rule.hasClamp && match(key, rule))
                return std::clamp(value, rule.clampMin, rule.clampMax);
        }
        return value;
    }

    bool isAssignment(const std::string& key) const {
        if (auto* def = find(key)) return def->isAssignment;
        for (const auto& rule : mPatterns) {
            if (rule.isAssignment && match(key, rule)) return true;
        }
        return false;
    }

    HiddenVarScaling scalingCategory(const std::string& key) const {
        if (auto* def = find(key)) return def->scaling;
        for (const auto& rule : mPatterns) {
            if (match(key, rule)) return rule.scaling;
        }
        return HiddenVarScaling::None;
    }

    /** @brief 用配置中的初始值初始化 HiddenMap */
    void initialize(HiddenMap& target) const {
        target = HiddenMap::object();
        for (const auto& [key, def] : mVars) {
            switch (def.type) {
                case VariableDef::Type::Int:    target[key] = def.initialInt; break;
                case VariableDef::Type::Bool:   target[key] = def.initialBool; break;
                case VariableDef::Type::String: target[key] = def.initialString; break;
            }
        }
    }

private:
    static void parseVariableDef(const nlohmann::json& v, VariableDef& def) {
        std::string typeStr = v.value("type", "int");
        if (typeStr == "bool")        def.type = VariableDef::Type::Bool;
        else if (typeStr == "string") def.type = VariableDef::Type::String;

        def.isAssignment = (v.value("merge", "accumulate") == "assign");

        std::string sc = v.value("scaling", "none");
        if (sc == "benefit")      def.scaling = HiddenVarScaling::Benefit;
        else if (sc == "burden")  def.scaling = HiddenVarScaling::Burden;

        if (v.contains("min") && v.contains("max")) {
            def.clampMin = v["min"];
            def.clampMax = v["max"];
            def.hasClamp = true;
        }

        if (v.contains("initial")) {
            const auto& initVal = v["initial"];
            if (initVal.is_number_integer()) {
                def.initialInt = initVal.get<int>();
            } else if (initVal.is_boolean()) {
                def.type = VariableDef::Type::Bool;
                def.initialBool = initVal.get<bool>();
            } else if (initVal.is_string()) {
                def.type = VariableDef::Type::String;
                def.initialString = initVal.get<std::string>();
            }
        }
    }

    static bool match(const std::string& key, const PatternRule& rule) {
        if (rule.kind == PatternKind::PrefixSuffix) {
            return key.size() >= rule.prefix.size() + rule.suffix.size()
                && key.compare(0, rule.prefix.size(), rule.prefix) == 0
                && key.compare(key.size() - rule.suffix.size(), rule.suffix.size(), rule.suffix) == 0;
        }
        // Suffix
        return key.size() >= rule.suffix.size()
            && key.compare(key.size() - rule.suffix.size(), rule.suffix.size(), rule.suffix) == 0;
    }

    std::unordered_map<std::string, VariableDef> mVars;
    std::vector<PatternRule> mPatterns;
};

#endif
