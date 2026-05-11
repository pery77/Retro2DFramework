cmake_minimum_required(VERSION 3.20)

project(Retro2DFramework VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(R2D_BUILD_EXAMPLES "Build Retro2DFramework examples" ON)
option(R2D_BUILD_TOOLS "Build Retro2DFramework tools" ON)
option(R2D_PACKAGE_ASSETS "Package app assets into per-target .assets files" ON)

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
    src/r2d_assets.c
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
    $<$<CONFIG:Debug>:R2D_DEVELOPMENT_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/">
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

add_executable(r2d_asset_pack tools/r2d_pack/main.c)

function(r2d_package_assets target_name source_file)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND $<TARGET_FILE:r2d_asset_pack>
            $<CONFIG>
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${target_name}>/${target_name}.assets
            ${CMAKE_CURRENT_SOURCE_DIR}/${source_file}
        DEPENDS r2d_asset_pack
        COMMENT "Packing release assets for ${target_name}"
        VERBATIM
    )
endfunction()

function(r2d_add_app target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE r2d)
    r2d_copy_assets(${target_name})
endfunction()

function(r2d_add_packaged_app target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE r2d)
    if(R2D_PACKAGE_ASSETS)
        r2d_package_assets(${target_name} ${source_file})
    else()
        r2d_copy_assets(${target_name})
    endif()
endfunction()

function(r2d_use_windows_subsystem target_name)
    if(WIN32)
        target_link_options(${target_name} PRIVATE
            $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/SUBSYSTEM:WINDOWS>
            $<$<AND:$<CONFIG:Release>,$<C_COMPILER_ID:MSVC>>:/ENTRY:mainCRTStartup>
            $<$<AND:$<CONFIG:Release>,$<NOT:$<C_COMPILER_ID:MSVC>>>:-mwindows>
        )
    endif()
endfunction()

if(R2D_BUILD_EXAMPLES)
    r2d_add_packaged_app(r2d_sandbox examples/sandbox/main.c)
    r2d_add_packaged_app(r2d_collect examples/collect/main.c)

    r2d_use_windows_subsystem(r2d_sandbox)
    r2d_use_windows_subsystem(r2d_collect)
endif()

if(R2D_BUILD_TOOLS)
    r2d_add_app(r2d_sfx_editor tools/sfx_editor/main.c)
    r2d_add_app(r2d_midi_player tools/midi_player/main.c)
endif()
