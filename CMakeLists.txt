cmake_minimum_required(VERSION 3.20)

project(Retro2DFramework VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(R2D_FRAMEWORK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Retro2DFramework source directory")

option(R2D_BUILD_EXAMPLES "Build Retro2DFramework examples" ON)
option(R2D_BUILD_TOOLS "Build Retro2DFramework tools" ON)
option(R2D_PACKAGE_ASSETS "Package app assets into per-target .assets files" ON)
option(R2D_ENABLE_LUAJIT "Enable LuaJIT scripting support" OFF)
set(R2D_DIST_DIR "${CMAKE_BINARY_DIR}/dist" CACHE PATH "Output directory for packaged game distributions")

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

if(R2D_ENABLE_LUAJIT)
    set(R2D_VENDORED_LUAJIT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/luajit")
    set(LUAJIT_ROOT "" CACHE PATH "LuaJIT install root")
    if("${LUAJIT_ROOT}" STREQUAL "" AND EXISTS "${R2D_VENDORED_LUAJIT_DIR}/src/lua.h")
        set(LUAJIT_ROOT "${R2D_VENDORED_LUAJIT_DIR}")
    endif()

    set(R2D_BUILD_VENDORED_LUAJIT OFF)
    if("${LUAJIT_ROOT}" STREQUAL "${R2D_VENDORED_LUAJIT_DIR}" AND MSVC)
        set(LUAJIT_INCLUDE_DIR "${R2D_VENDORED_LUAJIT_DIR}/src" CACHE PATH "LuaJIT include directory" FORCE)
        set(LUAJIT_LIBRARY "${R2D_VENDORED_LUAJIT_DIR}/src/lua51.lib")
        set(R2D_BUILD_VENDORED_LUAJIT ON)
    else()
        find_path(LUAJIT_INCLUDE_DIR
            NAMES lua.h
            HINTS ${LUAJIT_ROOT}
            PATH_SUFFIXES include include/luajit-2.1 include/luajit-2.0 src
        )
        find_library(LUAJIT_LIBRARY
            NAMES luajit-5.1 lua51 lua5.1 luajit
            HINTS ${LUAJIT_ROOT}
            PATH_SUFFIXES lib lib64 src
        )
    endif()

    if(NOT LUAJIT_INCLUDE_DIR OR NOT LUAJIT_LIBRARY)
        message(FATAL_ERROR "R2D_ENABLE_LUAJIT is ON, but LuaJIT was not found. Set LUAJIT_ROOT or LUAJIT_INCLUDE_DIR/LUAJIT_LIBRARY.")
    endif()
endif()

add_library(r2d STATIC
    src/r2d.c
    src/r2d_asset_cache.c
    src/r2d_assets.c
    src/r2d_audio.c
    src/r2d_camera.c
    src/r2d_cinematic.c
    src/r2d_collision.c
    src/r2d_crt.c
    src/r2d_debug.c
    src/r2d_entity.c
    src/r2d_grid.c
    src/r2d_input.c
    src/r2d_locale.c
    src/r2d_log.c
    src/r2d_hot_reload.c
    src/r2d_music.c
    src/r2d_palette.c
    src/r2d_particle.c
    src/r2d_radiance.c
    src/r2d_runtime.c
    src/r2d_save.c
    src/r2d_script.c
    src/r2d_state.c
    src/r2d_sprite.c
    src/r2d_time.c
    src/r2d_tilemap.c
    src/r2d_ui.c
)

target_include_directories(r2d
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/tinysoundfont
)

target_link_libraries(r2d PUBLIC raylib)

if(R2D_ENABLE_LUAJIT)
    if(R2D_BUILD_VENDORED_LUAJIT)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(R2D_MSVC_ARCH x64)
        else()
            set(R2D_MSVC_ARCH x86)
        endif()

        set(R2D_VS_DEV_CMD "${CMAKE_GENERATOR_INSTANCE}/Common7/Tools/VsDevCmd.bat")
        if(NOT EXISTS "${R2D_VS_DEV_CMD}")
            message(FATAL_ERROR "Vendored LuaJIT needs VsDevCmd.bat to build with MSVC, but it was not found at ${R2D_VS_DEV_CMD}.")
        endif()

        add_custom_command(
            OUTPUT ${LUAJIT_LIBRARY}
            COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/cmake/r2d_build_luajit_msvc.bat"
                "${R2D_VS_DEV_CMD}"
                "${R2D_MSVC_ARCH}"
                "${R2D_VENDORED_LUAJIT_DIR}/src"
            WORKING_DIRECTORY "${R2D_VENDORED_LUAJIT_DIR}/src"
            DEPENDS
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/r2d_build_luajit_msvc.bat"
                "${R2D_VENDORED_LUAJIT_DIR}/src/msvcbuild.bat"
                "${R2D_VENDORED_LUAJIT_DIR}/src/ljamalg.c"
            COMMENT "Building vendored LuaJIT"
            VERBATIM
        )
        add_custom_target(r2d_luajit_vendor DEPENDS ${LUAJIT_LIBRARY})
        add_dependencies(r2d r2d_luajit_vendor)
    endif()

    target_include_directories(r2d PRIVATE ${LUAJIT_INCLUDE_DIR})
    target_link_libraries(r2d PUBLIC ${LUAJIT_LIBRARY})
    target_compile_definitions(r2d PUBLIC R2D_HAS_LUAJIT=1)
else()
    target_compile_definitions(r2d PUBLIC R2D_HAS_LUAJIT=0)
endif()

if(MSVC)
    target_compile_options(r2d PRIVATE /W4)
else()
    target_compile_options(r2d PRIVATE -Wall -Wextra -Wpedantic)
endif()

function(r2d_copy_assets target_name)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
        add_custom_target(${target_name}_assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:${target_name}>/assets
            COMMENT "Copying assets for ${target_name}"
            VERBATIM
        )
        add_dependencies(${target_name} ${target_name}_assets)
    endif()
endfunction()

function(r2d_copy_runtime_config target_name)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/r2d.ini")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/r2d.ini
                $<TARGET_FILE_DIR:${target_name}>/r2d.ini
            COMMENT "Copying runtime config for ${target_name}"
            VERBATIM
        )
    endif()
endfunction()

function(r2d_copy_locale target_name)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/locale")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/locale
                $<TARGET_FILE_DIR:${target_name}>/locale
            COMMENT "Copying locale files for ${target_name}"
            VERBATIM
        )
    endif()
endfunction()

add_executable(r2d_asset_pack tools/r2d_pack/main.c)

add_custom_target(r2d_pack_game
    COMMENT "Packaging Retro2D game distributions"
)

function(r2d_package_assets target_name)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
        set(r2d_package_sources "")
        foreach(source_file IN LISTS ARGN)
            list(APPEND r2d_package_sources "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
        endforeach()

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND $<TARGET_FILE:r2d_asset_pack>
                $<CONFIG>
                ${CMAKE_CURRENT_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:${target_name}>/${target_name}.assets
                ${r2d_package_sources}
            DEPENDS r2d_asset_pack
            COMMENT "Packing release assets for ${target_name}"
            VERBATIM
        )
    endif()
endfunction()

function(r2d_add_app target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE r2d)
    r2d_copy_runtime_config(${target_name})
    r2d_copy_locale(${target_name})
    r2d_copy_assets(${target_name})
endfunction()

function(r2d_add_packaged_app target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE r2d)
    r2d_copy_runtime_config(${target_name})
    r2d_copy_locale(${target_name})
    r2d_copy_assets(${target_name})
    if(R2D_PACKAGE_ASSETS)
        r2d_package_assets(${target_name} ${source_file})
    endif()
endfunction()

function(r2d_add_packaged_app_sources target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE r2d)
    r2d_copy_runtime_config(${target_name})
    r2d_copy_locale(${target_name})
    r2d_copy_assets(${target_name})
    if(R2D_PACKAGE_ASSETS)
        r2d_package_assets(${target_name} ${ARGN})
    endif()
endfunction()

function(r2d_add_game_package target_name)
    set(package_target "r2d_pack_game_${target_name}")
    add_custom_target(${package_target}
        COMMAND ${CMAKE_COMMAND}
            -DR2D_DIST_TARGET=${target_name}
            -DR2D_DIST_EXECUTABLE=$<TARGET_FILE:${target_name}>
            -DR2D_DIST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DR2D_DIST_OUTPUT_DIR=${R2D_DIST_DIR}/$<CONFIG>
            -DR2D_DIST_CONFIG=$<CONFIG>
            -P ${R2D_FRAMEWORK_SOURCE_DIR}/cmake/r2d_pack_game.cmake
        DEPENDS ${target_name}
        COMMENT "Packaging final game folder for ${target_name}"
        VERBATIM
    )
    add_dependencies(r2d_pack_game ${package_target})
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
    r2d_add_packaged_app(r2d_hello_index examples/hello_index/main.c)
    r2d_add_packaged_app(r2d_input_example examples/input/main.c)
    r2d_add_packaged_app(r2d_ui_example examples/ui/main.c)
    r2d_add_packaged_app(r2d_audio_example examples/audio/main.c)
    r2d_add_packaged_app(r2d_state_example examples/state/main.c)
    r2d_add_packaged_app(r2d_collision_example examples/collision/main.c)
    r2d_add_packaged_app(r2d_particle_example examples/particles/main.c)
    r2d_add_packaged_app(r2d_palette_example examples/palette/main.c)
    r2d_add_packaged_app(r2d_time_example examples/time/main.c)
    r2d_add_packaged_app(r2d_save_example examples/save/main.c)
    r2d_add_packaged_app(r2d_script_example examples/script/main.c)
    r2d_add_packaged_app(r2d_radiance_example examples/radiance/main.c)
    r2d_add_packaged_app(r2d_template_game examples/template/main.c)
    r2d_add_packaged_app(r2d_platformer_example examples/platformer/main.c)
    r2d_add_packaged_app(r2d_topdown_example examples/topdown/main.c)
    r2d_add_packaged_app(r2d_collect examples/collect/main.c)
    r2d_add_packaged_app(r2d_relic_run examples/relic_run/main.c)
    r2d_add_packaged_app_sources(r2d_collection_example
        examples/collection/main.c
        examples/collection/launcher.c
        examples/collection/games/pong/pong_game.c
        examples/collection/games/bomber/bomber_game.c
        examples/collection/games/bomber/bomber_map.c
        examples/collection/games/bomber/bomber_player.c
    )

    r2d_add_game_package(r2d_hello_index)
    r2d_add_game_package(r2d_input_example)
    r2d_add_game_package(r2d_ui_example)
    r2d_add_game_package(r2d_audio_example)
    r2d_add_game_package(r2d_state_example)
    r2d_add_game_package(r2d_collision_example)
    r2d_add_game_package(r2d_particle_example)
    r2d_add_game_package(r2d_palette_example)
    r2d_add_game_package(r2d_time_example)
    r2d_add_game_package(r2d_save_example)
    r2d_add_game_package(r2d_script_example)
    r2d_add_game_package(r2d_radiance_example)
    r2d_add_game_package(r2d_template_game)
    r2d_add_game_package(r2d_platformer_example)
    r2d_add_game_package(r2d_topdown_example)
    r2d_add_game_package(r2d_collect)
    r2d_add_game_package(r2d_relic_run)
    r2d_add_game_package(r2d_collection_example)

    r2d_use_windows_subsystem(r2d_hello_index)
    r2d_use_windows_subsystem(r2d_input_example)
    r2d_use_windows_subsystem(r2d_ui_example)
    r2d_use_windows_subsystem(r2d_audio_example)
    r2d_use_windows_subsystem(r2d_state_example)
    r2d_use_windows_subsystem(r2d_collision_example)
    r2d_use_windows_subsystem(r2d_particle_example)
    r2d_use_windows_subsystem(r2d_palette_example)
    r2d_use_windows_subsystem(r2d_time_example)
    r2d_use_windows_subsystem(r2d_save_example)
    r2d_use_windows_subsystem(r2d_script_example)
    r2d_use_windows_subsystem(r2d_radiance_example)
    r2d_use_windows_subsystem(r2d_template_game)
    r2d_use_windows_subsystem(r2d_platformer_example)
    r2d_use_windows_subsystem(r2d_topdown_example)
    r2d_use_windows_subsystem(r2d_collect)
    r2d_use_windows_subsystem(r2d_relic_run)
    r2d_use_windows_subsystem(r2d_collection_example)
endif()

if(R2D_BUILD_TOOLS)
    r2d_add_app(r2d_sfx_editor tools/sfx_editor/main.c)
    r2d_add_app(r2d_midi_player tools/midi_player/main.c)
endif()
