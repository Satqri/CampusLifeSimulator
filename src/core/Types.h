#ifndef CLS_CORE_TYPES_H
#define CLS_CORE_TYPES_H

/**
 * @struct Attributes
 * @brief 角色属性数据，值域 0-100（金钱 0-9999）
 *
 * 用于 Character 及其派生类，存储角色的核心属性值
 */
struct Attributes {
    int san;      ///< 理智 (Sanity) — 核心属性，过低触发战斗
    int energy;   ///< 体力
    int academic; ///< 学业
    int social;   ///< 社交
    int gold;     ///< 金钱

    Attributes() : san(80), energy(80), academic(60), social(60), gold(100) {}
    Attributes(int s, int e, int a, int so, int g)
        : san(s), energy(e), academic(a), social(so), gold(g) {}
};

/**
 * @enum EmotionType
 * @brief 敌人情绪类型枚举
 *
 * 不同情绪对应不同的战斗对抗方式
 */
enum class EmotionType {
    ANXIETY,     ///< 焦虑 — 对抗方式: 理性分析
    DEPRESSION,  ///< 抑郁 — 对抗方式: 意志坚守
    ANGER,       ///< 愤怒 — 对抗方式: 发泄释放
    FEAR,        ///< 恐惧 — 对抗方式: 意志坚守
    LONELINESS   ///< 孤独 — 对抗方式: 倾诉求助
};

/**
 * @enum QuestPhase
 * @brief 主线任务阶段枚举
 *
 * 用于 MainQuest 及其派生类，控制任务 UI 的阶段性流转
 */
enum class QuestPhase {
    NOT_STARTED,   ///< 任务未激活
    ANNOUNCEMENT,  ///< 显示任务公告/描述
    CHOICE,        ///< 玩家选择选项（简单任务）
    PREPARATION,   ///< 考试专属：复习选择阶段
    EXAM_ROUND,    ///< 考试专属：掷骰检定阶段
    ROUND_RESULT,  ///< 考试专属：显示单轮结果
    FINAL_RESULT,  ///< 显示任务最终结果
    COMPLETED      ///< 任务已完成
};

/**
 * @enum MainQuestType
 * @brief 主线任务类型标识
 *
 * QuestManager 工厂方法根据此枚举创建对应的 MainQuest 子类
 */
enum class MainQuestType {
    ORIENTATION,       ///< 新生报到
    COURSE_SELECTION,  ///< 选课周
    CLUB_ACTIVITY,     ///< 社团活动
    MIDTERM_EXAM,      ///< 期中考试
    FINAL_EXAM,        ///< 期末考试
    GRADUATION         ///< 毕业典礼
};

/**
 * @struct ExamRollResult
 * @brief 考试单轮 d20 检定结果
 *
 * 用于 ExamQuest，记录每轮考试检定的详细数据
 */
struct ExamRollResult {
    int d20Roll;       ///< d20 原始结果 (1-20)
    int academicBonus; ///< 学业加值 (academic - 50) / 10，范围 [-5, +5]
    int reviewBonus;   ///< 复习加值
    int total;         ///< 最终结果 = d20Roll + academicBonus + reviewBonus
    int dc;            ///< 本轮 DC
    bool success;      ///< total >= DC 即为通过
};

/**
 * @enum StateType
 * @brief 游戏顶层状态枚举
 */
enum class StateType {
    EXPLORATION,   ///< 探索状态：地图移动
    EVENT_DIALOG,  ///< 事件对话状态
    COMBAT,        ///< 战斗状态
    MAIN_QUEST,    ///< 主线任务状态
    MENU,          ///< 菜单状态
    GAME_OVER      ///< 游戏结束状态
};

#endif // CLS_CORE_TYPES_H
