cmake_minimum_required(VERSION 3.20)

project(Retro2DFramework VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(R2D_BUILD_EXAMPLES "Build Retro2DFramework examples" ON)

find_package(raylib CONFIG QUIET)

if(NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 6.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

add_library(r2d STATIC
    src/r2d.c
    src/r2d_crt.c
)

target_include_directories(r2d
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(r2d PUBLIC raylib)

if(MSVC)
    target_compile_options(r2d PRIVATE /W4)
else()
    target_compile_options(r2d PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(R2D_BUILD_EXAMPLES)
    add_executable(r2d_sandbox examples/sandbox/main.c)
    target_link_libraries(r2d_sandbox PRIVATE r2d)

    if(WIN32)
        target_link_options(r2d_sandbox PRIVATE
            $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/SUBSYSTEM:WINDOWS>
            $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/ENTRY:mainCRTStartup>
            $<$<AND:$<CONFIG:Release>,$<NOT:$<C_COMPILER_ID:MSVC>>>:-mwindows>
        )
    endif()
endif()
