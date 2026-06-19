# 校园生活模拟器 AI 编程事件规范（附录速查）

> 用途：给 Claude / AI 编程工具快速查字段、变量、现有事件文件、链条结构与结局映射。与《校园生活模拟器 AI 编程事件规范（主文档）》配套使用。

---

## A. 基础属性速查

| 字段 | 含义 | 常见变化方向 |
| --- | --- | --- |
| `energy` | 体力 / 行动力 | 睡觉、吃饭恢复；学习、赶路、兼职、熬夜消耗 |
| `health` | 身体健康 | 可作为显性健康值；若保留当前体系，也可弱化其地位 |
| `gold` | 金钱 | 打工增加；购买与赔偿减少 |
| `san` | 压力 / 心态 | 失败、尴尬、危机增加；休息、成功、被帮助降低 |
| `academic` | 学术能力 | 上课、读书、复习、大创推进增加 |
| `social` | 社交能力 | 帮助同学、沟通、展示、协调增加 |

> 说明：如果后续项目要把 `san` 全面重命名为 `stress`，必须全局统一迁移，不要混搭。

---

## B. 隐藏变量速查

### B1. 课程与点名

| 字段 | 作用 |
| --- | --- |
| `lateCount` | 迟到次数 |
| `skipClassCount` | 缺课/逃课次数 |
| `teacherTrust` | 老师信任度 |
| `classAttendCount` | 到课次数 |
| `rollCallCount` | 点名触发次数 |
| `rollCallSavedCount` | 点名救场成功次数 |
| `absencePenaltyCount` | 缺勤惩罚次数 |
| `returnClassCount` | 赶回教室次数 |

### B2. 同学互助

| 字段 | 作用 |
| --- | --- |
| `friendBond` | 同学羁绊强度 |
| `friendHelpCount` | 同学互助次数 |
| `friendRefuseCount` | 拒绝互助次数 |
| `friendRollCallHelpCount` | 同学点名救场次数 |
| `friendReviewCount` | 考前互助复习次数 |
| `sharedNotes` | 是否获得/共享复习资料 |
| `owedFavor` | 是否欠人情 |

### B3. 社团活动

| 字段 | 作用 |
| --- | --- |
| `clubType` | 社团类型 |
| `clubActivityCount` | 参与次数 |
| `clubContribution` | 贡献度 |
| `clubRelation` | 社团关系值 |
| `clubShowcaseScore` | 展示表现 |
| `clubShowcaseSuccess` | 展示是否成功 |

### B4. 大创比赛

| 字段 | 作用 |
| --- | --- |
| `innovationJoined` | 是否参加大创 |
| `innovationLeader` | 是否担任负责人 |
| `innovationIntel` | 是否提前打听情报 |
| `innovationTopic` | 选题方向 |
| `innovationProgress` | 项目进度 |
| `innovationTeamTrust` | 团队信任 |
| `innovationDemoMode` | Demo 展示方式 |
| `innovationSpeaker` | 主讲人 |
| `innovationDefenseScore` | 答辩分 |
| `innovationCrisisCount` | 危机次数 |
| `innovationResult` | 大创路线结算结果 |

### B5. 日常积累

| 字段 | 作用 |
| --- | --- |
| `healthIndex` | 长期健康底线 |
| `lateNightLevel` | 熬夜程度 |
| `partTimeCount` | 兼职次数 |
| `socialAwkwardCount` | 社死/尴尬累计 |
| `storeTrust` | 便利店排班信任 |
| `storeNightShiftCount` | 夜班次数 |
| `libraryVisitCount` | 图书馆交互次数 |
| `exerciseCount` | 锻炼次数 |
| `mealCount` | 正常吃饭次数 |
| `gameAddiction` | 游戏沉迷值 |
| `researchUnlocked` | Research 解锁 |
| `expressionUnlocked` | Expression 解锁 |
| `logicUnlocked` | Logic 解锁 |
| `campusIntelUnlocked` | Campus Intel 解锁 |

---

## C. 推荐 action_id / trigger 速查

### C1. interaction action_id

| action_id | 含义 |
| --- | --- |
| `campus_square` | 校园广场 / 路演 / 公共活动 |
| `campus_bulletin` | 公告栏 / 通知入口 |
| `cafeteria_counter` | 食堂打饭窗口 |
| `cafeteria_table` | 食堂餐桌 |
| `library_desk` | 图书馆阅读桌 / 自习点 |
| `gym_bench` | 健身房器械区 |
| `dormitory_room` | 宿舍房间通用交互 |
| `dormitory_phone` | 宿舍手机相关交互 |
| `store_counter` | 便利店柜台 / 购物入口 |
| `store_night_shift` | 便利店夜班兼职入口 |
| `store_shelf` | 便利店货架 / 夜宵入口 |
| `store_register` | 便利店收银台 / 顾客处理 |

### C2. time_schedule method

| method | 含义 |
| --- | --- |
| `crossed_class_time` | 时间跨过上课节点 |

---

## D. 常规交互推荐表

| 场景 | 常规交互 | 是否建议做成 choice |
| --- | --- | --- |
| 食堂 | 打饭 / 吃饭 | 否 |
| 宿舍 | 睡觉 / 短休 / 学习 / 打游戏 / 刷手机 | 否 |
| 图书馆 | 读书 / 查资料 / 阅读桌 / 兼职 | 否 |
| 教室 | 听课 / 看黑板 / 复习笔记 | 否 |
| 健身房 | 跑步 / 力量训练 / 前台兼职 | 否 |
| 便利店 | 买吃的 / 买饮料 / 泡面 / 收银兼职 | 否 |
| 大创 | 写方案 / 做 PPT / 开会 | 否 |

