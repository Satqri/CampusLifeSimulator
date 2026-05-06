# 校园生活模拟器 (Campus Life Simulator) — 开发计划 v3

## Context

C++ 课程项目。一款 SFML 像素风格校园生活模拟游戏：玩家在校园地图上移动，遭遇随机事件改变属性（事件可接入 LLM API 动态生成），当 SAN 值过低时触发与负面情绪的 TRPG d20 拼点战斗。

**必须满足的课程要求：**

| 要求 | 满足方案 |
|---|---|
| ≥5 个类 | 设计 15+ 个类 |
| ≥2 层继承层次 | 3 条继承链达到 3 层 |
| 封装/继承/多态 | private 成员 + virtual 接口 + 多级继承 |
| 随机文件读写处理 | 地图读取、JSON 配置读写、存档系统、游戏日志 |
| ≥2000 行代码 | 模块化分解，预估 2500+ 行 |
| UI 界面 | SFML 图形界面 (HUD/对话框/战斗UI) |
| 详细注释 | 每个类/函数按规定格式注释 |

---

## 技术栈

| 层 | 选型 |
|---|---|
| 语言 | C++17 |
| 图形 | SFML 2.6 |
| 构建 | CMake 3.20+ |
| JSON | nlohmann/json (header-only) |
| HTTP | libcurl |
| 地图 | 自定义文本格式 |

---

## 项目目录结构

```
├── CMakeLists.txt                  # 跨平台构建 (SFML + nlohmann + libcurl)
├── plan.md                         # 本文档
├── assets/
│   ├── maps/
│   │   └── campus.map              # 校园地图文本
│   ├── config/
│   │   ├── events.json             # 本地事件池
│   │   ├── enemies.json            # 敌人数据
│   │   └── settings.json           # 游戏设置 + API Key
│   └── fonts/
│       └── pixelfont.ttf
├── src/
│   ├── main.cpp                    # 程序入口
│   ├── core/
│   │   ├── Types.h                 # [DONE] 公共类型 (Attributes, EmotionType)
│   │   ├── Game.h / Game.cpp       # 主循环 + 状态管理
│   │   └── AssetManager.h / .cpp   # 资源加载
│   ├── entity/
│   │   ├── Entity.h                # [DONE] 实体抽象基类
│   │   ├── Character.h / .cpp      # [DONE] 角色基类 (继承Entity)
│   │   ├── Player.h / Player.cpp   # [DONE] 玩家 (继承Character)
│   │   └── Enemy.h / Enemy.cpp     # [DONE] 敌人 (继承Character)
│   ├── map/
│   │   ├── Tile.h                  # 瓦片数据
│   │   ├── TileMap.h / TileMap.cpp # 地图渲染
│   │   └── MapLoader.h / .cpp      # 地图文件解析
│   ├── event/
│   │   ├── Event.h                 # 事件抽象基类
│   │   ├── RandomEvent.h / .cpp    # 随机事件基类 (继承Event)
│   │   ├── LocalEvent.h / .cpp     # 本地事件 (继承RandomEvent)
│   │   ├── LLMEvent.h / .cpp       # LLM事件 (继承RandomEvent)
│   │   ├── EventSystem.h / .cpp    # 事件管理器
│   │   └── LLMClient.h / .cpp      # LLM API客户端
│   ├── combat/
│   │   ├── Dice.h / Dice.cpp       # 骰子系统
│   │   └── CombatSystem.h / .cpp   # 战斗流程
│   ├── state/
│   │   ├── GameState.h             # 状态抽象基类
│   │   ├── ExplorationState.h/.cpp # 探索状态
│   │   ├── EventDialogState.h/.cpp # 事件对话状态
│   │   └── CombatState.h / .cpp    # 战斗状态
│   ├── fileio/
│   │   ├── FileManager.h           # 文件IO抽象基类
│   │   ├── JsonFileManager.h/.cpp  # JSON文件管理 (继承FileManager)
│   │   ├── SaveManager.h / .cpp    # 存档管理 (继承JsonFileManager)
│   │   └── ConfigManager.h / .cpp  # 配置管理 (继承JsonFileManager)
│   └── ui/
│       ├── UIComponent.h           # UI组件抽象基类
│       ├── HUD.h / HUD.cpp         # 状态栏
│       └── DialogBox.h / .cpp      # 对话框
```

---

## OOP 类继承体系设计 (核心)

### 继承树 1: 实体体系 (Entity → Character → Player/Enemy) 【3 层】 ✅ 已实现

