#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define TILEMAP_RADIANCE_PATH "tilemaps/radiance_lighting.json"
#define TILEMAP_RADIANCE_MASK_TEXTURE_PATH "tilemaps/radiance_tiles.png"
#define TILEMAP_RADIANCE_WALL_TEXTURE_PATH "textures/DawnLike/Objects/Wall.png"
#define TILEMAP_RADIANCE_FLOOR_TEXTURE_PATH "textures/DawnLike/Objects/Floor.png"
#define TILEMAP_RADIANCE_CASCADE_SHADER_PATH "shaders/radiance_flatland_cascade.fs"
#define TILEMAP_RADIANCE_COMPOSE_SHADER_PATH "shaders/radiance_flatland_compose.fs"
#define TILEMAP_RADIANCE_PADDING 96
#define TILEMAP_RADIANCE_WATCH_COUNT 6

typedef struct TilemapRadianceWatch {
    const char *path;
    bool reload_tilemap;
    bool reload_radiance;
    R2D_FileWatch watch;
} TilemapRadianceWatch;

typedef struct RadianceTilemapExample {
    R2D_Context *context;
    R2D_Radiance *radiance;
    R2D_Crt *crt;
    R2D_Tilemap tilemap;
    Vector2 camera;
    int background_layer;
    int collision_layer;
    int lighting_layer;
    bool sky;
    bool padding_enabled;
    bool show_lighting_layer;
    bool show_collision_debug;
    bool hot_reload_enabled;
    bool pending_tilemap_reload;
    bool pending_radiance_reload;
    float reload_delay;
    float status_timer;
    char status[128];
    TilemapRadianceWatch watches[TILEMAP_RADIANCE_WATCH_COUNT];
    R2D_RadianceDebugView debug;
} RadianceTilemapExample;

static const TilemapRadianceWatch TILEMAP_RADIANCE_WATCHES[TILEMAP_RADIANCE_WATCH_COUNT] = {
    { TILEMAP_RADIANCE_PATH, true, false, { 0 } },
    { TILEMAP_RADIANCE_MASK_TEXTURE_PATH, true, false, { 0 } },
    { TILEMAP_RADIANCE_WALL_TEXTURE_PATH, true, false, { 0 } },
    { TILEMAP_RADIANCE_FLOOR_TEXTURE_PATH, true, false, { 0 } },
    { TILEMAP_RADIANCE_CASCADE_SHADER_PATH, false, true, { 0 } },
    { TILEMAP_RADIANCE_COMPOSE_SHADER_PATH, false, true, { 0 } }
};

static const char *TilemapRadiance_DebugName(R2D_RadianceDebugView view)
{
    switch (view) {
    case R2D_RADIANCE_DEBUG_MASK: return "mask";
    case R2D_RADIANCE_DEBUG_CASCADE: return "cascade";
    default: return "final";
    }
}

static void TilemapRadiance_SetStatus(RadianceTilemapExample *example, const char *status)
{
    snprintf(example->status, sizeof(example->status), "%s", status);
    example->status_timer = 2.5f;
}

static void TilemapRadiance_UseProjectAssets(void)
{
    char asset_dir[1024];

    snprintf(asset_dir, sizeof(asset_dir), "%s../../assets", GetApplicationDirectory());
    if (DirectoryExists(asset_dir)) {
        R2D_SetDevelopmentAssetDir(asset_dir);
    }
}

static float TilemapRadiance_WorldWidth(const RadianceTilemapExample *example)
{
    return (float)(example->tilemap.width * example->tilemap.tile_width);
}

static float TilemapRadiance_WorldHeight(const RadianceTilemapExample *example)
{
    return (float)(example->tilemap.height * example->tilemap.tile_height);
}

static void TilemapRadiance_ClampCamera(RadianceTilemapExample *example)
{
    float max_x;
    float max_y;

    if (!R2D_TilemapIsReady(&example->tilemap)) {
        example->camera = (Vector2) { 0.0f, 0.0f };
        return;
    }

    max_x = TilemapRadiance_WorldWidth(example) - (float)example->context->config.virtual_width;
    max_y = TilemapRadiance_WorldHeight(example) - (float)example->context->config.virtual_height;
    if (max_x < 0.0f) max_x = 0.0f;
    if (max_y < 0.0f) max_y = 0.0f;

    if (example->camera.x < 0.0f) example->camera.x = 0.0f;
    if (example->camera.y < 0.0f) example->camera.y = 0.0f;
    if (example->camera.x > max_x) example->camera.x = max_x;
    if (example->camera.y > max_y) example->camera.y = max_y;
}

