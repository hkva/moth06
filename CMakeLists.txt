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
else()
    # Assume GCC-like
    target_compile_options(hk PUBLIC
        -Wall -Wextra
    )
endif()

# C++23
target_compile_features(hk PUBLIC cxx_std_23)

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
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.89.9/imgui.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.89.9/imgui_demo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.89.9/imgui_draw.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.89.9/imgui_tables.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.89.9/imgui_widgets.cpp"

    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.89.9/backends/imgui_impl_sdl2.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.89.9/backends/imgui_impl_sdlrenderer2.cpp"
)
target_include_directories(imgui PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui-1.89.9"
)
target_compile_features(imgui PRIVATE cxx_std_23)

#
# Dep: FluidSynth
# https://www.fluidsynth.org/
#
if(WIN32)
    set(FLUIDSYNTH_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/thirdparty/fluidsynth-2.3.4-win64/include")
    set(FLUIDSYNTH_LIBRARIES    "${PROJECT_SOURCE_DIR}/thirdparty/fluidsynth-2.3.4-win64/lib/fluidsynth.lib")
elseif(APPLE)
    # Homebrew
    set(FLUIDSYNTH_INCLUDE_DIRS "/opt/homebrew/include")
    set(FLUIDSYNTH_LIBRARIES    "/opt/homebrew/lib/libfluidsynth.dylib")
else()
    # XXX: Linux
endif()

#
# Dep: SDL
# https://www.libsdl.org/
#
if(WIN32)
    set(SDL_INCLUDE_DIRS    "${PROJECT_SOURCE_DIR}/thirdparty/SDL2-2.28.5-win64/include")
    set(SDL_LIBRARIES       "${PROJECT_SOURCE_DIR}/thirdparty/SDL2-2.28.5-win64/lib/x64/SDL2.lib")
    set(SDL_DLLS            "${PROJECT_SOURCE_DIR}/thirdparty/SDL2-2.28.5-win64/lib/x64/SDL2.dll")
elseif(APPLE)
    # Homebrew
    set(SDL_INCLUDE_DIRS    "/opt/homebrew/include/SDL2")
    set(SDL_LIBRARIES       "/opt/homebrew/lib/libSDL2.dylib")
else()
    # XXX: Linux
endif()
target_include_directories(imgui PRIVATE ${SDL_INCLUDE_DIRS})
target_link_libraries(imgui PRIVATE ${SDL_LIBRARIES})

#
# Dep: stb_image
# https://github.com/nothings/stb/blob/master/stb_image.h
#
add_library(stb_image STATIC "${CMAKE_CURRENT_LIST_DIR}/thirdparty/stb/stb_image.c")
target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_LIST_DIR}/thirdparty/stb")
