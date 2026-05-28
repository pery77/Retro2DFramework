#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define PAINT_WORLD_SCALE 2
#define PAINT_VIEW_PADDING 96

typedef struct PaintColor {
    const char *name;
    unsigned int rgba;
} PaintColor;

typedef struct RadiancePaintExample {
    R2D_Context *context;
    R2D_Radiance *radiance;
    R2D_Crt *crt;
    Image mask_image;
    Image visual_image;
    Texture2D mask_texture;
    Texture2D visual_texture;
    Vector2 camera;
    int selected_color;
    float brush_radius;
    bool sky;
    bool padding_enabled;
    bool seed_on_init;
    R2D_RadianceDebugView debug;
} RadiancePaintExample;

static const PaintColor PALETTE[] = {
    { "shadow", 0x000000ff },
    { "cyan",   0x72d7ffff },
    { "pink",   0xff65baff },
    { "amber",  0xffb34eff },
    { "violet", 0xb28cffff },
    { "green",  0x6cff9fff }
};

static Rectangle Paint_TextureSource(Texture2D texture)
{
    return (Rectangle) { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
}

static Rectangle Paint_WorldDest(const RadiancePaintExample *example, float offset)
{
    return (Rectangle) {
        -example->camera.x + offset,
        -example->camera.y + offset,
        (float)example->visual_texture.width,
        (float)example->visual_texture.height
    };
}

static bool Paint_TextureReady(const RadiancePaintExample *example)
{
    return example != 0 &&
        IsImageValid(example->mask_image) &&
        IsImageValid(example->visual_image) &&
        IsTextureValid(example->mask_texture) &&
        IsTextureValid(example->visual_texture);
}

static void Paint_UpdateTextures(RadiancePaintExample *example)
{
    if (!Paint_TextureReady(example)) {
        return;
    }

    UpdateTexture(example->mask_texture, example->mask_image.data);
    UpdateTexture(example->visual_texture, example->visual_image.data);
}

static void Paint_ClearCanvas(RadiancePaintExample *example)
{
    Color *mask_pixels;
    Color *visual_pixels;
    int count;

    if (!Paint_TextureReady(example)) {
        return;
    }

    mask_pixels = (Color *)example->mask_image.data;
    visual_pixels = (Color *)example->visual_image.data;
    count = example->mask_image.width * example->mask_image.height;

    for (int i = 0; i < count; ++i) {
        mask_pixels[i] = WHITE;
        visual_pixels[i] = BLANK;
    }

    Paint_UpdateTextures(example);
}

static void Paint_Stamp(RadiancePaintExample *example, Vector2 center, float radius, Color color)
{
    Color *mask_pixels;
    Color *visual_pixels;
    int width;
    int height;
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    float radius_sq;
    bool erase;

    if (!Paint_TextureReady(example)) {
        return;
    }

    width = example->mask_image.width;
    height = example->mask_image.height;
    min_x = (int)floorf(center.x - radius);
    max_x = (int)ceilf(center.x + radius);
    min_y = (int)floorf(center.y - radius);
    max_y = (int)ceilf(center.y + radius);
    radius_sq = radius * radius;
    erase = color.r == 255 && color.g == 255 && color.b == 255;
    mask_pixels = (Color *)example->mask_image.data;
    visual_pixels = (Color *)example->visual_image.data;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= width) max_x = width - 1;
    if (max_y >= height) max_y = height - 1;

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            float dx = ((float)x + 0.5f) - center.x;
            float dy = ((float)y + 0.5f) - center.y;

            if (dx * dx + dy * dy <= radius_sq) {
                int index = y * width + x;
                mask_pixels[index] = color;
                visual_pixels[index] = erase ? BLANK : color;
            }
        }
    }
}