static void TilemapRadiance_ApplyPadding(RadianceTilemapExample *example)
{
    R2D_RadianceSetViewportPadding(
        example->radiance,
        example->padding_enabled ? TILEMAP_RADIANCE_PADDING : 0
    );
}

static Rectangle TilemapRadiance_CameraView(const RadianceTilemapExample *example)
{
    return (Rectangle) {
        example->camera.x,
        example->camera.y,
        (float)example->context->config.virtual_width,
        (float)example->context->config.virtual_height
    };
}

static Vector2 TilemapRadiance_MapPosition(const RadianceTilemapExample *example)
{
    return (Vector2) { -example->camera.x, -example->camera.y };
}

static Rectangle TilemapRadiance_PaddedCameraView(const RadianceTilemapExample *example)
{
    float padding = (float)example->radiance->viewport_padding;

    return (Rectangle) {
        example->camera.x - padding,
        example->camera.y - padding,
        (float)example->context->config.virtual_width + padding * 2.0f,
        (float)example->context->config.virtual_height + padding * 2.0f
    };
}

static Vector2 TilemapRadiance_PaddedMapPosition(const RadianceTilemapExample *example)
{
    float padding = (float)example->radiance->viewport_padding;

    return (Vector2) { -example->camera.x + padding, -example->camera.y + padding };
}

static bool TilemapRadiance_LoadTilemap(RadianceTilemapExample *example, const char *ok_status)
{
    R2D_Tilemap next_tilemap = { 0 };

    example->background_layer = -1;
    example->collision_layer = -1;
    example->lighting_layer = -1;

    if (!R2D_TilemapLoadTiledJson(&next_tilemap, R2D_AssetPath(TILEMAP_RADIANCE_PATH))) {
        TilemapRadiance_SetStatus(example, "Tilemap reload failed.");
        return false;
    }

    example->background_layer = R2D_TilemapLayerIndex(&next_tilemap, "Background");
    example->collision_layer = R2D_TilemapLayerIndex(&next_tilemap, "Collision");
    example->lighting_layer = R2D_TilemapLayerIndex(&next_tilemap, "Lighting");

    if (example->background_layer < 0 || example->collision_layer < 0 || example->lighting_layer < 0) {
        R2D_TilemapUnload(&next_tilemap);
        TilemapRadiance_SetStatus(example, "Tilemap missing Background/Collision/Lighting.");
        return false;
    }

    R2D_TilemapUnload(&example->tilemap);
    example->tilemap = next_tilemap;
    TilemapRadiance_ClampCamera(example);
    TilemapRadiance_SetStatus(example, ok_status);
    return true;
}

static void TilemapRadiance_InitWatches(RadianceTilemapExample *example)
{
    for (int i = 0; i < TILEMAP_RADIANCE_WATCH_COUNT; ++i) {
        example->watches[i] = TILEMAP_RADIANCE_WATCHES[i];
        R2D_FileWatchInit(&example->watches[i].watch);

        if (!R2D_FileWatchSet(&example->watches[i].watch, example->watches[i].path)) {
            TilemapRadiance_SetStatus(example, "Hot reload watch missing an asset.");
        }
    }
}

static void TilemapRadiance_ReloadPendingAssets(RadianceTilemapExample *example)
{
    bool ok = true;

    if (example->pending_tilemap_reload) {
        ok = TilemapRadiance_LoadTilemap(example, "Tilemap/textures reloaded.") && ok;
    }
    if (example->pending_radiance_reload) {
        ok = R2D_RadianceReload(example->radiance) && ok;
        TilemapRadiance_SetStatus(example, ok ? "Radiance shaders reloaded." : "Radiance shader reload failed.");
    }

    example->pending_tilemap_reload = false;
    example->pending_radiance_reload = false;
}

static void TilemapRadiance_QueueReload(RadianceTilemapExample *example, bool tilemap, bool radiance)
{
    example->pending_tilemap_reload = example->pending_tilemap_reload || tilemap;
    example->pending_radiance_reload = example->pending_radiance_reload || radiance;
    example->reload_delay = 0.20f;
}

