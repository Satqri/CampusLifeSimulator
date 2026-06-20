# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

### Mandatory AI Build Policy

When working from an AI/assistant terminal in this repository, **do not use MSYS, MSYS2, UCRT64, MinGW, or `build-msys2` for build verification**.

Forbidden for AI terminal use:

- `cmake --build build-msys2`
- `cmake -S . -B build-msys2 ...`
- `.\run-game.bat mingw`
- `mingw32-make`, MSYS2 `ninja`, or `pacman`
- Direct compiler/tool paths under `D:\msys2\...`
- Direct compiler/tool paths under `C:\x86_64-*-mingw64\...`

The user's manual MSYS/MinGW workflow may work locally, but AI terminal sessions in this project have repeatedly failed on that route. Do not retry it unless the user explicitly overrides this rule in the current conversation.

Default AI verification command on Windows:

```powershell
cmake --build build --config Debug
```

If the Visual Studio `build` directory is not configured, configure/use Visual Studio instead of MSYS:

```powershell
.\run-game.bat vs
```

Only use vcpkg or Homebrew for non-Windows/cross-platform contexts when appropriate.

### Build Priority For Humans

For manual/user-driven Windows development, the repository runner is available:

```powershell
.\run-game.bat mingw
```

Human/manual route order:

1. Windows local development: `.\run-game.bat mingw`
2. Windows Visual Studio generator: `.\run-game.bat vs`
3. Cross-platform / CI / clean setup: vcpkg preset
4. macOS: Homebrew preset
5. MSYS2 UCRT64: manual use only, not AI terminal verification

### Windows runner（default on Windows）

```powershell
.\run-game.bat mingw
```

### vcpkg（推荐跨平台）

```bash
cmake --preset vcpkg -S . -B build && cmake --build build
./build/Debug/CampusLifeSimulator.exe
```

中文版：`cmake --preset vcpkg -S . -B build -DCLS_LANG_CHINESE=ON`

### macOS Homebrew

```bash
brew install sfml nlohmann-json curl
cmake --preset homebrew -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build build
```

### MSYS2 UCRT64（manual use only）

