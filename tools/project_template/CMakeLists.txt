cmake_minimum_required(VERSION 3.20)

project(@PROJECT_TARGET@ VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(R2D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(R2D_BUILD_TOOLS ON CACHE BOOL "" FORCE)
set(R2D_PACKAGE_ASSETS ON CACHE BOOL "" FORCE)
set(R2D_DIST_DIR "${CMAKE_BINARY_DIR}/dist" CACHE PATH "Output directory for packaged game distributions")

add_subdirectory("@R2D_FRAMEWORK_PATH@" external/Retro2DFramework)

r2d_add_packaged_app(@PROJECT_TARGET@ src/main.c)
r2d_add_game_package(@PROJECT_TARGET@)
r2d_use_windows_subsystem(@PROJECT_TARGET@)