static void TilemapRadiance_CheckHotReload(RadianceTilemapExample *example, float dt)
{
    if (example->status_timer > 0.0f) {
        example->status_timer -= dt;
    }

    if (example->hot_reload_enabled) {
        for (int i = 0; i < TILEMAP_RADIANCE_WATCH_COUNT; ++i) {
            TilemapRadianceWatch *watch = &example->watches[i];

            if (R2D_FileWatchCheck(&watch->watch)) {
                TilemapRadiance_QueueReload(example, watch->reload_tilemap, watch->reload_radiance);
            }
        }
    }

    if (example->reload_delay > 0.0f) {
        example->reload_delay -= dt;
        if (example->reload_delay <= 0.0f) {
            TilemapRadiance_ReloadPendingAssets(example);
        }
    }
}

static void TilemapRadiance_Init(void *user_data)
{
    RadianceTilemapExample *example = (RadianceTilemapExample *)user_data;

    TilemapRadiance_InitWatches(example);
    TilemapRadiance_LoadTilemap(example, "Project assets live. Edit and save.");
    R2D_RadianceSetQuality(example->radiance, 1, 16, 6);
    R2D_RadianceSetLight(example->radiance, 2.0f, 0.08f);
    R2D_RadianceSetFalloff(example->radiance, 1.15f);
    R2D_RadianceSetLightRange(example->radiance, 248.0f);
    TilemapRadiance_ApplyPadding(example);
}

static void TilemapRadiance_Update(float dt, void *user_data)
{
    RadianceTilemapExample *example = (RadianceTilemapExample *)user_data;
    Vector2 movement = { 0.0f, 0.0f };
    float speed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ? 220.0f : 120.0f;

    if (IsKeyDown(KEY_A)) movement.x -= 1.0f;
    if (IsKeyDown(KEY_D)) movement.x += 1.0f;
    if (IsKeyDown(KEY_W)) movement.y -= 1.0f;
    if (IsKeyDown(KEY_S)) movement.y += 1.0f;

    if (movement.x != 0.0f || movement.y != 0.0f) {
        float length = sqrtf(movement.x * movement.x + movement.y * movement.y);
        example->camera.x += (movement.x / length) * speed * dt;
        example->camera.y += (movement.y / length) * speed * dt;
    }

    if (IsKeyPressed(KEY_P)) {
        example->padding_enabled = !example->padding_enabled;
        TilemapRadiance_ApplyPadding(example);
    }
    if (IsKeyPressed(KEY_F5)) {
        TilemapRadiance_QueueReload(example, true, true);
    }
    if (IsKeyPressed(KEY_H)) {
        example->hot_reload_enabled = !example->hot_reload_enabled;
        TilemapRadiance_SetStatus(example, example->hot_reload_enabled ? "Hot reload enabled." : "Hot reload disabled.");
    }
    if (IsKeyPressed(KEY_V)) {
        example->debug = (R2D_RadianceDebugView)(((int)example->debug + 1) % 3);
    }
    if (IsKeyPressed(KEY_L)) {
        example->show_lighting_layer = !example->show_lighting_layer;
    }
    if (IsKeyPressed(KEY_G)) {
        example->show_collision_debug = !example->show_collision_debug;
    }
    if (IsKeyPressed(KEY_Y)) {
        example->sky = !example->sky;
    }
    if (IsKeyPressed(KEY_T) && example->crt != 0) {
        R2D_CrtSetEnabled(example->crt, !example->crt->enabled);
    }

    TilemapRadiance_ClampCamera(example);
    TilemapRadiance_CheckHotReload(example, dt);
    R2D_RadianceSetDebugView(example->radiance, example->debug);
    R2D_RadianceSetSky(example->radiance, example->sky, R2D_ColorFromHex(0x203a67ff));
}

static void TilemapRadiance_DrawScene(const RadianceTilemapExample *example)
{
    const Rectangle camera_view = TilemapRadiance_CameraView(example);
    const Vector2 map_position = TilemapRadiance_MapPosition(example);

    ClearBackground(R2D_ColorFromHex(0x141a26ff));

    if (!R2D_TilemapIsReady(&example->tilemap)) {
        DrawText("Failed to load " TILEMAP_RADIANCE_PATH, 8, 86, 8, R2D_ColorFromHex(0xff7474ff));
        return;
    }

    R2D_TilemapDrawLayerVisible(&example->tilemap, example->background_layer, camera_view, map_position);
    R2D_TilemapDrawLayerVisible(&example->tilemap, example->collision_layer, camera_view, map_position);
    if (example->show_lighting_layer) {
        R2D_TilemapDrawLayerVisible(&example->tilemap, example->lighting_layer, camera_view, map_position);
    }

    if (example->show_collision_debug) {
        R2D_TilemapDrawCollisionDebugVisible(
            &example->tilemap,
            example->collision_layer,
            camera_view,
            map_position,
            R2D_ColorFromHex(0xff4d8dcc)
        );
    }
}

