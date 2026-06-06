#ifndef CLS_ENTITY_COMBATHELPER_H
#define CLS_ENTITY_COMBATHELPER_H

#include "core/Types.h"
#include "entity/Player.h"

/**
 * @brief 根据情绪类型获取对应玩家属性值（用于战斗检定）
 */
inline int statForEmotion(const Player& player, EmotionType type) {
    const auto& a = player.getAttributes();
    switch (type) {
        case EmotionType::ANXIETY:    return a.academic;
        case EmotionType::ANGER:      return a.energy;
        case EmotionType::LONELINESS: return a.social;
        case EmotionType::DEPRESSION: // fallthrough
        case EmotionType::FEAR:       return a.san;
    }
    return a.san;
}

/**
 * @brief 获取情绪类型对应的战斗动作名称
 */
inline const char* actionNameForEmotion(EmotionType type) {
    switch (type) {
        case EmotionType::ANXIETY:    return "Rational Analysis";
        case EmotionType::DEPRESSION: return "Will Stand";
        case EmotionType::ANGER:      return "Vent / Release";
        case EmotionType::FEAR:       return "Will Stand";
        case EmotionType::LONELINESS: return "Confide / Seek Help";
    }
    return "Unknown";
}

#endif
