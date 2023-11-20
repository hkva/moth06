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
# Moth06
#
add_executable(moth06
    "${CMAKE_CURRENT_LIST_DIR}/moth06.cc"
    "${CMAKE_CURRENT_LIST_DIR}/moth06_fs.cc"
    "${CMAKE_CURRENT_LIST_DIR}/moth06_gfx.cc"
    "${CMAKE_CURRENT_LIST_DIR}/moth06_hash.cc"
    "${CMAKE_CURRENT_LIST_DIR}/moth06_pbg.cc"
)

# Win32 application
if(WIN32)
    set_target_properties(moth06 PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Compiler options
if(MSVC)
    target_compile_options(moth06 PRIVATE
        /W4
    )
else()
    # Assume GCC-like
    target_compile_options(moth06 PRIVATE
        -Wall -Wextra -g
    )
endif()

# C++23
target_compile_features(moth06 PRIVATE cxx_std_23)

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
target_link_libraries(moth06 PRIVATE
    imgui
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
target_include_directories(moth06 PRIVATE ${FLUIDSYNTH_INCLUDE_DIRS})
target_link_libraries(moth06 PRIVATE ${FLUIDSYNTH_LIBRARIES})

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
target_include_directories(moth06 PRIVATE ${SDL_INCLUDE_DIRS})
target_link_libraries(moth06 PRIVATE ${SDL_LIBRARIES})
target_include_directories(imgui PRIVATE ${SDL_INCLUDE_DIRS})
target_link_libraries(imgui PRIVATE ${SDL_LIBRARIES})

#
# Dep: stb_image
# https://github.com/nothings/stb/blob/master/stb_image.h
#
add_library(stb_image STATIC "${CMAKE_CURRENT_LIST_DIR}/thirdparty/stb/stb_image.c")
target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_LIST_DIR}/thirdparty/stb")
target_link_libraries(moth06 PRIVATE stb_image)
