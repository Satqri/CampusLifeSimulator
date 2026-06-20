# 事件系统 JSON 格式文档

## 概述

事件系统用 JSON 描述多步分支事件链。每个事件是一个**节点图**：节点按 ID 互相链接，不同类型节点控制不同的行为（展示文字、玩家选择、概率判定、条件检测、属性修改）。

## 顶层结构

```json
{
  "events": [
    { "id": "...", "trigger": {...}, "root": "入口节点ID", "steps": {...} }
  ]
}
```

| 字段 | 说明 |
|------|------|
| `id` | 事件唯一标识 |
| `trigger` | 触发条件，见下方 |
| `root` | 事件入口节点 ID（第一步执行的节点） |
| `steps` | 节点集合，key 为节点 ID |

## 触发类型

### 时间触发

```json
"trigger": { "type": "time_schedule", "method": "crossed_class_time" }
```

每天会在 08:50-12:15 之间按 10 分钟精度生成一个点名时间点；当游戏时间跨过这个点且玩家不在教室时触发。

### 交互触发

```json
"trigger": { "type": "interaction", "action_id": "dormitory_bed" }
```

当玩家在交互点按 Enter 时触发（暂未实现，保留接口）。

## 节点类型

### `display` — 展示文字

显示标题和正文，玩家按 Enter 后跳转。

```json
"intro": {
  "type": "display",
  "title": "欢迎",
  "body": "这是事件的第一段文字描述。",
  "footer": "Press Enter to continue",
  "then": "next_step"
}
```

| 字段 | 必需 | 说明 |
|------|------|------|
| `title` | 否 | 弹窗标题 |
| `body` | 否 | 弹窗正文 |
| `footer` | 否 | 底部提示（默认 "Press Enter to continue"） |
| `then` | 是 | 下一步节点 ID |

### `choice` — 玩家选择

显示 2-3 个选项，玩家按 1/2/3 键选择。

```json
"decide": {
  "type": "choice",
  "title": "食堂",
  "body": "你想吃什么？",
  "options": [
    { "text": "套餐A", "then": "meal_a" },
    { "text": "套餐B", "then": "meal_b" },
    { "text": "不吃",   "then": "skip_meal" }
  ]
}
```

| 字段 | 必需 | 说明 |
|------|------|------|
| `title` | 否 | 弹窗标题 |
| `body` | 否 | 弹窗正文（选项说明） |
| `footer` | 否 | 底部提示（2选项默认 "Press 1 or 2"，3选项 "Press 1, 2, or 3"） |
| `options` | 是 | 选项数组（2-3 个），每项包含 `text`（显示文字）和 `then`（跳转节点 ID） |

交互: Num1 选第1项，Num2 选第2项，Num3 选第3项。2选项时 Enter 等价于选第1项。Escape 取消事件。

### `random_check` — 概率判定

按百分比概率分支，可附带时间推进和闪光效果。

```json
"roll_dice": {
  "type": "random_check",
  "probability": 45,
  "title": "点名中...",
  "body": "上课中途，老师开始点名。",
  "success": "caught",
  "failure": "safe",
  "time_advance": 90,
  "flash": "时间流逝..."
}
```

| 字段 | 必需 | 说明 |
|------|------|------|
| `probability` | 是 | 成功率 0-100 |
| `title` | 否 | **有则**先展示弹窗等 Enter 后再判定；**无则**静默判定直接跳转 |
| `body` | 否 | 弹窗正文 |
| `success` | 是 | 判定成功(roll < probability)跳转节点 ID |
| `failure` | 是 | 判定失败跳转节点 ID |
| `time_advance` | 否 | 判定前推进游戏时间(分钟) |
| `flash` | 否 | 判定前黑屏闪烁文字 |
| `delta` | 否 | 判定前属性修改（格式同 outcome） |

### `check` — 条件组合判定

对玩家当前状态做条件检测，支持多条件 AND/OR 组合，可附带时间推进和闪光效果。

```json
"check_requirements": {
  "type": "check",
  "conditions": [
    { "kind": "location", "place": "library" },
    { "kind": "stat", "stat": "academic", "op": ">=", "value": 80 },
    { "kind": "stat", "stat": "social", "op": ">=", "value": 60 }
  ],
  "require": "all",
  "title": "毕业条件检测",
  "body": "检查你是否满足毕业条件...",
  "then": "graduation",
  "else": "not_ready"
}
```

| 字段 | 必需 | 说明 |
|------|------|------|
| `conditions` | 是 | 条件数组（至少 1 条） |
| `require` | 否 | `"all"`(AND, 默认) 或 `"any"`(OR) |
| `title` | 否 | **有则**先展示弹窗等 Enter 后再判定；**无则**静默判定直接跳转 |
| `body` | 否 | 弹窗正文 |
| `then` | 是 | 条件通过跳转节点 ID |
| `else` | 是 | 条件失败跳转节点 ID |
| `time_advance` | 否 | 判定前推进游戏时间(分钟) |
| `flash` | 否 | 判定前黑屏闪烁文字 |
| `delta` | 否 | 判定前属性修改（格式同 outcome） |

#### 条件对象格式

**位置检测 (`location`)**
```json
{ "kind": "location", "place": "classroom" }
```
place 取值: `classroom`, `dormitory`, `cafeteria`, `gym`, `library`, `campus`

**属性数值 (`stat`)**
```json
{ "kind": "stat", "stat": "energy", "op": ">=", "value": 50 }
```
stat 取值: `energy`, `health`, `gold`, `san`, `academic`, `social`
op 取值: `>=` (大于等于), `<` (小于)

**游戏旗标 (`flag`)**
```json
{ "kind": "flag", "flag": "is_midterm_day" }
```
flag 取值: `is_midterm_day`（后续可扩展）

### `outcome` — 结局节点

应用属性修改、推进时间、显示结果、结束事件。

```json
"happy_end": {
  "type": "outcome",
  "title": "大成功！",
  "body": "一切顺利，你感到非常满足。",
  "delta": { "san": 10, "academic": 5 },
  "time_advance": 30,
  "flash": "活动进行中..."
}
```

| 字段 | 必需 | 说明 |
|------|------|------|
| `title` | 否 | 结果弹窗标题 |
| `body` | 否 | 结果弹窗正文 |
| `delta` | 否 | 属性修改，键: `energy`,`health`,`gold`,`san`,`academic`,`social` |
| `time_advance` | 否 | 推进游戏时间(分钟) |
| `flash` | 否 | 黑屏闪烁文字 |

## 完整示例：上课点名事件

```
事件流程:
  上课时间到 → 检测位置
    ├─ 在教室 → 自动上课 (+academic, -energy, -san) → [结束]
    └─ 不在教室 → 时间跳到当天随机点名时间，老师点名
                   ├─ 45% 点名 → 社交检定(social ≥ 50?)
                   │               ├─ 通过 → 同学代答 → [结束]
                   │               └─ 失败 → 逃课被抓(-san,-social) → [结束]
                   └─ 55% 没点 → 侥幸逃脱(-academic) → [结束]
```

详见 `assets/config/events/class_attendance.json`。

## 设计原则

1. **有 title 的判定节点** = 先展示剧情再判定，给玩家阅读时间
2. **无 title 的判定节点** = 静默判定，直接跳转（适合纯逻辑分支）
3. **outcome 总是终止事件**，显示结果弹窗后清除事件状态
4. **不同日子的变体事件**建独立 JSON 事件，用不同 trigger 区分（如期中考试日再创建一个 `class_attendance_midterm` 事件，概率更严，trigger 加 flag 检测）