static void Paint_ResetDemo(RadiancePaintExample *example)
{
    Paint_ClearCanvas(example);

    Paint_Stamp(example, (Vector2) { 58.0f, 68.0f }, 8.0f, R2D_ColorFromHex(0xff65baff));
    Paint_Stamp(example, (Vector2) { 246.0f, 52.0f }, 7.0f, R2D_ColorFromHex(0x72d7ffff));
    Paint_Stamp(example, (Vector2) { 102.0f, 156.0f }, 9.0f, R2D_ColorFromHex(0xffb34eff));
    Paint_Stamp(example, (Vector2) { 290.0f, 144.0f }, 10.0f, R2D_ColorFromHex(0x6cff9fff));
    Paint_Stamp(example, (Vector2) { 420.0f, 74.0f }, 8.0f, R2D_ColorFromHex(0xb28cffff));
    Paint_Stamp(example, (Vector2) { 566.0f, 108.0f }, 11.0f, R2D_ColorFromHex(0xffef7aff));
    Paint_Stamp(example, (Vector2) { 388.0f, 304.0f }, 9.0f, R2D_ColorFromHex(0x72d7ffff));
    Paint_Stamp(example, (Vector2) { 568.0f, 330.0f }, 11.0f, R2D_ColorFromHex(0xffb34eff));

    Paint_Stamp(example, (Vector2) { 154.0f, 96.0f }, 18.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 184.0f, 96.0f }, 18.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 214.0f, 96.0f }, 18.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 158.0f, 132.0f }, 6.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 178.0f, 132.0f }, 6.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 198.0f, 132.0f }, 6.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 418.0f, 210.0f }, 22.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 456.0f, 210.0f }, 22.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 494.0f, 210.0f }, 22.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 392.0f, 252.0f }, 7.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 424.0f, 252.0f }, 7.0f, BLACK);
    Paint_Stamp(example, (Vector2) { 456.0f, 252.0f }, 7.0f, BLACK);

    Paint_UpdateTextures(example);
}

static void Paint_CreateCanvas(RadiancePaintExample *example)
{
    int width = example->context->config.virtual_width * PAINT_WORLD_SCALE;
    int height = example->context->config.virtual_height * PAINT_WORLD_SCALE;

    example->mask_image = GenImageColor(width, height, WHITE);
    example->visual_image = GenImageColor(width, height, BLANK);
    example->mask_texture = LoadTextureFromImage(example->mask_image);
    example->visual_texture = LoadTextureFromImage(example->visual_image);

    if (IsTextureValid(example->mask_texture)) {
        SetTextureFilter(example->mask_texture, TEXTURE_FILTER_POINT);
        SetTextureWrap(example->mask_texture, TEXTURE_WRAP_CLAMP);
    }
    if (IsTextureValid(example->visual_texture)) {
        SetTextureFilter(example->visual_texture, TEXTURE_FILTER_POINT);
        SetTextureWrap(example->visual_texture, TEXTURE_WRAP_CLAMP);
    }
}

static void Paint_CloseCanvas(RadiancePaintExample *example)
{
    if (IsTextureValid(example->mask_texture)) {
        UnloadTexture(example->mask_texture);
        example->mask_texture = (Texture2D) { 0 };
    }
    if (IsTextureValid(example->visual_texture)) {
        UnloadTexture(example->visual_texture);
        example->visual_texture = (Texture2D) { 0 };
    }
    if (IsImageValid(example->mask_image)) {
        UnloadImage(example->mask_image);
        example->mask_image = (Image) { 0 };
    }
    if (IsImageValid(example->visual_image)) {
        UnloadImage(example->visual_image);
        example->visual_image = (Image) { 0 };
    }
}

static bool Paint_PointInViewport(const RadiancePaintExample *example, Vector2 point)
{
    return point.x >= 0.0f &&
        point.y >= 0.0f &&
        point.x < (float)example->context->config.virtual_width &&
        point.y < (float)example->context->config.virtual_height;
}

static bool Paint_PointInWorld(const RadiancePaintExample *example, Vector2 point)
{
    return point.x >= 0.0f &&
        point.y >= 0.0f &&
        point.x < (float)example->mask_image.width &&
        point.y < (float)example->mask_image.height;
}

static Vector2 Paint_MouseWorldPosition(const RadiancePaintExample *example)
{
    Vector2 mouse = R2D_MouseVirtualPosition(example->context);
    return (Vector2) { mouse.x + example->camera.x, mouse.y + example->camera.y };
}

static void Paint_ClampCamera(RadiancePaintExample *example)
{
    float max_x = (float)example->mask_image.width - (float)example->context->config.virtual_width;
    float max_y = (float)example->mask_image.height - (float)example->context->config.virtual_height;

    if (example->camera.x < 0.0f) example->camera.x = 0.0f;
    if (example->camera.y < 0.0f) example->camera.y = 0.0f;
    if (example->camera.x > max_x) example->camera.x = max_x;
    if (example->camera.y > max_y) example->camera.y = max_y;
}

static void Paint_ApplyPadding(RadiancePaintExample *example)
{
    R2D_RadianceSetViewportPadding(example->radiance, example->padding_enabled ? PAINT_VIEW_PADDING : 0);
}

