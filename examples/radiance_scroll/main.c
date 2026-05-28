#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define SCROLL_WORLD_WIDTH 960.0f
#define SCROLL_PADDING 96

typedef struct ScrollBlock {
    Rectangle rect;
    unsigned int edge;
} ScrollBlock;

typedef struct ScrollLight {
    Vector2 center;
    float radius;
    unsigned int color;
} ScrollLight;

typedef struct RadianceScrollExample {
    R2D_Context *context;
    R2D_Radiance *radiance;
    R2D_Crt *crt;
    float camera_x;
    float time;
    bool auto_scroll;
    bool padding_enabled;
    bool sky;
    R2D_RadianceDebugView debug;
} RadianceScrollExample;

static const ScrollBlock SCROLL_BLOCKS[] = {
    { { 74.0f, 56.0f, 28.0f, 92.0f }, 0x758098ff },
    { { 142.0f, 38.0f, 96.0f, 12.0f }, 0x758098ff },
    { { 228.0f, 96.0f, 44.0f, 66.0f }, 0x758098ff },
    { { 322.0f, 70.0f, 12.0f, 88.0f }, 0x8a93a8ff },
    { { 344.0f, 78.0f, 12.0f, 80.0f }, 0x8a93a8ff },
    { { 366.0f, 86.0f, 12.0f, 72.0f }, 0x8a93a8ff },
    { { 440.0f, 40.0f, 130.0f, 12.0f }, 0x758098ff },
    { { 516.0f, 78.0f, 34.0f, 86.0f }, 0x758098ff },
    { { 612.0f, 54.0f, 24.0f, 96.0f }, 0x758098ff },
    { { 668.0f, 116.0f, 118.0f, 12.0f }, 0x758098ff },
    { { 792.0f, 44.0f, 18.0f, 122.0f }, 0x758098ff },
    { { 834.0f, 70.0f, 10.0f, 24.0f }, 0x8a93a8ff },
    { { 852.0f, 78.0f, 10.0f, 24.0f }, 0x8a93a8ff },
    { { 870.0f, 86.0f, 10.0f, 24.0f }, 0x8a93a8ff }
};

static const ScrollLight SCROLL_LIGHTS[] = {
    { { 18.0f, 94.0f }, 11.0f, 0xff65baff },
    { { 154.0f, 156.0f }, 8.0f, 0xffb34eff },
    { { 300.0f, 46.0f }, 7.0f, 0x72d7ffff },
    { { 414.0f, 122.0f }, 10.0f, 0xb28cffff },
    { { 588.0f, 84.0f }, 9.0f, 0x6cff9fff },
    { { 744.0f, 152.0f }, 12.0f, 0x72d7ffff },
    { { 930.0f, 82.0f }, 12.0f, 0xffef7aff }
};

static float Scroll_MaxCameraX(void)
{
    return SCROLL_WORLD_WIDTH - (float)R2D_DEFAULT_VIRTUAL_WIDTH;
}

static Rectangle Scroll_ToScreenRect(Rectangle rect, float camera_x, float offset)
{
    rect.x = rect.x - camera_x + offset;
    rect.y += offset;
    return rect;
}

static Vector2 Scroll_ToScreenPoint(Vector2 point, float camera_x, float offset)
{
    return (Vector2) { point.x - camera_x + offset, point.y + offset };
}

static void Scroll_ApplyPadding(RadianceScrollExample *example)
{
    R2D_RadianceSetViewportPadding(example->radiance, example->padding_enabled ? SCROLL_PADDING : 0);
}

static void Scroll_Init(void *user_data)
{
    RadianceScrollExample *example = (RadianceScrollExample *)user_data;

    example->auto_scroll = true;
    example->sky = true;
    example->camera_x = 0.0f;

    R2D_RadianceSetQuality(example->radiance, 1, 16, 6);
    R2D_RadianceSetLight(example->radiance, 1.8f, 0.045f);
    R2D_RadianceSetFalloff(example->radiance, 1.15f);
    R2D_RadianceSetLightRange(example->radiance, 224.0f);
    Scroll_ApplyPadding(example);
}

static void Scroll_Update(float dt, void *user_data)
{
    RadianceScrollExample *example = (RadianceScrollExample *)user_data;
    float max_camera = Scroll_MaxCameraX();

    example->time += dt;

    if (IsKeyPressed(KEY_SPACE)) {
        example->auto_scroll = !example->auto_scroll;
    }
    if (IsKeyPressed(KEY_P)) {
        example->padding_enabled = !example->padding_enabled;
        Scroll_ApplyPadding(example);
    }
    if (IsKeyPressed(KEY_S)) {
        example->sky = !example->sky;
    }
    if (IsKeyPressed(KEY_D)) {
        example->debug = (R2D_RadianceDebugView)(((int)example->debug + 1) % 3);
    }
    if (IsKeyPressed(KEY_C) && example->crt != 0) {
        R2D_CrtSetEnabled(example->crt, !example->crt->enabled);
    }

    if (example->auto_scroll) {
        example->camera_x = (sinf(example->time * 0.28f) * 0.5f + 0.5f) * max_camera;
    }
    if (IsKeyDown(KEY_LEFT)) {
        example->auto_scroll = false;
        example->camera_x -= 120.0f * dt;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        example->auto_scroll = false;
        example->camera_x += 120.0f * dt;
    }

    if (example->camera_x < 0.0f) example->camera_x = 0.0f;
    if (example->camera_x > max_camera) example->camera_x = max_camera;

    R2D_RadianceSetDebugView(example->radiance, example->debug);
    R2D_RadianceSetSky(example->radiance, example->sky, R2D_ColorFromHex(0x244a7dff));
}