```
Entity                       (抽象基类)
  ├── # posX, posY           (位置，封装为 protected)
  ├── # visible, active      (状态标志)
  ├── + update(dt) = 0       (纯虚函数 — 多态)
  ├── + render(window) = 0   (纯虚函数 — 多态)
  ├── + getPosition() const  (公共接口访问 protected 成员 — 封装)
  └── + setPosition(x, y)
        └── Character        (抽象基类)
              ├── # attributes: Attributes  (属性结构体)
              ├── # moveSpeed: float
              ├── # name: string
              ├── + move(dirX, dirY, dt) = 0  (纯虚)
              ├── + takeDamage(int)           (虚 — 可覆盖)
              ├── + getAttributes() const
              └── + clampAttributes()         (属性 clamp [0,100])
                    ├── Player             (具体类)
                    │     ├── - moveCooldown: float
                    │     ├── - moveCooldownTimer: float
                    │     ├── - combatBuffs: CombatBuffs
                    │     ├── - sprite: sf::RectangleShape
                    │     ├── + move(dirX, dirY, dt) override
                    │     ├── + modifyAttributes(delta)
                    │     ├── + isSanCritical() const    (< 30)
                    │     ├── + isSanDangerous() const   (< 10)
                    │     ├── + getSanLevel() const      (0-3)
                    │     ├── + getCombatBuffs()
                    │     └── + clearBuffs()
                    └── Enemy              (具体类)
                          ├── - emotionType: EmotionType
                          ├── - baseDC / baseAttack: int
                          ├── - scaledDC / scaledAttack: int
                          ├── - sprite: sf::RectangleShape
                          ├── + move(dirX, dirY, dt) override (空实现)
                          ├── + scaleWithSanLevel(sanLevel)
                          ├── + getDC() / getAttackPower() const
                          ├── + getBaseDC() / getBaseAttack() const
                          ├── + getEmotionType() const
                          ├── - colorForEmotion(type) static
                          └── - nameForEmotion(type) static
```

**新增类型:**
- `CombatBuffs` 结构体: `{ nextEventPositive: bool, nextRollModifier: int }`
- `Attributes` 默认值: SAN=80, energy=80, academic=60, social=60, gold=100
- 新增 `src/core/Types.h` 集中管理 `Attributes` 和 `EmotionType` 枚举

**OOP 体现:**
- **封装**: 所有数据成员为 private/protected，通过 public 接口访问
- **继承**: Entity → Character → Player/Enemy，共 3 层
- **多态**: `update()`, `render()`, `move()` 为虚函数，由子类不同实现；可通过 `Entity*` / `Character*` 指针统一操作

### 继承树 2: 事件体系 (Event → RandomEvent → LocalEvent/LLMEvent) 【3 层】

```
Event                        (抽象基类)
  ├── # title, description
  ├── # choices: vector<EventChoice>
  ├── + execute(player) = 0  (纯虚 — 多态)
  ├── + getTitle() const
  └── + getChoices() const
        └── RandomEvent      (抽象基类)
              ├── # region: string
              ├── # triggerChance: float
              ├── + canTrigger(region) const
              └── + loadFromFile(ifstream&) = 0
                    ├── LocalEvent         (具体类)
                    │     ├── - localPoolId: int
                    │     ├── + execute(player) override
                    │     └── + loadFromFile() override
                    └── LLMEvent           (具体类)
                          ├── - llmClient: LLMClient*
                          ├── - promptTemplate: string
                          ├── + execute(player) override
                          └── + loadFromFile() override
```

### 继承树 3: 游戏状态体系 (GameState → 三种具体状态) 【2 层】

```
GameState                    (抽象基类)
  ├── # game: Game*          (指向主游戏对象的指针)
  ├── + handleInput() = 0    (纯虚)
  ├── + update(dt) = 0       (纯虚)
  └── + render(window) = 0   (纯虚)
        ├── ExplorationState (具体类)
        ├── EventDialogState (具体类)
        └── CombatState      (具体类)
```

### 继承树 4: 文件IO体系 (FileManager → JsonFileManager → SaveManager/ConfigManager) 【3 层】

```
FileManager                  (抽象基类)
  ├── # filePath: string
  ├── + open() / close() = 0 (纯虚)
  ├── + read() = 0           (纯虚)
  └── + write(data) = 0      (纯虚 — 多态)
        └── JsonFileManager  (抽象基类)
              ├── # jsonData: json
              ├── + read() override
              ├── + write(data) override
              └── + parseJson() = 0
                    ├── SaveManager       (具体类)
                    │     ├── + saveGame(player, map)
                    │     ├── + loadGame(player, map)
                    │     └── + deleteSave()
                    └── ConfigManager    (具体类)
                          ├── + loadSettings()
                          ├── + updateSetting(key, val)  (随机更新)
                          └── + saveSettings()
```