static void TilemapRadiance_DrawHUD(const RadianceTilemapExample *example)
{
    char line[128];

    DrawText(
        "Tilemap Lighting layer renders into Radiance mask",
        4,
        4,
        4,
        R2D_ColorFromHex(0xf6f8ffff)
    );
    DrawText(
        "WASD move | P pad | V view | L layer | G coll | H hot | F5 reload",
        4,
        14,
        4,
        R2D_ColorFromHex(0xf6f8ffff)
    );

    snprintf(
        line,
        sizeof(line),
        "Y sky | T CRT | Cam %.0f %.0f | Pad:%s(%d) | Hot:%s | %s",
        example->camera.x,
        example->camera.y,
        example->padding_enabled ? "on" : "off",
        example->radiance->viewport_padding,
        example->hot_reload_enabled ? "on" : "off",
        TilemapRadiance_DebugName(example->debug)
    );
    DrawText(line, 4, 24, 4, R2D_ColorFromHex(0xf6f8ffff));

    if (example->status_timer > 0.0f && example->status[0] != '\0') {
        DrawText(example->status, 4, 34, 4, R2D_ColorFromHex(0x8ff0a4ff));
    }
}

static void TilemapRadiance_DrawMask(RadianceTilemapExample *example)
{
    Rectangle view = TilemapRadiance_PaddedCameraView(example);
    Vector2 map_position = TilemapRadiance_PaddedMapPosition(example);

    if (example->radiance == 0 || !example->radiance->is_ready) {
        return;
    }

    R2D_RadianceBeginMask(example->context, example->radiance);
    R2D_TilemapDrawLayerVisible(&example->tilemap, example->lighting_layer, view, map_position);
    R2D_RadianceEndMask(example->context, example->radiance);
}

static void TilemapRadiance_Draw(void *user_data)
{
    RadianceTilemapExample *example = (RadianceTilemapExample *)user_data;

    TilemapRadiance_DrawScene(example);
    TilemapRadiance_DrawMask(example);
    R2D_BeginOverlay(example->context);
    TilemapRadiance_DrawHUD(example);
    R2D_EndOverlay(example->context);
}

static void TilemapRadiance_Shutdown(void *user_data)
{
    RadianceTilemapExample *example = (RadianceTilemapExample *)user_data;
    R2D_TilemapUnload(&example->tilemap);
}

int main(int argc, char **argv)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    R2D_Radiance radiance = { 0 };
    R2D_Crt crt = { 0 };
    RadianceTilemapExample example = { 0 };
    bool use_project_assets = true;

    config.title = "Retro2D Radiance Tilemap";
    config.clear_color = BLACK;
    example.sky = true;
    example.padding_enabled = true;
    example.hot_reload_enabled = true;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-padding") == 0) {
            example.padding_enabled = false;
        } else if (strcmp(argv[i], "--no-sky") == 0) {
            example.sky = false;
        } else if (strcmp(argv[i], "--packaged-assets") == 0) {
            example.hot_reload_enabled = false;
            use_project_assets = false;
        } else if (strcmp(argv[i], "--debug") == 0 && i + 1 < argc) {
            ++i;
            if (strcmp(argv[i], "mask") == 0) {
                example.debug = R2D_RADIANCE_DEBUG_MASK;
            } else if (strcmp(argv[i], "cascade") == 0 || strcmp(argv[i], "gi") == 0) {
                example.debug = R2D_RADIANCE_DEBUG_CASCADE;
            }
        }
    }

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    if (use_project_assets) {
        TilemapRadiance_UseProjectAssets();
    }

    if (!R2D_RadianceInit(&radiance, config.virtual_width, config.virtual_height)) {
        R2D_Close(&context);
        return 1;
    }

    R2D_SetRadiance(&context, &radiance);
    R2D_CrtInit(&crt);
    R2D_CrtSetEnabled(&crt, false);
    R2D_SetCrt(&context, &crt);

    example.context = &context;
    example.radiance = &radiance;
    example.crt = &crt;

    R2D_Run(&context, (R2D_App) {
        TilemapRadiance_Init,
        TilemapRadiance_Update,
        TilemapRadiance_Draw,
        TilemapRadiance_Shutdown,
        &example
    });

    R2D_CrtClose(&crt);
    R2D_RadianceClose(&radiance);
    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