This section is for the user running commands manually. AI assistants must not use this route for build verification.

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,sfml,nlohmann-json,curl}
cmake -S . -B build-msys2 -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-msys2
```

依赖：SFML 3.0 (Graphics/Window/System)、nlohmann-json、libcurl。`CMakeLists.txt` 使用 `file(GLOB_RECURSE)` 自动发现源文件，新增 `.h/.cpp` 无需修改 CMake。

## Architecture

C++20 + SFML 3.0 像素风校园生活模拟器。渲染到 320×180 纹理后 3× 放大到 960×540。

### 顶层流程

`main.cpp` 拥有所有系统对象，通过 `GameContext` struct 共享状态。主循环内联，4 个顶层屏幕：`TITLE → SETTINGS/HELP → DIFFICULTY → GAME`。无 Game 类、无状态机。

### 模块结构

| 目录 | 职责 |
|------|------|
| `src/core/` | 核心游戏系统：CharacterState（属性+隐藏变量）、TimeSystem（时间日程）、CombatSystem（战斗）、SettlementResolver（结局）、ActivityRunner（限时活动）、SleepSystem（睡觉/闹钟）、GameContext（状态总线）、Localization（i18n） |
| `src/config/` | JSON 配置加载器：HiddenVariableConfig、LibraryConfig、MealConfig、SceneConfig |
| `src/utils/` | 纯工具（无业务依赖）：AssetPath（路径解析）、Log.h（调试 macro）、TextUtils（sf::Text 构造）、WindowScaler（窗口缩放） |
| `src/entity/` | Entity → Character → Player/Enemy；Character 持有 `Attributes attributes` + `HiddenMap mHidden` 两套变量 |
| `src/event/` | JSON 事件框架：EventRunner 加载 `assets/config/events/*.json`（24 个事件）并驱动分支节点图 |
| `src/interaction/` | CafeteriaInteraction（三餐选餐）+ DormitoryInteraction（睡觉）+ RegularInteraction（通用交互点） |
| `src/map/` | BuildingInterior 基类 + 6 栋室内地图（Dormitory/Gym/Library/Classroom/Cafeteria/Store），交互点配置从 `assets/config/interiors/*.json` 加载 |
| `src/quest/` | 主线任务系统：QuestManager 工厂创建 MainQuest 子类（Orientation/MidtermExam/FinalExam 等） |
| `src/ui/` | UI 组件：HUD、TitleScreen、DifficultyPanel、ModalBox、ChoicePrompt、ActivityNotice 等 |
| `src/fileio/` | SaveManager、ConfigManager、JSON 持久化 |

### 数据流

```
键盘 Enter → handleInteraction(ip)
  → EventRunner::triggerByAction(actionId)   // JSON 事件优先
  → CafeteriaInteraction (三餐) / DormitoryInteraction (睡觉)
  → RegularInteraction (通用交互点) 兜底

交互点/事件调用 → ctx.runTimedActivity / ctx.runTimedActivityWithHidden
  → ActivityRunner::promptTimedActivityDuration (可选自定义时长)
  → ActivityRunner::executeTimedActivity (时间推进 + 属性变化 + 事件触发)

睡觉 → ctx.sleepFromDormitory
  → SleepSystem::sleepFromDormitory (显示闹钟选项)
  → SleepSystem::executeSleep (执行睡觉逻辑)

跨天/活动触发 → ctx.checkEventTriggers → EventRunner::checkTriggers
  → time_schedule 触发（如 crossed_class_time → class_attendance 事件）
```

### 属性与隐藏变量

- `Attributes`（`CharacterState.h`）— 六个可见属性：energy/health/gold/san/academic/social，值域 0-100/gold 0-9999
- `HiddenMap`（`config/HiddenVariableConfig.h`，即 `nlohmann::json`）— 隐藏变量元数据定义在 `assets/config/hidden_variables.json`（**单一数据源**）

隐藏变量的 type/initial/min/max/merge/scaling 全部从 JSON 加载。C++ 代码通过 `isHiddenAssignmentKey()`、`clampHiddenInteger()`、`isScalableHiddenBenefit()` 等函数查询——这些函数内部委托到 `HiddenVariableConfig`，不再硬编码。新增隐藏变量只需编辑 JSON，不改 C++。

Schema 文档：`docs/hidden_variables_schema.md`。语义说明：`docs/数据说明.md`。

### 重复活动 & 时长缩放

重复同一活动触发收益递减/惩罚递增（`CharacterState.h`）：
- 正面收益：streak=2→80%，streak=3→60%，最低 40%
- 负面惩罚：streak=2→125%，streak=3→150%，最高 200%
- Benefit 类 key（如 teacherTrust）受益递减；Burden 类 key（如 lateNightLevel）惩罚递增

时长自定义（10-120 分钟），delta 按 `实际分钟 / 基础分钟` 线性缩放。

### JSON 事件系统

`EventRunner` (`src/event/`) 加载 `assets/config/events/` 下全部 `.json` 文件，24 个事件。格式文档：`docs/events_schema.md`。

节点类型：`display` → `choice` → `check`（location/stat/flag 组合 AND/OR）→ `random_check` → `outcome`

触发类型：
- `time_schedule` — 时间跨越触发，由 `ActivityRunner::executeTimedActivity` 驱动
- `interaction` — 交互点触发，由 `handleInteraction` 驱动

### 时间与日程

`TimeSystem` (`src/core/TimeSystem.h`) 管理 14 天周期。时间不自动流逝，随玩家交互推进。每天 08:00 起始。点名时间每日随机化（08:50-12:15 间 10 分钟精度）。

饭点：早餐 06:30-10:00、午餐 11:30-13:30、晚餐 16:40-19:30。第 7 天为期中考日。

时间工具函数（同文件，inline 自由函数）：
- `normalizedMinute(minute)` — 归一化到 [0, 1440)
- `isWithinClockWindow(minute, start, end)` — 时间段判断（支持跨日）
- `absoluteGameMinute(ts)` — 从第 1 天 00:00 起的绝对分钟
- `durationLabel(minutes)` — 格式化为可读时长

### 地点系统

`MapPortal.h` 定义 `CampusPlace` enum 及工具函数：
- `campusPlaceKey(place)` — 地点→内部 key 字符串
- `commuteMinutes(from, to)` — 两地通勤时间（5-15 分钟）
- `placeName(place)` — 地点→本地化显示名

### 战斗系统

`namespace CombatSystem`（`src/core/CombatSystem.h`）d20 对抗：`d20 + (对应属性 - 50) / 10 + buff` vs 敌人 DC。情绪→属性映射见 `src/entity/CombatHelper.h`。

### 本地化

`cls::text(key)` 返回当前语言文本，`cls::format(key, {{"var","val"}})` 支持变量替换。翻译表在 `src/core/Localization.cpp` 硬编码（953 条）。`#ifdef CLS_LANG_CHINESE` 编译期选择语言（CMake 选项 `CLS_LANG_CHINESE`）。

## Code Conventions

- **中文注释**，Doxygen `@brief/@param/@return` 格式
- **UI 字符串**：用 `cls::text(key)` 查表，`cls::makeText(font, text, size)` 创建 `sf::Text`（内部调 `sf::String::fromUtf8()` 正确渲染 UTF-8 中文）。**禁止直接用 `sf::Text(font, string, size)` 构造含中文的文本**。
- Include guard：`CLS_DIR_FILE_H` 模式
- 属性修改后调 `clampAttributes()`，坐标用 `float` 像素坐标
- 文件编码：**UTF-8 without BOM**，MSVC 编译已加 `/utf-8`
- 跨平台字体：Windows 优先 `msyh.ttc` → `msyh.ttf` → `simhei.ttf` → `arial.ttf`
- 新增隐藏变量：编辑 `assets/config/hidden_variables.json` 即可，无需改 C++ 代码
- 匿名 namespace 仅用于 main.cpp 内部胶水代码；业务逻辑放对应模块 header（inline 自由函数）
