# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
cmake -S . -B build && cmake --build build   # configure + compile
./build/SLS                                   # run
```

Dependencies (macOS): `brew install sfml nlohmann-json curl`. Windows MSYS2 users pass `-DCMAKE_PREFIX_PATH=...` to cmake.

Source globs in CMakeLists.txt (`file(GLOB_RECURSE ...)`) auto-pick up new `.h`/`.cpp` files — no need to edit CMakeLists when adding sources. Re-run cmake only when adding dependencies.

## Architecture

Pixel-art campus life simulator with TRPG d20 combat, built with C++17 + SFML 3.0. Full design in `plan.md`.

**Planned 5 inheritance trees** (plan.md §继承体系):

| # | Chain | Levels | Status |
|---|-------|--------|--------|
| 1 | `Entity → Character → Player / Enemy` | 3 | Done |
| 2 | `Event → RandomEvent → LocalEvent / LLMEvent` | 3 | TODO |
| 3 | `GameState → ExplorationState / EventDialogState / CombatState` | 2 | TODO |
| 4 | `FileManager → JsonFileManager → SaveManager / ConfigManager` | 3 | TODO |
| 5 | `UIComponent → HUD / DialogBox` | 2 | TODO |

**Game loop** (state-machine driven): Exploration → event triggers → EventDialog → (if SAN low) → Combat → back to Exploration. Input and updates are dispatched to the current GameState.

**Pixel rendering**: render to `sf::RenderTexture` at 320×180, then upscale 3× to 960×540 with `setSmooth(false)`.

**Shared types** (`src/core/Types.h`): `Attributes` struct (san/energy/academic/social/gold, all 0-100 except gold 0-9999), `EmotionType` enum (ANXIETY/DEPRESSION/ANGER/FEAR/LONELINESS).

**SAN-triggered combat**: when player SAN drops below thresholds (30 → critical, 10 → dangerous), enemy difficulty scales with SAN level (0-3). Combat is d20 opposed rolls with attribute modifier `(stat - 50) / 10` (range [-5, +5]).

## Code Conventions

- **Comments in Chinese**, Doxygen `@brief/@param/@return` format — see `plan.md` line 484-513 for the exact templates. Every class and public method gets this comment block.
- **UI strings in English** — all `sf::Text`, `std::cout`, and JSON data strings use English. Comments remain Chinese.
- **Include guards**: `SLS_<DIR>_<FILE>_H` pattern (e.g., `SLS_CORE_TYPES_H`, `SLS_ENTITY_PLAYER_H`).
- **Delta time**: all `update()` methods receive `float deltaTime` in seconds.
- **Attribute clamping**: always call `clampAttributes()` after modifying character stats.
- **SFML coordinate system**: positions are `float` pixel coordinates, not tile indices.
- **API keys**: store in `assets/config/settings.json` (git-ignored via `.gitignore`).

## Current Status

Phase 1 complete (entity inheritance). `main.cpp` exists as a class demo showcasing all inheritance trees:
- Page 1: Entity demo — exploration map + SAN-threshold enemy spawning + d20 combat
- Page 2-4: Quest demos (SimpleQuest / MidtermExam / FinalExam)
- Page 5: QuestManager demo (JSON factory + quest chain)

Next: map system, game loop, and state machine (Phases 2-3 in plan.md).
