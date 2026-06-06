#ifndef CLS_INTERACTION_LIBRARYINTERACTION_H
#define CLS_INTERACTION_LIBRARYINTERACTION_H

struct GameContext;
struct InteractionPoint;

namespace LibraryInteraction {

/**
 * @brief 处理图书馆交互（书架浏览 + 阅览桌阅读）
 * @return 是否已处理该交互
 */
bool handle(GameContext& ctx, const InteractionPoint& ip);

} // namespace LibraryInteraction

#endif