static void Paint_Init(void *user_data)
{
    RadiancePaintExample *example = (RadiancePaintExample *)user_data;

    Paint_CreateCanvas(example);
    example->brush_radius = 7.0f;
    example->selected_color = 0;

    R2D_RadianceSetQuality(example->radiance, 1, 16, 6);
    R2D_RadianceSetLight(example->radiance, 1.8f, 0.045f);
    R2D_RadianceSetFalloff(example->radiance, 1.15f);
    R2D_RadianceSetLightRange(example->radiance, 224.0f);
    Paint_ApplyPadding(example);

    if (example->seed_on_init) {
        Paint_ResetDemo(example);
    } else {
        Paint_ClearCanvas(example);
    }
}

static void Paint_Update(float dt, void *user_data)
{
    RadiancePaintExample *example = (RadiancePaintExample *)user_data;
    Vector2 mouse = R2D_MouseVirtualPosition(example->context);
    Vector2 world_mouse;
    float wheel = GetMouseWheelMove();
    float camera_speed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ? 180.0f : 96.0f;
    bool dirty = false;

    if (IsKeyPressed(KEY_ONE)) example->selected_color = 0;
    if (IsKeyPressed(KEY_TWO)) example->selected_color = 1;
    if (IsKeyPressed(KEY_THREE)) example->selected_color = 2;
    if (IsKeyPressed(KEY_FOUR)) example->selected_color = 3;
    if (IsKeyPressed(KEY_FIVE)) example->selected_color = 4;
    if (IsKeyPressed(KEY_SIX)) example->selected_color = 5;

    if (IsKeyDown(KEY_A)) example->camera.x -= camera_speed * dt;
    if (IsKeyDown(KEY_D)) example->camera.x += camera_speed * dt;
    if (IsKeyDown(KEY_W)) example->camera.y -= camera_speed * dt;
    if (IsKeyDown(KEY_S)) example->camera.y += camera_speed * dt;
    Paint_ClampCamera(example);

    if (IsKeyPressed(KEY_V)) {
        example->debug = (R2D_RadianceDebugView)(((int)example->debug + 1) % 3);
    }
    if (IsKeyPressed(KEY_Y)) {
        example->sky = !example->sky;
    }
    if (IsKeyPressed(KEY_P)) {
        example->padding_enabled = !example->padding_enabled;
        Paint_ApplyPadding(example);
    }
    if (IsKeyPressed(KEY_T) && example->crt != 0) {
        R2D_CrtSetEnabled(example->crt, !example->crt->enabled);
    }
    if (IsKeyPressed(KEY_C)) {
        Paint_ClearCanvas(example);
    }
    if (IsKeyPressed(KEY_R)) {
        Paint_ResetDemo(example);
    }
    if (IsKeyDown(KEY_Q)) {
        example->brush_radius = fmaxf(1.0f, example->brush_radius - 18.0f * GetFrameTime());
    }
    if (IsKeyDown(KEY_E)) {
        example->brush_radius = fminf(28.0f, example->brush_radius + 18.0f * GetFrameTime());
    }
    if (wheel != 0.0f) {
        example->brush_radius = fmaxf(1.0f, fminf(28.0f, example->brush_radius + wheel));
    }

    world_mouse = Paint_MouseWorldPosition(example);
    if (Paint_PointInViewport(example, mouse) && Paint_PointInWorld(example, world_mouse)) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Paint_Stamp(example, world_mouse, example->brush_radius, R2D_ColorFromHex(PALETTE[example->selected_color].rgba));
            dirty = true;
        } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Paint_Stamp(example, world_mouse, example->brush_radius, WHITE);
            dirty = true;
        }
    }

    if (dirty) {
        Paint_UpdateTextures(example);
    }

    R2D_RadianceSetDebugView(example->radiance, example->debug);
    R2D_RadianceSetSky(example->radiance, example->sky, R2D_ColorFromHex(0x244a7dff));
}

