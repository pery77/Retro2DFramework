cmake_minimum_required(VERSION 3.20)

project(Retro2DFramework VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(R2D_BUILD_EXAMPLES "Build Retro2DFramework examples" ON)
option(R2D_BUILD_TOOLS "Build Retro2DFramework tools" ON)

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
    src/r2d_audio.c
    src/r2d_camera.c
    src/r2d_crt.c
    src/r2d_music.c
    src/r2d_sprite.c
    src/r2d_tilemap.c
)

target_include_directories(r2d
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/tinysoundfont
)

target_compile_definitions(r2d PRIVATE
    R2D_DEVELOPMENT_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)

target_link_libraries(r2d PUBLIC raylib)

if(MSVC)
    target_compile_options(r2d PRIVATE /W4)
else()
    target_compile_options(r2d PRIVATE -Wall -Wextra -Wpedantic)
endif()

function(r2d_copy_assets target_name)
    add_custom_target(${target_name}_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${target_name}>/assets
        COMMENT "Copying assets for ${target_name}"
        VERBATIM
    )
    add_dependencies(${target_name} ${target_name}_assets)
endfunction()

function(r2d_add_app target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE r2d)
    r2d_copy_assets(${target_name})
endfunction()

if(R2D_BUILD_EXAMPLES)
    r2d_add_app(r2d_sandbox examples/sandbox/main.c)
    r2d_add_app(r2d_collect examples/collect/main.c)

    if(WIN32)
        target_link_options(r2d_sandbox PRIVATE
            $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/SUBSYSTEM:WINDOWS>
            $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/ENTRY:mainCRTStartup>
            $<$<AND:$<CONFIG:Release>,$<NOT:$<C_COMPILER_ID:MSVC>>>:-mwindows>
        )
        target_link_options(r2d_collect PRIVATE
            $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/SUBSYSTEM:WINDOWS>
            $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/ENTRY:mainCRTStartup>
            $<$<AND:$<CONFIG:Release>,$<NOT:$<C_COMPILER_ID:MSVC>>>:-mwindows>
        )
    endif()
endif()

if(R2D_BUILD_TOOLS)
    r2d_add_app(r2d_sfx_editor tools/sfx_editor/main.c)
    r2d_add_app(r2d_midi_player tools/midi_player/main.c)
endif()
