#ifndef CLS_QUEST_MIDTERMEXAMQUEST_H
#define CLS_QUEST_MIDTERMEXAMQUEST_H

#include "quest/ExamQuest.h"

/**
 * @class MidtermExamQuest
 * @brief 期中考试具体类
 *
 * 继承关系: MainQuest → ExamQuest → MidtermExamQuest（3 层继承）
 * 配置: 科目=基础学科, DC=14, 5 轮, 需 3 次通过, 复习消耗 20 体力
 * 属性: 无额外属性（全部继承自 ExamQuest）
 * 行为: 无新增行为（全部继承自 ExamQuest）
 * 派生关系: 无（具体类，不再派生）
 */
class MidtermExamQuest : public ExamQuest {
public:
    MidtermExamQuest();
};

#endif // CLS_QUEST_MIDTERMEXAMQUEST_H
