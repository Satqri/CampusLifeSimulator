cmake_minimum_required(VERSION 3.20)
project(CampusLifeSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Platform-specific hints ──────────────────────────────
# 优先使用 vcpkg manifest 模式 (cmake --preset vcpkg)
# 备选: macOS Homebrew 自动搜索 /opt/homebrew 和 /usr/local
if(APPLE AND EXISTS "/opt/homebrew")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
elseif(APPLE AND EXISTS "/usr/local")
    list(APPEND CMAKE_PREFIX_PATH "/usr/local")
endif()

# ── SFML ─────────────────────────────────────────────────
find_package(SFML COMPONENTS Graphics Window System REQUIRED)

# ── nlohmann/json (header-only) ──────────────────────────
find_package(nlohmann_json REQUIRED)

# ── libcurl ──────────────────────────────────────────────
find_package(CURL REQUIRED)

# ── Source files ─────────────────────────────────────────
set(CLS_HEADERS
    src/core/AssetPath.h
    src/core/Types.h
    src/entity/Character.h
    src/entity/Enemy.h
    src/entity/Entity.h
    src/entity/Player.h
    src/entity/Student.h
    src/map/BuildingInterior.h
    src/map/CafeteriaInterior.h
    src/map/CampusMap.h
    src/map/ClassroomInterior.h
    src/map/DormitoryInterior.h
    src/map/GymInterior.h
    src/map/LibraryInterior.h
    src/map/MapPortal.h
    src/quest/ExamQuest.h
    src/quest/FinalExamQuest.h
    src/quest/MainQuest.h
    src/quest/MidtermExamQuest.h
    src/quest/QuestManager.h
    src/quest/SimpleQuest.h
    src/state/CanteenGameState.h
    src/state/DiceGameState.h
    src/state/GameState.h
    src/state/MainQuestState.h
    src/ui/DialogBox.h
    src/ui/DifficultyPanel.h
    src/ui/HelpPanel.h
    src/ui/HUD.h
    src/ui/QuestPanel.h
    src/ui/SceneBackground.h
    src/ui/TitleScreen.h
    src/ui/UIComponent.h
)

set(CLS_SOURCES
    src/main.cpp
    src/core/AssetPath.cpp
    src/entity/Character.cpp
    src/entity/Enemy.cpp
    src/entity/Player.cpp
    src/entity/Student.cpp
    src/map/BuildingInterior.cpp
    src/map/CafeteriaInterior.cpp
    src/map/CampusMap.cpp
    src/map/ClassroomInterior.cpp
    src/map/DormitoryInterior.cpp
    src/map/GymInterior.cpp
    src/map/LibraryInterior.cpp
    src/quest/ExamQuest.cpp
    src/quest/FinalExamQuest.cpp
    src/quest/MainQuest.cpp
    src/quest/MidtermExamQuest.cpp
    src/quest/QuestManager.cpp
    src/quest/SimpleQuest.cpp
    src/state/CanteenGameState.cpp
    src/state/DiceGameState.cpp
    src/state/MainQuestState.cpp
    src/ui/DialogBox.cpp
    src/ui/DifficultyPanel.cpp
    src/ui/HelpPanel.cpp
    src/ui/HUD.cpp
    src/ui/QuestPanel.cpp
    src/ui/SceneBackground.cpp
    src/ui/TitleScreen.cpp
)

add_executable(CampusLifeSimulator ${CLS_SOURCES} ${CLS_HEADERS})

if(MSVC)
    add_compile_options(/utf-8)
endif()

target_include_directories(CampusLifeSimulator PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(CampusLifeSimulator PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

target_compile_options(CampusLifeSimulator PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/W4,-Wall -Wextra>)
