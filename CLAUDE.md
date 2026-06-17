# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

### vcpkg（推荐）

```bash
cmake --preset vcpkg -S . -B build && cmake --build build
./build/Debug/CampusLifeSimulator.exe
```

中文版：`cmake --preset vcpkg -S . -B build -DCLS_LANG_CHINESE=ON`

### macOS Homebrew

```bash
brew install sfml nlohmann-json curl
cmake --preset homebrew -S . -B build && cmake --build build
```

### MSYS2 UCRT64（Windows 备选）

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,sfml,nlohmann-json,curl}
cmake -S . -B build-msys2 -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-msys2
```

依赖：SFML 3.0 (Graphics/Window/System)、nlohmann-json、libcurl。`CMakeLists.txt` 使用 `file(GLOB_RECURSE)` 自动发现源文件，新增 `.h/.cpp` 无需修改 CMake。

## Architecture

C++20 + SFML 3.0 像素风校园生活模拟器。渲染到 320×180 纹理后 3× 放大到 960×540。

### 顶层流程

`main.cpp` 拥有所有系统对象（Player、地图、TimeSystem、UI 组件等），通过 `GameContext` struct 共享状态。主循环内联，只有 3 个顶层屏幕：`TITLE → DIFFICULTY → GAME`。无 Game 类、无状态机。

### 模块结构

| 目录 | 职责 |
|------|------|
| `src/core/` | 属性系统（CharacterState.h）、时间系统、战斗系统、GameContext、本地化 |
| `src/entity/` | Entity → Character → Player/Enemy；Character 持有 `Attributes attributes` + `HiddenMap mHidden` 两套变量 |
| `src/event/` | JSON 事件框架：EventRunner 加载 `assets/config/events/*.json` 并驱动分支节点图 |
| `src/interaction/` | **仅剩** CafeteriaInteraction（三餐选餐）+ DormitoryInteraction（睡觉），其余已被 JSON 事件取代 |
| `src/map/` | BuildingInterior 基类 + 室内地图，交互点配置从 `assets/config/interiors/*.json` 加载 |
| `src/ui/` | ModalBox、HUD、TitleScreen、DifficultyPanel、ChoicePrompt、ActivityNotice 等 |
| `src/fileio/` | SaveManager、JSON 持久化 |

### 数据流

```
键盘 Enter → handleInteraction(ip)
  → EventRunner::triggerByAction(actionId)  // JSON 事件优先
  → CafeteriaInteraction (三餐) / DormitoryInteraction (睡觉)
  → activityNotice 兜底

时间推进 → runTimedActivity → checkEventTriggers → EventRunner::checkTriggers
  → time_schedule 触发（如 crossed_class_time → class_attendance 事件）
```

### 属性与隐藏变量

`src/core/CharacterState.h` 统一定义：
- `Attributes` — 六个可见属性（energy/health/gold/san/academic/social），值域 0-100/gold 0-9999
- `HiddenMap`（`nlohmann::json`）— 隐藏变量（teacherTrust、friendBond、innovationProgress 等），int 累加、bool/string 覆盖。详见 `docs/数据说明.md`

Character 基类同时持有两者：`attributes` + `mHidden`。JSON 事件通过 `delta`（可见）+ `hidden_delta`（隐藏）修改。

### JSON 事件系统

`EventRunner` (`src/event/`) 加载 `assets/config/events/` 下全部 `.json` 文件，24 个事件。

节点类型：`display` → `choice` → `check`（location/stat/flag 组合 AND/OR）→ `random_check` → `outcome`

触发类型：
- `time_schedule` — 时间跨越触发，由 `runTimedActivity` 驱动
- `interaction` — 交互点触发，由 `handleInteraction` 驱动

格式文档见 `docs/events_schema.md`。

### 时间与日程

TimeSystem 管理 14 天周期，时间不自动流逝，随玩家交互推进。每天 08:00 起始，08:50 由 JSON 事件 `class_attendance` 检测位置决定上课/逃课分支。12:00-14:00、17:00-19:00 为饭点。第 7 天为期中考日。

### 战斗系统

`namespace CombatSystem`（`src/core/CombatSystem.h`）提供两套 API：
- `trySpawnEnemy(GameContext&)` / `fightNearestEnemy(GameContext&)` — 当前 main.cpp 使用
- `trySpawnEnemy(Player&, vector<Enemy>&, int&)` / `findNearestEnemy()` / `resolveRoll()` — 显式参数重载

d20 对抗：`d20 + (对应属性 - 50) / 10 + buff` vs 敌人 DC。情绪→属性映射见 `src/entity/CombatHelper.h`。

### 本地化

`cls::text(key)` 返回当前语言文本，`cls::format(key, {{"var","val"}})` 支持变量替换。翻译表在 `src/core/Localization.cpp` 硬编码。`#ifdef CLS_LANG_CHINESE` 编译期选择语言（CMake 选项 `CLS_LANG_CHINESE`）。

## Code Conventions

- **中文注释**，Doxygen `@brief/@param/@return` 格式
- **UI 字符串**：用 `cls::text(key)` 查表，`cls::makeText(font, text, size)` 创建 `sf::Text`（内部调 `sf::String::fromUtf8()` 正确渲染 UTF-8 中文）。**禁止直接用 `sf::Text(font, string, size)` 构造含中文的文本**。
- Include guard：`CLS_DIR_FILE_H` 模式
- 属性修改后调 `clampAttributes()`，坐标用 `float` 像素坐标
- 文件编码：**UTF-8 without BOM**，MSVC 编译已加 `/utf-8`
- 跨平台字体：Windows 优先 `msyh.ttc` → `msyh.ttf` → `simhei.ttf` → `arial.ttf`