### 继承树 5: UI 组件体系 (UIComponent → HUD / DialogBox) 【2 层】

```
UIComponent                  (抽象基类)
  ├── # visible: bool
  ├── # bounds: sf::FloatRect
  ├── + handleInput(event) = 0
  ├── + update(dt) = 0
  └── + render(window) = 0
        ├── HUD             (具体类)
        │     ├── - player: Player*
        │     ├── - attrBars: vector<sf::RectangleShape>
        │     └── + render(window) override
        └── DialogBox       (具体类)
              ├── - event: Event*
              ├── - selectedChoice: int
              ├── + handleInput(event) override
              └── + render(window) override
```

### 额外工具类 (满足 ≥5 类要求)

- `Dice` — 骰子掷骰工具类
- `LLMClient` — LLM API 客户端 (封装 HTTP 请求)
- `EventSystem` — 事件管理器 (组合模式，持有 Event 集合)
- `CombatSystem` — 战斗逻辑控制器
- `TileMap` — 地图数据与渲染
- `MapLoader` — 地图文件解析
- `AssetManager` — 资源管理器

**类总数: 20+，继承层级最多 3 层，满足所有 OOP 要求。**

---

## 随机文件处理 (File I/O)

满足 "Random file processing (writing, reading, and updating)" 要求:

| 文件 | 操作 | 说明 |
|---|---|---|
| `campus.map` | 读取 | 解析地图文本文件 |
| `events.json` | 读取 | 加载本地事件池 |
| `enemies.json` | 读取 | 加载敌人数据 |
| `settings.json` | 读取 + 更新 | 游戏设置，运行时修改并回写 |
| `savegame.json` | 写入 + 读取 + 更新 + 删除 | 存档系统：保存进度、读取存档、更新存档、删除存档 |
| `gamelog.txt` | 写入 + 读取 | 游戏日志：记录每次事件/战斗/属性变化 |

---

## 渲染方案

用 SFML `sf::RenderTexture` 实现像素风格:
1. 游戏内容渲染到低分辨率 RenderTexture (如 320×180)
2. RenderTexture 放大到窗口 (如 960×540, 3x)
3. `sf::Texture::setSmooth(false)` 保持锯齿
4. 简易像素精灵用 `sf::RectangleShape` + `sf::Text` 构建（无需美术资源）

---

## 核心数据结构

```cpp
// 属性 (值域 0-100，金钱 0-9999)
struct Attributes {
    int san;        // 理智 (Sanity) — 核心属性
    int energy;     // 体力
    int academic;   // 学业
    int social;     // 社交
    int gold;       // 金钱
};

// 事件选项
struct EventChoice {
    std::string text;           // 选项文本
    Attributes delta;           // 属性变化
    std::string outcomeText;    // 结果描述
};

// 骰子检定结果
struct DiceResult {
    int roll;       // d20 结果 (1-20)
    int modifier;   // 属性加值 (-5 ~ +5)
    int total;      // roll + modifier
    bool success;   // total >= DC ?
};

// 情绪类型
enum class EmotionType { ANXIETY, DEPRESSION, ANGER, FEAR, LONELINESS };

// 地图区域类型
enum class Region { LIBRARY, DORMITORY, CANTEEN, TEACHING, PLAYGROUND, ROAD, GRASS };

// 游戏状态
enum class StateType { EXPLORATION, EVENT_DIALOG, COMBAT, MENU, GAME_OVER };
```

---

## 游戏核心循环

```
┌──────────────────────────────────────────────────┐
│                 Game::run()                      │
│  while (window.isOpen()) {                      │
│    deltaTime = clock.restart();                 │
│    handleInput();    // 事件分发到当前 State     │
│    update(dt);       // 当前 State 逻辑更新      │
│    render();         // 清除 → 渲染到RT → 放大   │
│    window.display();                            │
│  }                                              │
└──────────────────────────────────────────────────┘
```

### 状态流转

