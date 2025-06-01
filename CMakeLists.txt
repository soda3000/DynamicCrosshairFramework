cmake_minimum_required(VERSION 3.21)

# Set project name and version
set(PLUGIN_NAME DynamicCrosshairFramework)
set(PLUGIN_VERSION 0.1.0)
project(${PLUGIN_NAME} VERSION ${PLUGIN_VERSION} LANGUAGES CXX)

# Set compile defitions
add_compile_definitions(PLUGIN_NAME="${PLUGIN_NAME}")
add_compile_definitions(PLUGIN_VERSION="${PLUGIN_VERSION}")

include_directories("include")

# External Packages

# Setup commonlibsse-ng
set(BUILD_TESTS OFF)
add_subdirectory("extern/commonlibsse" commonlibsse)

include_directories("extern/commonlibsse/include")

# clib-util
add_subdirectory("extern/CLibUtil" CLibUtil)
include_directories("extern/CLibUtil/include")

# detours
include_directories("extern/detours")

# imgui
set(IMGUI_SOURCES
    extern/imgui/imgui.cpp
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_tables.cpp
    extern/imgui/imgui_widgets.cpp
    extern/imgui/backends/imgui_impl_dx11.cpp
    extern/imgui/backends/imgui_impl_win32.cpp
)
# Add ImGui include directories
include_directories(
    "extern/imgui"
    "extern/imgui/backends" 
)

set(headers
    "include/CrosshairMonitor.h"
	"include/Globals.h"
    "include/PCH.h"
    "include/CrosshairUI.h"
    "include/Menu.h"
)

set(sources
    "src/CrosshairMonitor.cpp"
    "src/Globals.cpp"
    "src/CrosshairUI.cpp"
    "src/Menu.cpp"
    "src/main.cpp"
)

add_library("${PLUGIN_NAME}" SHARED ${sources} ${headers} ${IMGUI_SOURCES})

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PLUGIN_NAME} PRIVATE include/PCH.h) # <--- PCH.h is required!

target_include_directories(
	"${PLUGIN_NAME}"
    PUBLIC
    "include"
)

target_link_libraries(
    "${PLUGIN_NAME}"
    PRIVATE
    CommonLibSSE::CommonLibSSE
)