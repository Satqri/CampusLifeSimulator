# CampusLifeSimulator 操作文档

> 本文档是当前游戏操作与交互规则的统一记录。任何项目成员只要修改了按键、鼠标交互、地图交互、任务流程、战斗规则、UI 流程或任何会影响玩家操作方式的内容，都必须同步修改本文档，以便团队对游戏操作形成一致共识。

最后更新：2026-06-05

## 维护规则

- 每次新增、删除或修改玩家可执行操作时，必须更新本文档。
- 每次修改已有操作的效果、触发条件、适用页面或限制时，必须更新本文档。
- 每次新增页面、地图、建筑、任务、考试、交互点或调试按键时，必须更新本文档。
- 如果代码行为和本文档不一致，以代码为准，但提交者必须在同一次提交或后续修复提交中更新本文档。

## 标题界面

| 操作 | 效果 |
| --- | --- |
| `Enter` | 进入难度选择界面 |
| `H` | 直接进入 Help / Settings 页面，跳过难度选择 |
| 鼠标点击 `New Semester` | 进入难度选择界面 |
| 鼠标点击右下角 `?` | 进入 Help / Settings 页面 |
| 点击窗口关闭按钮 | 退出游戏 |

## 难度选择界面

| 操作 | 效果 |
| --- | --- |
| `1` | 选择 Easy 并开始游戏 |
| `2` | 选择 Normal 并开始游戏 |
| `Enter` | 选择 Normal 并开始游戏 |
| `3` | 选择 Hard 并开始游戏 |
| `Esc` | 返回标题界面 |
| 鼠标点击 Easy 卡片 | 选择 Easy 并开始游戏 |
| 鼠标点击 Normal 卡片 | 选择 Normal 并开始游戏 |
| 鼠标点击 Hard 卡片 | 选择 Hard 并开始游戏 |
| 鼠标点击左上角 `<` | 返回标题界面 |

难度效果：

| 难度 | 属性变化 |
| --- | --- |
| Easy | `SAN +20`、`Energy +15`、`Academic +5`、`Social +5` |
| Normal | 属性不变 |
| Hard | `SAN -20`、`Energy -10`、`Social -5` |

注意：难度加成只会应用一次。

## 游戏内通用页面切换

| 操作 | 页面 |
| --- | --- |
| `1` | Entity / 探索地图页 |
| `2` | Simple Quest 演示任务 |
| `3` | Midterm Exam 期中考试 |
| `4` | Final Exam 期末考试 |
| `5` | QuestManager / 主线任务链演示 |
| `0` | Help / Settings 页面 |
| `6` | Help / Settings 页面 |

## 探索地图页

| 操作 | 效果 |
| --- | --- |
| `W` 或 `↑` | 向上移动 |
| `S` 或 `↓` | 向下移动 |
| `A` 或 `←` | 向左移动 |
| `D` 或 `→` | 向右移动 |
| 斜向组合移动 | 支持斜向移动，速度会归一化 |
| `Enter` | 在传送门或家具交互点上执行交互 |
| `C` | 触发压力事件，`SAN -15`，SAN 低于 30 时可能生成情绪敌人 |
| `F` | 与 100 像素内最近的敌人战斗 |
| `V` | 恢复 SAN，`SAN +15`；SAN 恢复到安全区时会清除或减少敌人 |
| `X` | 设置测试战斗 buff，下一次战斗检定 `+2` |

`Enter` 的优先级：

1. 如果玩家站在当前地图的传送门区域内，触发场景切换。
2. 如果没有踩中传送门，但玩家站在家具交互点内，触发对应交互。部分交互会消耗游戏内时间并修改属性。

## 场景切换

| 操作 | 条件 | 效果 |
| --- | --- | --- |
| `Enter` | 转场开始约 `0.45s` 后 | 完成场景切换 |
| `Esc` | 转场开始约 `0.45s` 后 | 完成场景切换 |
| 鼠标点击 | 转场开始约 `0.45s` 后 | 完成场景切换 |

## 可进入地点

校园地图可进入以下地点：

- Dormitory
- Gym
- Library
- Classroom
- Cafeteria

每个室内地图底部绿色出口区域按 `Enter` 会返回 Campus。

## 时间流动与日程系统

当前主流程为 `14` 天。顶部 HUD 固定显示 `Day XX/14 HH:MM` 的 24 小时制计时器。每天醒来固定为 `08:00`。

时间不会随真实秒数自动流动。只有玩家执行会消耗时间的交互后，游戏才会短暂黑屏，并把计时器跳到活动结束后的时间。

固定日程：

