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

#
# Game executable
#

add_executable(moth06
    "${CMAKE_CURRENT_LIST_DIR}/src/moth06.cc"
)
target_link_libraries(moth06 PRIVATE hk SDL2::SDL2 SDL2::SDL2main)

# Tests
if(TRUE)
    enable_testing()

    add_executable("test-hk" "${CMAKE_CURRENT_LIST_DIR}/src/test_hk.cc")
    target_link_libraries("test-hk" PRIVATE "hk")
    add_test(NAME "test-hk" COMMAND "$<TARGET_FILE:test-hk>")
endif()

#
# Dep: SDL
# https://www.libsdl.org/
#
set(SDL_SHARED  ON  CACHE BOOL "" FORCE)
set(SDL_STATIC  OFF CACHE BOOL "" FORCE)
set(SDL_TEST    OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/thirdparty/SDL-release-2.28.5")

