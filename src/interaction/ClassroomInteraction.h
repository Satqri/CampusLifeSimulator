#ifndef CLS_INTERACTION_CLASSROOMINTERACTION_H
#define CLS_INTERACTION_CLASSROOMINTERACTION_H

struct GameContext;
struct InteractionPoint;

namespace ClassroomInteraction {

/**
 * @brief 强制进入晨课 — 传送玩家到教室并弹窗
 */
void forceMorningClass(GameContext& ctx);

/**
 * @brief 检查是否跨越了上课时间，若是则触发晨课
 */
void checkClassSchedule(GameContext& ctx, int previousMinute);

/**
 * @brief 玩家对晨课弹窗的选择（参与/翘课）
 */
void resolveClassChoice(GameContext& ctx, bool attend);

/**
 * @brief 处理教室交互（黑板 + 课桌）
 */
bool handle(GameContext& ctx, const InteractionPoint& ip);

} // namespace ClassroomInteraction

#endif