- 第 `7` 天为期中考日，HUD 会显示 `Midterm Day`。
- 每天第一次时间推进到或越过 `08:50` 时，玩家会被强制传送到 Classroom，并随机站到一个 `Sit at Desk` 交互点上。
- 08:50 课堂事件会弹出选择：`Attend class carefully` / `Skip class`。
- 选择认真上课：时间直接跳到 `12:15`。普通上课效果为 `Academic +8`、`SAN -8`、`Energy -12`。
- 第 7 天选择认真上课会进行期中考检定：`d20 + (Academic - 50) / 10` 对抗 `12`。通过时 `SAN -10`、`Energy -16`、`Academic +12`；未通过时 `SAN -16`、`Energy -18`、`Academic +4`。
- 选择逃课：时间跳到 `10:20`，随机触发老师点名。普通日点名概率为 `45%`，第 7 天为 `80%`。
- 逃课被点名：普通日 `SAN -10`、`Energy -2`、`Academic -10`、`Social -8`；第 7 天 `SAN -18`、`Energy -4`、`Academic -18`、`Social -12`。
- 逃课未被点名：`SAN +3`、`Energy -2`、`Academic -2`。

饭堂时间：

- `12:00-14:00` 和 `17:00-19:00` 可以在 Cafeteria 获取食物。
- 当前食物系统为占位版本，`Get Food` 或 `Eat at Table` 都按同一份校园餐处理。
- 吃饭固定消耗 `20min`，当前效果为 `Gold -15`、`SAN +5`、`Energy +18`、`Social +2`。
- 不在饭点交互不会消耗时间，只提示饭堂未开放。

图书馆读书：

- `Browse Shelf A/B/C/D` 只选择当前阅读书籍，不消耗时间，也不是借书或放入背包。
- `Read at Table` 会阅读当前选中的书，每次消耗 `30min`，并让该书阅读进度 `+25%`，最高 `100%`。
- Shelf A：`Reference Methods`，技能方向 `Research`，读书效果 `SAN -3`、`Energy -6`、`Academic +4`。
- Shelf B：`Literature Notes`，技能方向 `Reflection`，读书效果 `SAN +2`、`Energy -5`、`Social +3`。
- Shelf C：`Science Primer`，技能方向 `Logic`，读书效果 `SAN -4`、`Energy -7`、`Academic +6`。
- Shelf D：`Campus History`，技能方向 `Context`，读书效果 `SAN -2`、`Energy -5`、`Academic +2`、`Social +4`。

健身房锻炼：

- `Run on Treadmill` 每次消耗 `40min`，当前效果 `SAN -4`、`Energy -14`、`Social +2`。
- `Lift Barbell` 每次消耗 `40min`，当前效果 `SAN -5`、`Energy -16`、`Social +1`。

宿舍与睡觉：

- `Rest on Bed` 在 `22:30` 后会进入睡觉流程。这里按“晚上 10:30”实现为 `22:30`。
- 睡觉会直接进入下一天 `08:00`。睡眠恢复量按上床时间到次日 `08:00` 的长度计算。
- 当前恢复系数为每睡 `1` 小时 `SAN +5`、`Energy +8`，单次睡眠最高 `SAN +45`、`Energy +70`。
- 第 `14` 天睡觉后会提示 14 天项目周期结束。
- `Study at Desk` 消耗 `45min`，当前效果 `SAN -6`、`Energy -10`、`Academic +7`。
- `Sit on Rug` 当前不消耗时间。

## 室内家具交互点

Dormitory：

- `Rest on Bed`
- `Study at Desk`
- `Sit on Rug`

Gym：

- `Run on Treadmill`，共 2 个跑步机交互点
- `Lift Barbell`，共 4 个杠铃交互点

Library：

- `Browse Shelf A`
- `Browse Shelf B`
- `Browse Shelf C`
- `Browse Shelf D`
- `Read at Table`

Classroom：

- `Look at Board`
- `Sit at Desk`，共 15 个课桌交互点

Cafeteria：

- `Get Food`
- `Eat at Table`，共 5 个餐桌交互点

## Simple Quest 演示任务

| 阶段 | 操作 | 效果 |
| --- | --- | --- |
| 说明阶段 | `Enter` | 进入选择阶段 |
| 选择阶段 | `Up` | 向上切换选项，顶部继续向上会循环到底部 |
| 选择阶段 | `Down` | 向下切换选项，底部继续向下会循环到顶部 |
| 选择阶段 | `Enter` | 确认当前选项，进入结果阶段 |
| 结果阶段 | `Enter` | 确认结果并完成任务 |

Simple Quest 演示任务选项：

