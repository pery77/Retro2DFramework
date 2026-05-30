#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define TILEMAP_RADIANCE_PATH "tilemaps/radiance_lighting.json"
#define TILEMAP_RADIANCE_PADDING 96

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
    R2D_RadianceDebugView debug;
} RadianceTilemapExample;

static const char *TilemapRadiance_DebugName(R2D_RadianceDebugView view)
{
    switch (view) {
    case R2D_RADIANCE_DEBUG_MASK: return "mask";
    case R2D_RADIANCE_DEBUG_CASCADE: return "cascade";
    default: return "final";
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

static void TilemapRadiance_Init(void *user_data)
{
    RadianceTilemapExample *example = (RadianceTilemapExample *)user_data;

    example->background_layer = -1;
    example->collision_layer = -1;
    example->lighting_layer = -1;
    R2D_TilemapLoadTiledJson(&example->tilemap, R2D_AssetPath(TILEMAP_RADIANCE_PATH));
    if (R2D_TilemapIsReady(&example->tilemap)) {
        example->background_layer = R2D_TilemapLayerIndex(&example->tilemap, "Background");
        example->collision_layer = R2D_TilemapLayerIndex(&example->tilemap, "Collision");
        example->lighting_layer = R2D_TilemapLayerIndex(&example->tilemap, "Lighting");
    }

    R2D_RadianceSetQuality(example->radiance, 1, 16, 6);
    R2D_RadianceSetLight(example->radiance, 2.0f, 0.08f);
    R2D_RadianceSetFalloff(example->radiance, 1.15f);
    R2D_RadianceSetLightRange(example->radiance, 248.0f);
    TilemapRadiance_ApplyPadding(example);
    TilemapRadiance_ClampCamera(example);
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
        "WASD move | P pad | V view | L layer | G coll",
        4,
        14,
        4,
        R2D_ColorFromHex(0xf6f8ffff)
    );

    snprintf(
        line,
        sizeof(line),
        "Y sky | T CRT | Cam %.0f %.0f | Pad:%s(%d) | L:%s | %s",
        example->camera.x,
        example->camera.y,
        example->padding_enabled ? "on" : "off",
        example->radiance->viewport_padding,
        example->show_lighting_layer ? "on" : "off",
        TilemapRadiance_DebugName(example->debug)
    );
    DrawText(line, 4, 24, 4, R2D_ColorFromHex(0xf6f8ffff));
}

static void TilemapRadiance_DrawMask(RadianceTilemapExample *example)
{
    Rectangle view = TilemapRadiance_PaddedCameraView(example);
    Vector2 map_position = TilemapRadiance_PaddedMapPosition(example);

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

    config.title = "Retro2D Radiance Tilemap";
    config.clear_color = BLACK;
    example.sky = true;
    example.padding_enabled = true;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-padding") == 0) {
            example.padding_enabled = false;
        } else if (strcmp(argv[i], "--no-sky") == 0) {
            example.sky = false;
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