static void Scroll_DrawScene(const RadianceScrollExample *example)
{
    int block_count = (int)(sizeof(SCROLL_BLOCKS) / sizeof(SCROLL_BLOCKS[0]));
    int light_count = (int)(sizeof(SCROLL_LIGHTS) / sizeof(SCROLL_LIGHTS[0]));
    int start_x = ((int)example->camera_x / 32) * 32;

    ClearBackground(R2D_ColorFromHex(0x5f6878ff));
    DrawRectangle(0, 0, 320, 24, R2D_ColorFromHex(0x242c3aff));
    DrawRectangle(0, 164, 320, 36, R2D_ColorFromHex(0x303949ff));
    DrawRectangle(0, 184, 320, 16, R2D_ColorFromHex(0x202837ff));

    for (int x = start_x; x < start_x + 384; x += 32) {
        int sx = (int)((float)x - example->camera_x);
        DrawRectangle(sx, 24, 2, 140, R2D_ColorFromHex(0x535c6cff));
    }
    for (int y = 42; y < 164; y += 32) {
        DrawRectangle(0, y, 320, 2, R2D_ColorFromHex(0x535c6cff));
    }

    for (int i = 0; i < light_count; ++i) {
        Vector2 p = Scroll_ToScreenPoint(SCROLL_LIGHTS[i].center, example->camera_x, 0.0f);
        DrawCircleV(p, SCROLL_LIGHTS[i].radius + 2.0f, R2D_ColorFromHex(0x10131aff));
        DrawCircleV(p, SCROLL_LIGHTS[i].radius, R2D_ColorFromHex(SCROLL_LIGHTS[i].color));
    }

    for (int i = 0; i < block_count; ++i) {
        Rectangle rect = Scroll_ToScreenRect(SCROLL_BLOCKS[i].rect, example->camera_x, 0.0f);
        DrawRectangleRec(rect, R2D_ColorFromHex(0x0d1016ff));
        DrawRectangleLinesEx(rect, 1.0f, R2D_ColorFromHex(SCROLL_BLOCKS[i].edge));
    }
}

static void Scroll_DrawHUD(const RadianceScrollExample *example)
{
    char line[128];

    snprintf(
        line,
        sizeof(line),
        "Pad:%s(%d) Cam:%.0f Auto:%s Sky:%s V:%d",
        example->padding_enabled ? "on" : "off",
        example->radiance->viewport_padding,
        example->camera_x,
        example->auto_scroll ? "on" : "off",
        example->sky ? "on" : "off",
        (int)example->debug
    );
    DrawText(line, 4, 4, 4, R2D_ColorFromHex(0xe6edf3ff));
    DrawText("P pad | Arrows cam | Space auto | D view | S sky | C CRT", 4, 14, 4, R2D_ColorFromHex(0xe6edf3ff));
}

static void Scroll_DrawMask(RadianceScrollExample *example)
{
    int block_count = (int)(sizeof(SCROLL_BLOCKS) / sizeof(SCROLL_BLOCKS[0]));
    int light_count = (int)(sizeof(SCROLL_LIGHTS) / sizeof(SCROLL_LIGHTS[0]));
    float offset = (float)example->radiance->viewport_padding;

    R2D_RadianceBeginMask(example->context, example->radiance);

    for (int i = 0; i < block_count; ++i) {
        R2D_RadianceDrawOccluderRect(Scroll_ToScreenRect(SCROLL_BLOCKS[i].rect, example->camera_x, offset));
    }
    for (int i = 0; i < light_count; ++i) {
        R2D_RadianceDrawEmitterCircle(
            Scroll_ToScreenPoint(SCROLL_LIGHTS[i].center, example->camera_x, offset),
            SCROLL_LIGHTS[i].radius,
            R2D_ColorFromHex(SCROLL_LIGHTS[i].color)
        );
    }

    R2D_RadianceEndMask(example->context, example->radiance);
}

static void Scroll_Draw(void *user_data)
{
    RadianceScrollExample *example = (RadianceScrollExample *)user_data;

    Scroll_DrawScene(example);
    Scroll_DrawMask(example);
    R2D_BeginOverlay(example->context);
    Scroll_DrawHUD(example);
    R2D_EndOverlay(example->context);
}

static void Scroll_Shutdown(void *user_data)
{
    (void)user_data;
}

int main(int argc, char **argv)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    R2D_Radiance radiance = { 0 };
    R2D_Crt crt = { 0 };
    RadianceScrollExample example = { 0 };

    config.title = "Retro2D Radiance Scroll Padding";
    config.clear_color = BLACK;
    example.padding_enabled = true;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-padding") == 0) {
            example.padding_enabled = false;
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
        Scroll_Init,
        Scroll_Update,
        Scroll_Draw,
        Scroll_Shutdown,
        &example
    });

    R2D_CrtClose(&crt);
    R2D_RadianceClose(&radiance);
    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
