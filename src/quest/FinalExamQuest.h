#ifndef CLS_QUEST_FINALEXAMQUEST_H
#define CLS_QUEST_FINALEXAMQUEST_H

#include "quest/ExamQuest.h"

/**
 * @class FinalExamQuest
 * @brief 期末考试具体类
 *
 * 继承关系: MainQuest → ExamQuest → FinalExamQuest（3 层继承）
 * 配置: 科目=专业综合, DC=16, 7 轮, 需 4 次通过, 复习消耗 30 体力
 * 属性: 无额外属性（全部继承自 ExamQuest）
 * 行为: 无新增行为（全部继承自 ExamQuest）
 * 派生关系: 无（具体类，不再派生）
 */
class FinalExamQuest : public ExamQuest {
public:
    FinalExamQuest();
};

#endif // CLS_QUEST_FINALEXAMQUEST_H
