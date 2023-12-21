cmake_minimum_required(VERSION 3.15)
project(moth06)

#
# Generator settings
#

# Build to bin/
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

#
# HK standard library replacement
#

add_library(hk STATIC
    "${CMAKE_CURRENT_LIST_DIR}/src/hk.cc"
)
target_include_directories(hk PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/src"
)

# Additional compiler flags
if(MSVC)
    target_compile_options(hk PUBLIC
        /W4
    )
    target_compile_definitions(hk PUBLIC
        _CRT_SECURE_NO_WARNINGS
    )
else()
    # Assume GCC-like
    target_compile_options(hk PUBLIC
        -Wall -Wextra
    )
endif()

# C++23
target_compile_features(hk PUBLIC cxx_std_23)

#
# Game library
# NOTE(HK): This will eventually get an option to link statically for release builds
#

add_library(game SHARED
    "${CMAKE_CURRENT_LIST_DIR}/src/game.cc"
)
target_link_libraries(game PRIVATE hk)
set_target_properties(game PROPERTIES OUTPUT_NAME "moth06_game")

#
# Game executable
#

add_executable(moth06
    "${CMAKE_CURRENT_LIST_DIR}/src/moth06.cc"
    "${CMAKE_CURRENT_LIST_DIR}/src/moth06_gfx.cc"
)
target_link_libraries(moth06 PRIVATE hk SDL2::SDL2 SDL2::SDL2main imgui)
if(WIN32)
    set_target_properties(moth06 PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Tests
if(TRUE)
    enable_testing()

    add_executable("test-hk" "${CMAKE_CURRENT_LIST_DIR}/src/test_hk.cc")
    target_link_libraries("test-hk" PRIVATE "hk")
    add_test(NAME "test-hk" COMMAND "$<TARGET_FILE:test-hk>")
endif()

#
# Dep: Dear ImGui
# https://github.com/ocornut/imgui
#
add_library(imgui STATIC
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.90/imgui.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.90/imgui_demo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.90/imgui_draw.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.90/imgui_tables.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.90/imgui_widgets.cpp"

    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.90/backends/imgui_impl_sdl2.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.90/backends/imgui_impl_sdlrenderer2.cpp"
)
target_include_directories(imgui PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.90"
)
# C++23
target_compile_features(imgui PUBLIC cxx_std_23)
target_link_libraries(imgui PRIVATE
    SDL2::SDL2
)

#
# Dep: SDL
# https://www.libsdl.org/
#
set(SDL_SHARED  ON  CACHE BOOL "" FORCE)
set(SDL_STATIC  OFF CACHE BOOL "" FORCE)
set(SDL_TEST    OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/thirdparty/SDL-release-2.28.5")