```
        ┌──────────────┐
        │ EXPLORATION  │ ← 默认状态：地图移动
        └──┬───────┬───┘
  踩中事件 │       │ SAN < 30 时概率触发
           ▼       ▼
  ┌──────────┐  ┌──────────┐
  │EVENT_DLG │  │ COMBAT   │
  │对话框+选项│  │回合制战斗│
  └────┬─────┘  └────┬─────┘
       │ 选择完成     │ 战斗结束
       ▼              ▼
  ┌─────────────────────────┐
  │     EXPLORATION         │
  └─────────────────────────┘
```

---

## 战斗系统 (TRPG d20 拼点)

```
角色行动 → d20 + 属性加值 vs DC → 成功/失败
                │
    属性加值 = (属性值 - 50) / 10  范围: [-5, +5]
```

**玩家行动:**

| 行动 | 对抗类型 | 检定属性 | 基础 DC |
|---|---|---|---|
| 理性分析 | Anxiety | academic | 12 |
| 意志坚守 | Depression | san | 14 |
| 倾诉求助 | Loneliness | social | 12 |
| 发泄释放 | Anger | energy | 13 |

**敌人属性随 SAN 变化:**
- SAN 30-20: 敌人 DC +2
- SAN 20-10: 敌人 DC +4, 攻击力 +3
- SAN <10: 敌人 DC +6, 攻击力 +5

**胜负结算:**
- 胜利: SAN +20, 获得 buff (下次事件正面选项加成)
- 失败: SAN -15, 获得 debuff (下次检定 -2)
- 逃跑: 50% 成功率, SAN -5

---

## LLM API 集成

**调用时机:** 玩家在特定区域触发事件时（概率约 20%）

**Prompt 设计:**
```
你是一个校园生活模拟游戏的叙事引擎。根据以下信息生成一个随机事件：

玩家状态：SAN={}, 体力={}, 学业={}, 社交={}, 金钱={}
所在区域：{}
触发类型：{意外遭遇/日常事件/社交互动/学术挑战}

要求：
1. 叙述用中文，TRPG跑团风格
2. 提供 2 个选项，每个选项有不同的属性变化
3. 属性变化范围: -15 到 +15
4. 返回严格 JSON 格式

示例输出：
{ "title": "...", "description": "...", "choices": [...] }
```

**Fallback 机制:** API 超时/网络错误 → 自动使用 `events.json` 中的本地事件池

---

## 分阶段开发计划

### 阶段 1: 项目骨架 + 核心框架 (预估 2-3h) — 部分完成

**产出文件:** `main.cpp`, `Game.h/.cpp`, `AssetManager.h/.cpp`, `CMakeLists.txt`

- [x] CMake 项目 + SFML/nlohmann/libcurl 引入 (跨平台 macOS/Windows)
- [ ] `Game` 类: 窗口创建、游戏循环、deltaTime
- [ ] `AssetManager`: 字体加载、纹理管理
- [x] `Entity` 抽象基类 (位置、虚函数接口)
- [x] `Types.h`: Attributes 结构体 + EmotionType 枚举
- [ ] 确保编译运行，窗口显示
- **验证:** 窗口打开，显示背景色

### 阶段 2: 实体继承体系 + 地图系统 (预估 3-4h) — 实体部分已完成

**产出文件:** `Entity.h`, `Character.h/.cpp`, `Player.h/.cpp`, `Enemy.h/.cpp`, `Tile.h`, `TileMap.h/.cpp`, `MapLoader.h/.cpp`

- [x] `Entity` → `Character` → `Player` 完整继承链
- [x] `Player`: 属性、移动、SAN 检测、冷却系统
- [x] `Enemy` 基础结构 (情绪类型、DC 缩放、静态工厂方法)
- [x] `Character`: clampAttributes、takeDamage、属性访问
- [ ] `Tile` + `TileMap` 瓦片地图
- [ ] `MapLoader`: 解析 campus.map 文本文件 **(文件读取)**
- [ ] 玩家键盘移动 + 碰撞检测 + 相机跟随
- **验证:** 玩家在地图上移动，无法穿墙

### 阶段 3: 状态机 + UI 组件 (预估 2-3h)

**产出文件:** `GameState.h`, `ExplorationState.h/.cpp`, `UIComponent.h`, `HUD.h/.cpp`, `DialogBox.h/.cpp`

- [ ] `GameState` 抽象基类 → `ExplorationState` 具体实现
- [ ] `UIComponent` → `HUD` 属性条渲染
- [ ] `UIComponent` → `DialogBox` 对话框
- [ ] 状态切换逻辑集成到 `Game`
- [ ] HUD 实时反映属性变化
- **验证:** HUD 显示属性条，按键切换状态