---

## E. 核心突发事件建议表

| 事件 ID（建议） | 场景 | 核心考验 |
| --- | --- | --- |
| `random_roll_call` | 教室/全局上课时段 | Energy / Social / teacherTrust |
| `random_blackboard_problem` | 教室 | Academic / san / Social |
| `random_last_chicken_leg` | 食堂 | Social / Gold / Energy |
| `random_library_seat` | 图书馆 | Social / Academic / Energy |
| `random_dorm_network_down` | 宿舍 | Academic / Social / san |
| `random_wrong_post` | 宿舍 / 手机 | Social / san |
| `random_gym_equipment_taken` | 健身房 | Social / Academic |
| `random_campus_stage_invite` | 校园广场 | Social / san / Energy |
| `random_store_teacher_visit` | 便利店 | Social / teacherTrust |
| `random_store_overtime` | 便利店夜班 | Energy / Social / Gold |
| `random_store_food_sold_out` | 便利店深夜购物 | Gold / Energy / san |
| `random_store_customer_complaint` | 便利店兼职 | Academic / Social / storeTrust |

---

## F. 长任务链阶段索引

### F1. 课程与点名链

| 阶段 | 作用 |
| --- | --- |
| 上课时间到 | 读地点与时间 |
| 是否在教室 | 决定安全/逃课分支 |
| 是否点名 | 决定是否进入风险 |
| 是否找同学救场 | 接入同学互助链 |
| 是否赶回补救 | 影响老师信任与迟到类称号 |
| 学期结算 | 输出副结局与称号 |

### F2. 同学互助链

| 阶段 | 作用 |
| --- | --- |
| 同学卡题求助 | 建立关系起点 |
| 食堂回礼 | 建立回流与人情 |
| 点名救场 | 把关系转成实际帮助 |
| 考前互助复习 | 把关系转成学业收益 |
| 关系结果 | 输出副结局与称号 |

### F3. 社团活动链

| 阶段 | 作用 |
| --- | --- |
| 社团招新 | 选择路线 |
| 第一次任务 | 建立存在感 |
| 社团分歧 | 决定你是推进者还是协调者 |
| 社团展示 | 结果爆发点 |
| 学期结算 | 输出副结局与称号 |

### F4. 大创比赛链

| 阶段 | 作用 |
| --- | --- |
| 比赛通知 | 决定是否入场 |
| 组队与选题 | 决定路线基础质量 |
| 中期危机 | 决定进度与团队稳定 |
| 答辩前夜 | 决定展示策略 |
| 现场答辩 | 决定高低上限 |
| 结果结算 | 写入 `innovationResult` 并影响主结局/称号 |

---

## G. 主结局索引

| 方向 | 关键词 |
| --- | --- |
| 因病休学 | `healthIndex` 崩溃 |
| 崩溃退学 | `san/stress` 或 `energy` 崩溃 |
| 留级危机 | 学业差 + 缺课多 |
| 创业新星 | 大创路线优秀 |
| 学霸 | 高学术 + 高老师信任 |
| 社交达人 | 高社交 |
| 打工皇帝 | 高金钱 + 高兼职次数 |
| 健身达人 | 高健康积累 + 高锻炼 |
| 人脉王 | 高人际积累 |
| 电竞高手 | 高游戏沉迷 + 压力可控 |
| 肝帝 | 高学术 + 高熬夜 |
| 摆烂快乐人 | 低压力 + 中低学业 |
| 普通毕业 | 默认兜底 |

---

## H. 称号组索引

| 组别 | 代表称号 |
| --- | --- |
| 课程点名组 | 课堂稳定器 / 点名边缘人 / 迟到补锅人 / 缺勤边缘户 |
| 社交互助组 | 靠谱同桌 / 点名互保人 / 资料共享者 / 点头之交 / 已读不回的人 |
| 社团活动组 | 社团主心骨 / 气氛修补匠 / 展示担当 / 稳定成员 / 社团幽灵成员 / 自由时间守护者 |
| 大创专属组 | 校园创新之星 / 通宵救火队长 / 稳定结项人 / PPT 保命大师 / 项目群沉默者 / 大创旁观者 |
| 日常积累组 | 早八冲刺王 / 图书馆钉子户 / 食堂常驻居民 / 夜班战神 |

---

## I. 仓库现有事件文件映射

| 文件 | 当前职责 |
| --- | --- |
| `assets/config/events/cafeteria.json` | 食堂突发事件 |
| `assets/config/events/campus.json` | 校园公共事件 |
| `assets/config/events/class_attendance.json` | 上课点名链入口 |
| `assets/config/events/club_chain.json` | 社团活动链 |
| `assets/config/events/dormitory.json` | 宿舍突发事件 |
| `assets/config/events/endings.json` | 主结局定义 |
| `assets/config/events/friend_help_chain.json` | 同学互助链 |
| `assets/config/events/gym.json` | 健身房突发事件 |
| `assets/config/events/innovation_chain.json` | 大创比赛链 |
| `assets/config/events/library.json` | 图书馆突发事件 |
| `assets/config/events/store.json` | 便利店事件 |
| `assets/config/events/titles.json` | 称号定义 |

---

## J. AI 自检清单

每次让 AI 新增或改写事件时，自检：

1. 这是常规交互、突发事件、还是长任务链节点？
2. 有没有复用现有变量名与 action_id？
3. 是否同时给出 `delta` 与 `hidden_delta`？
4. 这个事件会不会影响某条链的后续节点？
5. 这个事件会不会影响主结局或称号？
6. 是否需要同步改 `endings.json` / `titles.json` / 文档附录？
7. 如果这是结构性重构，是否提供了迁移后的统一命名规则？
