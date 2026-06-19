#ifndef CLS_CORE_SETTLEMENTRESOLVER_H
#define CLS_CORE_SETTLEMENTRESOLVER_H

#include "core/CharacterState.h"
#include <string>
#include <vector>

class Player;

struct EndingDefinition {
    int priority = 0;
    std::string id;
    std::string name;
    std::string tagline;
    std::string description;
    std::string type;
    std::string trigger;
};

struct EarnedTitle {
    std::string group;
    std::string id;
    std::string name;
    std::string subtitle;
    std::string text;
};

struct SettlementResult {
    bool resolved = false;
    bool gameOver = false;
    EndingDefinition ending;
    std::vector<EarnedTitle> titles;
    std::string summary;
};

class SettlementResolver {
public:
    bool load(const std::string& endingsPath, const std::string& titlesPath);
    SettlementResult resolveImmediate(const Player& player) const;
    SettlementResult resolveFinal(const Player& player) const;

private:
    std::string buildSummary(const Player& player, const SettlementResult& result) const;

    std::vector<EndingDefinition> mEndings;
    std::vector<EarnedTitle> mTitleDefs;
};

#endif // CLS_CORE_SETTLEMENTRESOLVER_H
