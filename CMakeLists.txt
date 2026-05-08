cmake_minimum_required(VERSION 3.20)
project(CampusLifeSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Platform-specific hints ──────────────────────────────
# macOS Homebrew 默认路径; Windows 下通过 -DCMAKE_PREFIX_PATH=... 传入
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
file(GLOB_RECURSE CLS_HEADERS "src/*.h")
file(GLOB_RECURSE CLS_SOURCES "src/*.cpp")

add_executable(CampusLifeSimulator ${CLS_SOURCES} ${CLS_HEADERS})

target_include_directories(CampusLifeSimulator PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(CampusLifeSimulator PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

target_compile_options(CampusLifeSimulator PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/W4,-Wall -Wextra>)