| 选项 | 属性变化 |
| --- | --- |
| Explore the campus and get familiar with the surroundings | `Energy -5`、`Social +10` |
| Check out what books the library has | `Energy -3`、`Academic +8`、`Social +3` |
| Grab a good meal at the cafeteria to relax | `SAN +5`、`Energy +10`、`Social +5`、`Gold -20` |

完成奖励：`Social +5`

## 期中考试

参数：

- DC：14
- 总轮数：5
- 需要成功轮数：3
- 复习消耗：`Energy -20`
- 复习加成：检定 `+3`
- 完成奖励：`Energy -5`、`Academic +10`

操作：

| 阶段 | 操作 | 效果 |
| --- | --- | --- |
| 说明阶段 | `Enter` | 进入备考选择 |
| 备考选择 | `Up` 或 `Down` | 在 Review / Skip 之间切换 |
| 备考选择 | `Enter` | 确认是否复习，进入考试 |
| 每轮考试 | `Enter` | 掷 d20 |
| 每轮结果 | `Enter` | 进入下一轮或进入最终结果 |
| 最终结果 | `Enter` | 确认并完成考试 |

## 期末考试

参数：

- DC：16
- 总轮数：7
- 需要成功轮数：4
- 复习消耗：`Energy -30`
- 复习加成：检定 `+4`
- 完成奖励：`Energy -10`、`Academic +20`、`Social +5`

操作：

| 阶段 | 操作 | 效果 |
| --- | --- | --- |
| 说明阶段 | `Enter` | 进入备考选择 |
| 备考选择 | `Up` 或 `Down` | 在 Review / Skip 之间切换 |
| 备考选择 | `Enter` | 确认是否复习，进入考试 |
| 每轮考试 | `Enter` | 掷 d20 |
| 每轮结果 | `Enter` | 进入下一轮或进入最终结果 |
| 最终结果 | `Enter` | 确认并完成考试 |

## 战斗与 SAN 规则

压力事件：

- 在探索地图页按 `C`，玩家 `SAN -15`。
- 当 SAN 低于 30 时，有概率生成敌人。

SAN 等级：

| SAN 范围 | 等级 | 最大敌人数 | 生成概率 |
| --- | --- | --- | --- |
| `SAN >= 30` | 0 | 0 | 不生成 |
| `20 <= SAN < 30` | 1 | 1 | 40% |
| `10 <= SAN < 20` | 2 | 2 | 60% |
| `SAN < 10` | 3 | 3 | 90% |

敌人类型按生成次数循环：

- Anxiety
- Depression
- Anger
- Fear
- Loneliness

战斗：

- 按 `F` 会寻找 100 像素内最近的敌人。
- 如果没有敌人、距离不够或战斗结果弹窗仍在显示，则不会开战。
- 战斗检定为：`d20 + 属性修正 + buff` 对抗敌人 DC。
- 属性修正计算：`(对应属性 - 50) / 10`。

不同敌人对应属性：

| 敌人 | 检定属性 |
| --- | --- |
| Anxiety | Academic |
| Depression | SAN |
| Anger | Energy |
| Fear | SAN |
| Loneliness | Social |

战斗结果：

| 结果 | 效果 |
| --- | --- |
| 胜利 | `SAN +20`，下一次战斗 buff 为 `+2` |
| 失败 | `SAN -15`，下一次战斗 buff 为 `-2` |

战斗结束后，该敌人会从地图上消失。

## QuestManager 页面

无活动任务时：

| 操作 | 效果 |
| --- | --- |
| `E` | 模拟完成一个事件，事件计数 `+1` |
| `S` | 检查当前事件计数是否满足下一个任务触发条件 |
| `Enter` | 如果满足阈值，创建下一个主线任务 |
| `C` | 重置 QuestManager 任务链 |

有活动任务时：

- SimpleQuest 类型使用 `Enter`、`Up`、`Down` 操作。
- ExamQuest 类型使用 `Enter`、`Up`、`Down` 操作。

主线任务阈值：

| 阈值 | 任务 |
| --- | --- |
| 0 | Freshman Orientation |
| 3 | Course Selection Week |
| 6 | Club Recruitment |
| 10 | Midterm Exam |
| 13 | Advanced Club Activities |
| 17 | Final Exam |
| 20 | Graduation Ceremony |

QuestManager 任务链里的 SimpleQuest 选项以 `assets/config/quests.json` 为准。

## 当前没有实现的操作

- 地图上鼠标点击建筑或家具无效，必须移动到区域内按 `Enter`。
- 游戏内没有全局暂停菜单。
- 游戏内没有全局 `Esc` 返回标题界面。
- Help / Settings 页面目前只显示说明，没有可点击设置项。
