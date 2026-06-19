cmake_minimum_required(VERSION 3.20)
project(CampusLifeSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
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

# ── 语言选项（cmake -DCLS_LANG_CHINESE=ON 启用中文）────
option(CLS_LANG_CHINESE "Build with Chinese UI (default: English)" OFF)

# ── Source files ─────────────────────────────────────────
file(GLOB_RECURSE CLS_HEADERS "src/*.h")
file(GLOB_RECURSE CLS_SOURCES "src/*.cpp")

add_executable(CampusLifeSimulator ${CLS_SOURCES} ${CLS_HEADERS})

if(CLS_LANG_CHINESE)
    target_compile_definitions(CampusLifeSimulator PRIVATE CLS_LANG_CHINESE)
endif()

target_include_directories(CampusLifeSimulator PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(CampusLifeSimulator PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

target_compile_options(CampusLifeSimulator PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/W4 /utf-8,-Wall -Wextra>)

add_custom_command(TARGET CampusLifeSimulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:CampusLifeSimulator>/assets"
)
