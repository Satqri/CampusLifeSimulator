#ifndef CLS_QUEST_QUESTMANAGER_H
#define CLS_QUEST_QUESTMANAGER_H

#include <string>
#include <vector>
#include <memory>
#include "core/Types.h"

class MainQuest;

/**
 * @class QuestManager
 * @brief 主线任务链管理器
 *
 * 属性: questChain (任务定义列表), completedEventCount, currentQuestIndex, currentQuest
 * 行为: 加载 quests.json、事件计数、阈值检测、工厂创建任务、进度追踪
 * 派生关系: 无（独立工具类，组合于 Game）
 */
class QuestManager {
public:
    QuestManager();

    /**
     * @brief 从 JSON 文件加载任务链配置
     * @param filepath quests.json 文件路径
     * @return true 加载成功
     *
     * 算法步骤:
     * 1. 打开并解析 JSON 文件
     * 2. 遍历 quests 数组，构建 QuestDef 列表
     * 3. 存储到 questChain 成员
     */
    bool loadQuestChain(const std::string& filepath);

    /**
     * @brief 随机事件完成时调用，递增事件计数器
     */
    void onEventCompleted();

    /**
     * @brief 检测是否应触发下一个主线任务
     * @return true 当计数达到阈值且无活跃任务
     */
    bool shouldTriggerQuest() const;

    /**
     * @brief 工厂方法：创建下一个 MainQuest 子类实例
     * @return 新创建的任务对象，无任务时返回 nullptr
     */
    std::unique_ptr<MainQuest> createNextQuest();

    /**
     * @brief 当前任务完成时调用，推进索引
     */
    void onQuestCompleted();

    // --- Getters ---
    int getCompletedEventCount() const { return completedEventCount; }
    int getNextThreshold() const;
    MainQuest* getCurrentQuest() const { return currentQuest.get(); }
    bool isAllQuestsCompleted() const { return allCompleted; }
    int getTotalQuests() const { return static_cast<int>(questChain.size()); }
    int getCurrentQuestIndex() const { return currentQuestIndex; }

    /**
     * @brief 获取学期进度 (0.0 ~ 1.0)
     */
    float getSemesterProgress() const;

private:
    /**
     * @struct QuestDef
     * @brief 任务链中的单条任务定义（从 JSON 反序列化）
     */
    struct QuestDef {
        MainQuestType type;
        int eventThreshold;
        std::string customId;
        std::string displayName;
        std::string description;
        std::vector<std::pair<std::string, Attributes>> choices;
        Attributes reward;
        std::string subject;
        int dc;
        int totalRounds;
        int requiredPasses;
        int reviewEnergyCost;
        int reviewBonus;
    };

    std::vector<QuestDef> questChain;       ///< 任务链定义列表
    int completedEventCount;                 ///< 已完成随机事件数
    int currentQuestIndex;                   ///< 当前任务在链中的索引
    std::unique_ptr<MainQuest> currentQuest; ///< 当前活跃任务
    bool allCompleted;                       ///< 全部任务是否已完成
    bool questActive;                        ///< 是否有任务正在处理中
};

#endif // CLS_QUEST_QUESTMANAGER_H