static void Paint_DrawScene(const RadiancePaintExample *example)
{
    Rectangle source = Paint_TextureSource(example->visual_texture);
    Rectangle dest = Paint_WorldDest(example, 0.0f);
    int start_x = ((int)floorf(example->camera.x / 32.0f)) * 32;
    int start_y = ((int)floorf(example->camera.y / 32.0f)) * 32;
    int end_x = (int)(example->camera.x + (float)example->context->config.virtual_width) + 32;
    int end_y = (int)(example->camera.y + (float)example->context->config.virtual_height) + 32;
    float world_width = (float)example->visual_texture.width;
    float world_height = (float)example->visual_texture.height;

    ClearBackground(R2D_ColorFromHex(0x657080ff));
    DrawRectangle(
        (int)-example->camera.x,
        (int)(world_height - 34.0f - example->camera.y),
        (int)world_width,
        34,
        R2D_ColorFromHex(0x303949ff)
    );
    DrawRectangle(
        (int)-example->camera.x,
        (int)(world_height - 16.0f - example->camera.y),
        (int)world_width,
        16,
        R2D_ColorFromHex(0x202837ff)
    );

    for (int x = start_x; x <= end_x; x += 32) {
        DrawRectangle((int)((float)x - example->camera.x), (int)-example->camera.y, 2, (int)world_height, R2D_ColorFromHex(0x515b6cff));
    }
    for (int y = start_y; y <= end_y; y += 32) {
        DrawRectangle((int)-example->camera.x, (int)((float)y - example->camera.y), (int)world_width, 2, R2D_ColorFromHex(0x515b6cff));
    }

    DrawRectangleLinesEx(
        (Rectangle) { -example->camera.x, -example->camera.y, world_width, world_height },
        1.0f,
        R2D_ColorFromHex(0x9aa4b8ff)
    );

    DrawTexturePro(example->visual_texture, source, dest, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
}

static void Paint_DrawHUD(const RadiancePaintExample *example)
{
    char line[128];
    Vector2 mouse = R2D_MouseVirtualPosition(example->context);
    int palette_count = (int)(sizeof(PALETTE) / sizeof(PALETTE[0]));

    snprintf(
        line,
        sizeof(line),
        "LMB paint | RMB erase | WASD move | 1-6 color | Wheel/QE size"
    );
    DrawText(line, 4, 4, 4, R2D_ColorFromHex(0xe6edf3ff));

    snprintf(
        line,
        sizeof(line),
        "C clear | R reset | V view | Y sky | P pad:%s | T CRT | %s %.0fpx",
        example->padding_enabled ? "on" : "off",
        PALETTE[example->selected_color].name,
        example->brush_radius
    );
    DrawText(line, 4, 15, 4, R2D_ColorFromHex(0xe6edf3ff));

    for (int i = 0; i < palette_count; ++i) {
        Rectangle swatch = { 5.0f + (float)i * 15.0f, 188.0f, 11.0f, 8.0f };
        Color color = R2D_ColorFromHex(PALETTE[i].rgba);
        DrawRectangleRec((Rectangle) { swatch.x - 1.0f, swatch.y - 1.0f, swatch.width + 2.0f, swatch.height + 2.0f },
            i == example->selected_color ? R2D_ColorFromHex(0xffffffff) : R2D_ColorFromHex(0x202633ff));
        DrawRectangleRec(swatch, color);
    }

    snprintf(
        line,
        sizeof(line),
        "Cam %.0f %.0f / World %dx%d / V%d",
        example->camera.x,
        example->camera.y,
        example->mask_image.width,
        example->mask_image.height,
        (int)example->debug
    );
    DrawText(line, 4, 26, 4, R2D_ColorFromHex(0xe6edf3ff));

    if (Paint_PointInViewport(example, mouse)) {
        Color color = R2D_ColorFromHex(PALETTE[example->selected_color].rgba);
        DrawCircleLines((int)mouse.x, (int)mouse.y, example->brush_radius, color.r == 0 && color.g == 0 && color.b == 0 ? WHITE : color);
    }
}

static void Paint_DrawMask(RadiancePaintExample *example)
{
    Rectangle source = Paint_TextureSource(example->mask_texture);
    Rectangle dest = Paint_WorldDest(example, (float)example->radiance->viewport_padding);

    R2D_RadianceBeginMask(example->context, example->radiance);
    DrawTexturePro(example->mask_texture, source, dest, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
    R2D_RadianceEndMask(example->context, example->radiance);
}

static void Paint_Draw(void *user_data)
{
    RadiancePaintExample *example = (RadiancePaintExample *)user_data;

    Paint_DrawScene(example);
    Paint_DrawMask(example);
    R2D_BeginOverlay(example->context);
    Paint_DrawHUD(example);
    R2D_EndOverlay(example->context);
}

static void Paint_Shutdown(void *user_data)
{
    RadiancePaintExample *example = (RadiancePaintExample *)user_data;
    Paint_CloseCanvas(example);
}

int main(int argc, char **argv)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    R2D_Radiance radiance = { 0 };
    R2D_Crt crt = { 0 };
    RadiancePaintExample example = { 0 };

    config.title = "Retro2D Radiance Paint";
    config.clear_color = BLACK;
    example.sky = true;
    example.padding_enabled = true;
    example.seed_on_init = true;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--blank") == 0) {
            example.seed_on_init = false;
        } else if (strcmp(argv[i], "--no-sky") == 0) {
            example.sky = false;
        } else if (strcmp(argv[i], "--no-padding") == 0) {
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
        Paint_Init,
        Paint_Update,
        Paint_Draw,
        Paint_Shutdown,
        &example
    });

    R2D_CrtClose(&crt);
    R2D_RadianceClose(&radiance);
    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
