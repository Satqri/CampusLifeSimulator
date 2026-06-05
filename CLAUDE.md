# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

### 依赖项（三端统一：vcpkg manifest 模式）

项目使用 `vcpkg.json` + `CMakePresets.json` 管理依赖，一条命令即可安装并编译：

```bash
# 1. 安装 vcpkg（仅首次）
#    macOS:   brew install vcpkg
#    Windows: git clone https://github.com/Microsoft/vcpkg.git C:/vcpkg && C:/vcpkg/bootstrap-vcpkg.bat
#    Linux:   git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg && ~/vcpkg/bootstrap-vcpkg.sh

# 2. 设置环境变量 VCPKG_ROOT 指向 vcpkg 目录
#    macOS/Linux: export VCPKG_ROOT=/path/to/vcpkg
#    Windows:     setx VCPKG_ROOT C:/vcpkg

# 3. 一键配置 + 编译（vcpkg 会自动安装 sfml / nlohmann-json / curl）
cmake --preset vcpkg -S . -B build && cmake --build build

# 4. 运行
./build/CampusLifeSimulator
```

**依赖项清单**（见 `vcpkg.json`）：SFML (Graphics/Window/System)、nlohmann-json、libcurl。

**macOS 备选方案**（不装 vcpkg，直接用 Homebrew）：
```bash
brew install sfml nlohmann-json curl
cmake --preset homebrew -S . -B build && cmake --build build
```

### Docker（跨平台一键构建 + 运行）

不安装任何本地依赖，用 Docker 构建和运行 GUI 窗口：

```bash
# Windows (WSL2 + Docker Desktop):
#   WSLg 内建 GUI，无需额外配置
docker compose build && docker compose up

# macOS:
#   先装 XQuartz 并允许网络连接
brew install xquartz && open -a XQuartz
xhost +localhost
#   启动容器（TCP 转发 X11）
DISPLAY=host.docker.internal:0 docker compose up

# Linux:
docker compose build && docker compose up
```

Docker 多阶段构建：builder 用 vcpkg 安装 SFML 3.0 并编译，runtime 只保留二进制和运行时库。

**Docker 开发模式**（改代码不重建镜像）：

```bash
# 首次：构建 dev 镜像（预装所有依赖，一次性）
docker compose build dev

# 之后每次改代码，只需：
docker compose up dev
#   → 自动 cmake 配置 + 增量编译 + 运行
#   → 源码通过 volume 挂载，只重编译变更的 .cpp

# 退出：Ctrl+C，或 docker compose down

# 进容器调试：
docker compose run dev bash
```

Source globs in CMakeLists.txt (`file(GLOB_RECURSE ...)`) auto-pick up new `.h`/`.cpp` files — no need to edit CMakeLists when adding sources. Re-run cmake only when adding dependencies.

## Architecture

Pixel-art campus life simulator with TRPG d20 combat, built with C++17 + SFML 3.0. Full design in `plan.md`.

**Inheritance trees** (implemented + planned from plan.md §继承体系):

| # | Chain | Levels | Status |
|---|-------|--------|--------|
| 1 | `Entity → Character → Player / Enemy` | 3 | Done |
| 2 | `MainQuest → SimpleQuest` / `MainQuest → ExamQuest → MidtermExamQuest / FinalExamQuest` | 3 | Done |
| 3 | `GameState → MainQuestState` (+ planned: ExplorationState / EventDialogState / CombatState) | 2 | Partial |
| 4 | `Event → RandomEvent → LocalEvent / LLMEvent` | 3 | TODO |
| 5 | `FileManager → JsonFileManager → SaveManager / ConfigManager` | 3 | TODO |
| 6 | `UIComponent → HUD / DialogBox` | 2 | TODO |

Note: the Quest tree (row 2) was added during implementation — it's not in plan.md's original 5 trees.

**Quest hierarchy** (`src/quest/`):

