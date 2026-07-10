# 隐藏变量配置 JSON Schema

## 概述

隐藏变量配置文件 `assets/config/hidden_variables.json` 是所有隐藏变量的**单一数据源**。C++ 代码不再硬编码任何变量名、clamp 范围或合并规则，全部从此文件加载。

| 加载时机 | 游戏启动时，settings 加载之后 |
| 加载代码 | `HiddenVariableConfig::loadFromFile()` → `initHiddenVariableConfig()` |
| 消费方 | `CharacterState.h` 中的 `isHiddenAssignmentKey()`、`clampHiddenInteger()`、`isScalableHiddenBenefit()` 等 |

## 顶层结构

```json
{
  "variables": { ... },
  "patterns": [ ... ]
}
```

| 字段 | 说明 |
|------|------|
| `variables` | 精确 key 到变量定义的映射 |
| `patterns` | 通配符模式规则，按数组顺序匹配（精确定义优先） |

---

## `variables` — 变量定义

### 完整字段

```json
"healthIndex": {
  "type": "int",
  "initial": 100,
  "min": 0,
  "max": 130,
  "merge": "accumulate",
  "scaling": "benefit"
}
```

### 字段说明

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `type` | `"int"` / `"bool"` / `"string"` | `"int"` | 值类型。若 `initial` 能推断类型则不必显式写 |
| `initial` | 对应类型 | `int`: 0, `bool`: false, `string`: `""` | 游戏开始时的初始值 |
| `min` | int | 无 | 整数 clamp 下限。不写 = 不限制 |
| `max` | int | 无 | 整数 clamp 上限。不写 = 不限制。须与 `min` 成对出现 |
| `merge` | `"accumulate"` / `"assign"` | `"accumulate"` | `accumulate`: 数值 + 新值；`assign`: 直接覆盖 |
| `scaling` | `"none"` / `"benefit"` / `"burden"` | `"none"` | 重复活动时的缩放方向。`benefit`: 收益递减；`burden`: 惩罚递增 |

### 缩写规则

满足以下条件时可以省略字段，让 JSON 更干净：

- `type` = `"int"` 且 `merge` = `"accumulate"` 且 `scaling` = `"none"` 且无需 clamp → **只需写 `"key": { "type": "int" }`**
- `type` = `"bool"` 且 `initial` = `false` → **只需写 `"key": { "type": "bool" }`**
- `type` = `"string"` 且 `initial` = `"none"` → **只需写 `"key": { "type": "string", "initial": "none" }`**

示例 —— 大部分 `*Count` 变量只用一行（clamp 被 `*Count` 模式规则覆盖）：

```json
"classAttendCount":  { "type": "int" },
"mealCount":         { "type": "int" },
"libraryVisitCount": { "type": "int" }
```

---

## `patterns` — 模式规则

用于匹配**动态生成的 key**（如每个事件独立计数的 `_event_*_count`）或**后缀通配**（如所有 `*Count`）。

```json
{
  "pattern": "prefix_suffix",
  "prefix": "_event_",
  "suffix": "_count",
  "min": 0,
  "max": 99,
  "merge": "accumulate",
  "scaling": "none"
}
```

### 字段说明

| 字段 | 说明 |
|------|------|
| `pattern` | `"prefix_suffix"` — 前缀 + 后缀同时匹配；`"suffix"` — 仅后缀匹配 |
| `prefix` | 仅 `prefix_suffix` 有效。匹配 key 开头 |
| `suffix` | 匹配 key 结尾 |
| `min` / `max` | 同变量定义 |
| `merge` | 同变量定义 |
| `scaling` | 同变量定义 |

### 现有规则（3 条）

| 匹配模式 | clamp | merge | 用途 |
|----------|-------|-------|------|
| `_event_*_count` | [0, 99] | accumulate | 事件触发计数（如 `_event_class_attendance_count`） |
| `_event_*_last_day` | [-1, 14] | assign | 事件上次触发日（防止同日重复触发） |
| `*Count` | [0, 99] | accumulate | 所有 `*Count` 后缀整数的兜底 clamp |

### 匹配优先级

1. 先在 `variables` 中精确查找 key
2. 找不到则按 `patterns` 数组顺序匹配，命中即停

这意味着同名精确定义会**覆盖**模式规则。

---

## 常用操作

### 新增一个累加型整数变量

```json
// 在 "variables" 中添加
"clubMeetingCount": { "type": "int" }
```

不需要写 min/max/merge/scaling——`*Count` 模式规则自动提供 [0, 99] clamp 和 `accumulate` merge。

### 新增一个带自定义 clamp 的变量

```json
"newScore": {
  "type": "int",
  "initial": 50,
  "min": 0,
  "max": 100,
  "merge": "accumulate"
}
```

### 新增一个 assignment 型变量（覆盖不累加）

```json
"currentMood": {
  "type": "string",
  "initial": "neutral",
  "merge": "assign"
}
```

### 新增一个受重复衰减影响的变量

```json
"teamMorale": {
  "type": "int",
  "min": 0,
  "max": 60,
  "scaling": "benefit"
}
```

`scaling: "benefit"` 表示连续重复同一活动时正面收益递减；`scaling: "burden"` 表示负面累积递增（如 `lateNightLevel`、`gameAddiction`）。

### 删除变量

1. 从 `hidden_variables.json` 删除该条目
2. 搜索整个项目确认无代码引用该 key
3. 确认 `docs/数据说明.md` 中没有过时信息

---

## 与旧系统的对应关系

| 旧位置（已删除） | 新机制 |
|------|--------|
| `CharacterState.h` — `isHiddenAssignmentKey()` 的 if/else 链 | JSON 中 `merge: "assign"` |
| `CharacterState.h` — `clampHiddenInteger()` 的 if/else 链 | JSON 中 `min` / `max` |
| `main.cpp` — `isScalableHiddenBenefit()` 的 10 key 列表 | JSON 中 `scaling: "benefit"` |
| `main.cpp` — `isScalableHiddenBurden()` 的 2 key 列表 | JSON 中 `scaling: "burden"` |
| `main.cpp` — `initializeHiddenState()` 的 54 行赋值 | JSON 中 `initial` 字段 |
| `CharacterState.h` — `_event_*_count` / `_event_*_last_day` / `*Count` 硬编码 | JSON 中 `patterns` 数组 |

---

## 相关文件

| 文件 | 用途 |
|------|------|
| `assets/config/hidden_variables.json` | **本 schema 的实例** |
| `src/core/HiddenVariableConfig.h` | 加载与查询实现 |
| `src/core/CharacterState.h` | 消费方：`clampHiddenInteger()` 等委托到这里 |
| `docs/数据说明.md` | 各变量的语义、设计方向与数值标定 |