### 阶段 4: 事件系统 + LLM 集成 (预估 4-5h)

**产出文件:** `Event.h`, `RandomEvent.h/.cpp`, `LocalEvent.h/.cpp`, `LLMEvent.h/.cpp`, `EventSystem.h/.cpp`, `LLMClient.h/.cpp`, `EventDialogState.h/.cpp`

- [ ] `Event` → `RandomEvent` → `LocalEvent` 继承链
- [ ] `Event` → `RandomEvent` → `LLMEvent` 继承链
- [ ] `EventSystem`: 区域触发、随机选择、应用结果
- [ ] `LocalEvent`: 从 `events.json` 加载 **(文件读取)**
- [ ] `LLMClient`: libcurl HTTP POST + JSON 解析
- [ ] LLM Prompt 工程 + 响应解析
- [ ] `EventDialogState`: 事件展示 + 选项交互
- [ ] Fallback: API 失败 → 本地事件池
- **验证:** 移动触发事件 → 对话框出现 → 选择 → 属性变化

### 阶段 5: 文件 IO + 存档系统 (预估 2-3h)

**产出文件:** `FileManager.h`, `JsonFileManager.h/.cpp`, `SaveManager.h/.cpp`, `ConfigManager.h/.cpp`

- [ ] `FileManager` → `JsonFileManager` → `SaveManager` 继承链 **(3层)**
- [ ] `FileManager` → `JsonFileManager` → `ConfigManager` 继承链 **(3层)**
- [ ] 存档: 保存/读取玩家属性、位置 **(文件写入+读取)**
- [ ] 配置: 运行时修改设置并回写 **(文件更新)**
- [ ] 游戏日志: 记录事件/战斗到 gamelog.txt **(文件写入+读取)**
- [ ] 主菜单: 新游戏/继续/设置
- **验证:** 存档 → 关闭游戏 → 读档 → 恢复状态

### 阶段 6: 战斗系统 (预估 3-4h)

**产出文件:** `Dice.h/.cpp`, `CombatSystem.h/.cpp`, `CombatState.h/.cpp`

- [ ] `Dice`: d20 掷骰 + 属性加值计算
- [ ] `CombatSystem`: 回合制流程 + 行动处理
- [ ] `Enemy` 完整实现 (继承 Character, 多态)
- [ ] `CombatState`: 战斗 UI + 行动菜单
- [ ] SAN 阈值触发机制
- [ ] 胜负结算 + buff/debuff
- **验证:** SAN<30 → 触发战斗 → 完整流程 → 结算

### 阶段 7: 程序入口 + 注释 + 文档 (预估 1-2h)

- [ ] `main.cpp` 完整启动流程
- [ ] 所有类按规定格式添加注释
- [ ] 边界处理 (属性 clamp、空指针检查)
- [ ] 代码格式化
- [ ] 撰写 proposal (≥1000 words) + project report (≥3000 words)
- **验证:** 编译 + 完整游戏流程 + 检查代码行数 ≥2000

---

## 类/函数注释规范

**类注释模板:**
```
/**
 * @class Player
 * @brief 玩家类，游戏中的玩家角色
 *
 * 继承关系: Entity → Character → Player (3层继承)
 * 属性: position(继承自Entity), attributes(继承自Character), moveCooldown, combatBuffs
 * 行为: 键盘移动、属性修改、SAN值检测
 * 派生关系: 无 (具体类，不再派生)
 */
```

**函数注释模板:**
```
/**
 * @brief 玩家在地图上按方向移动
 * @param direction 移动方向 (UP/DOWN/LEFT/RIGHT)
 * @param tileMap 地图引用，用于碰撞检测
 * @return true 移动成功, false 撞墙
 *
 * 算法步骤:
 * 1. 检查移动冷却时间是否已过
 * 2. 计算目标格子坐标
 * 3. 检测目标格子是否可通过
 * 4. 如可通过，更新位置并重置冷却; 否则返回 false
 */
```

---

## 验证清单

| 阶段 | 验证方法 |
|---|---|
| 1 | SFML 窗口打开，无崩溃 |
| 2 | 玩家在地图上移动，碰撞检测正确 |
| 3 | HUD 属性条显示，状态可切换 |
| 4 | 事件触发 → 对话框 → 选择 → 属性变化 |
| 5 | 存档 → 重启 → 读档 → 状态一致 |
| 6 | SAN低 → 战斗 → 骰子检定 → 结算 |
| 7 | `wc -l src/**/*.{h,cpp}` ≥ 2000, 所有注释符合规范 |
