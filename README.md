# Campus Life Simulator

像素风校园生活模拟器，C++17 + SFML 3.0。

## 构建 & 运行

### 推荐路线

Windows 默认使用项目自带脚本：

```powershell
.\run-game.bat mingw
```

该脚本会自动配置、编译并启动游戏。只有在明确需要 Visual Studio 生成器时，才使用：

```powershell
.\run-game.bat vs
```

跨平台或干净环境优先使用 vcpkg。**不要默认使用 MSYS2 UCRT64 shell 构建；MSYS2 只是最后兜底路线。**

推荐优先级：

1. Windows 本机开发：`.\run-game.bat mingw`
2. Windows + Visual Studio：`.\run-game.bat vs`
3. 跨平台/CI/新机器：vcpkg preset
4. macOS：Homebrew preset
5. MSYS2 UCRT64：仅当前面路线不可用，或用户明确要求 MSYS2 时使用

### Windows 一键运行（首选）

```powershell
cd C:\Users\ASUS\Desktop\demo2
.\run-game.bat mingw
```

脚本内部使用 `build-local` 目录，不需要手动进入 MSYS2 shell。

### vcpkg（推荐，跨平台）

```bash
# 首次：安装 vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:/vcpkg
C:/vcpkg/bootstrap-vcpkg.bat
setx VCPKG_ROOT C:/vcpkg

# 编译
cmake --preset vcpkg -S . -B build
cmake --build build

# 中文版：加 -DCLS_LANG_CHINESE=ON
cmake --preset vcpkg -S . -B build -DCLS_LANG_CHINESE=ON

# 运行
./build/Debug/CampusLifeSimulator.exe
```

### MSYS2 UCRT64（最后兜底）

仅在以下情况使用本路线：

- `run-game.bat mingw` / `run-game.bat vs` 不可用
- vcpkg preset 不可用
- 用户明确要求使用 MSYS2

不要把 MSYS2 作为 Windows 默认构建方式。

```bash
# 首次：安装依赖
pacman -S --needed \
  mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-sfml \
  mingw-w64-ucrt-x86_64-nlohmann-json \
  mingw-w64-ucrt-x86_64-curl

# 编译
cmake -S . -B build-msys2 -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-msys2

# 运行
./build-msys2/CampusLifeSimulator.exe
```

### macOS（Homebrew）

```bash
brew install sfml nlohmann-json curl
cmake --preset homebrew -S . -B build
cmake --build build
```

## 操作说明

### 标题界面

| 操作 | 效果 |
| --- | --- |
| `Enter` | 进入难度选择 |
| `H` | 帮助页面 |
| 鼠标点击 `New Semester` | 进入难度选择 |

### 难度选择

| 操作 | 效果 |
| --- | --- |
| `1` / `2` / `3` | 选择 Easy / Normal / Hard |
| `Enter` | 确认选择 |
| `Esc` | 返回标题 |

### 探索地图

| 操作 | 效果 |
| --- | --- |
| `WASD` / 方向键 | 移动玩家 |
| 鼠标点击地图 | 引导玩家移动到目标点 |
| `Enter` | 场景切换 / 交互点触发 |
| `C` | 触发压力事件（SAN -15） |
| `F` | 与附近敌人战斗 |
| `V` | 恢复 SAN（+15） |
| `X` | 设置测试战斗 buff（+2） |
| `K` | 保存游戏 |
| `L` | 读取存档 |
| `Ctrl+H` | 打开帮助 |
| `Ctrl+S` | 打开设置 |

### 可进入地点

- Dormitory（宿舍）
- Gym（健身房）
- Library（图书馆）
- Classroom（教室）
- Cafeteria（食堂）

底部绿色出口区域按 `Enter` 返回校园。

## 游戏系统

### 时间系统

14 天周期，每天 08:00 开始。时间随玩家交互推进，不自动流逝。

- 清晨 06:00-11:00 / 正午 11:00-14:00 / 下午 14:00-17:30 / 傍晚 17:30-18:30 / 夜晚 18:30-06:00
- 第 7 天为 Midterm Day
- 每天首次到 08:50 触发晨课事件

### 饭堂

- 开放时间：12:00-14:00、17:00-19:00
- Meal A（8G）：SAN+3 Energy+12 Social+1
- Meal B（15G）：SAN+6 Energy+20 Social+2
- Meal C（28G）：SAN+10 Energy+30 Academic+2 Social+4

### 图书馆

- 4 个书架，每次阅读消耗 30min，进度 +25%
- 书籍：Reference Methods / Literature Notes / Science Primer / Campus History

### 健身房

- 跑步机（40min）：SAN-4 Energy-14 Social+2
- 杠铃（40min）：SAN-5 Energy-16 Social+1

### 宿舍

- 睡觉（22:30 后）：推进到下一天 08:00，按睡眠时长恢复
- 书桌学习（45min）：SAN-6 Energy-10 Academic+7
- 打游戏（60min）：前 2 次 SAN+12 Energy+8，第 3 次起效果衰减
- 第 14 天睡觉后学期结束

### 战斗系统

d20 对抗检定：`d20 + (对应属性 - 50) / 10 + buff` vs 敌人 DC

| SAN 等级 | 敌人上限 | 生成概率 |
| --- | --- | --- |
| ≥30 | 0 | 不生成 |
| 20-29 | 1 | 40% |
| 10-19 | 2 | 60% |
| <10 | 3 | 90% |

| 情绪敌人 | 对抗属性 |
| --- | --- |
| Anxiety | Academic |
| Depression | SAN |
| Anger | Energy |
| Fear | SAN |
| Loneliness | Social |

## 项目结构

```
src/
├── core/        — 核心系统（类型、时间、本地化、战斗、设置、存档）
├── entity/      — 实体层级（Entity → Character → Player/Enemy）
├── interaction/ — 房间交互模块（5 个地点）
├── fileio/      — 文件 I/O（存档/配置管理）
├── map/         — 地图与室内场景
├── quest/       — 任务系统
├── ui/          — UI 组件
└── main.cpp
```