```
MainQuest (abstract)           — questId, questName, currentPhase, choiceTexts/Outcomes
├── SimpleQuest                — 3-phase flow: Announce → Choice → Result
└── ExamQuest (abstract)       — multi-round d20 exam: Announce → Prep → Roll → Result → Final
    ├── MidtermExamQuest       — DC=14, 5 rounds, need 3 passes
    └── FinalExamQuest         — DC=16, 7 rounds, need 4 passes
QuestManager                   — JSON factory + event-threshold chain (loads quests.json)
```

Quest phases (`QuestPhase` enum): `NOT_STARTED → ANNOUNCEMENT → CHOICE/PREPARATION → EXAM_ROUND → ROUND_RESULT → FINAL_RESULT → COMPLETED`. SimpleQuest uses CHOICE; ExamQuest uses PREPARATION → EXAM_ROUND → ROUND_RESULT loop.

`QuestManager` maps JSON `"type"` strings to `MainQuestType` enum for factory creation. Quest chain is driven by `completedEventCount` meeting per-quest `threshold` values in `quests.json`.

**Game loop** (state-machine driven): Exploration → event triggers → EventDialog → (if SAN low) → Combat → back to Exploration. Input and updates are dispatched to the current GameState. Currently only `MainQuestState` is implemented; Exploration/EventDialog/Combat states are TODO.

**Pixel rendering**: render to `sf::RenderTexture` at 320×180, then upscale 3× to 960×540 with `setSmooth(false)`.

**Shared types** (`src/core/Types.h`): `Attributes` struct (san/energy/academic/social/gold, all 0-100 except gold 0-9999), `EmotionType` enum (ANXIETY/DEPRESSION/ANGER/FEAR/LONELINESS), `QuestPhase` enum, `MainQuestType` enum (factory key), `ExamRollResult` struct, `StateType` enum (EXPLORATION/EVENT_DIALOG/COMBAT/MAIN_QUEST/MENU/GAME_OVER).

**SAN-triggered combat**: when player SAN drops below thresholds (30 → critical, 10 → dangerous), enemy difficulty scales with SAN level (0-3). Combat is d20 opposed rolls with attribute modifier `(stat - 50) / 10` (range [-5, +5]). Each `EmotionType` maps to a different player stat for the opposed roll (e.g., ANXIETY → academic, ANGER → energy).

## Code Conventions

- **Comments in Chinese**, Doxygen `@brief/@param/@return` format — see `plan.md` line 484-513 for the exact templates. Every class and public method gets this comment block.
- **UI strings in English** — all `sf::Text`, `std::cout`, and JSON data strings use English. Comments remain Chinese.
- **Include guards**: `CLS_<DIR>_<FILE>_H` pattern (e.g., `CLS_CORE_TYPES_H`, `CLS_ENTITY_PLAYER_H`).
- **Delta time**: all `update()` methods receive `float deltaTime` in seconds.
- **Attribute clamping**: always call `clampAttributes()` after modifying character stats.
- **SFML coordinate system**: positions are `float` pixel coordinates, not tile indices.
- **Platform-specific code**: use compiler-predefined macros — `__APPLE__` / `_WIN32` / `__linux__`. These are auto-defined by the compiler, no build flags needed. E.g. font paths are resolved at compile time rather than runtime fallback:
  ```cpp
  #if defined(__APPLE__)
      font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
  #elif defined(_WIN32)
      font.openFromFile("C:/Windows/Fonts/arial.ttf");
  #elif defined(__linux__)
      font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
  #endif
  ```
- **API keys**: store in `assets/config/settings.json` (git-ignored via `.gitignore`).

## Current Status

Entity hierarchy + Quest system + QuestManager + MainQuestState all implemented. `main.cpp` is a class demo (not the final game loop) showcasing everything across 5 pages:
- Page 1: Entity demo — exploration map + SAN-threshold enemy spawning + d20 combat
- Page 2-4: Quest demos (SimpleQuest / MidtermExam / FinalExam) with full phase flow
- Page 5: QuestManager demo (JSON factory + quest chain threshold triggering)

No test framework yet.

Next: Exploration/Combat/EventDialog game states, map system, and the full state-machine game loop (Phases 2-3 in plan.md).
