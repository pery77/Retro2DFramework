#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

static Rectangle R2D_CalculateDestination(int virtual_width, int virtual_height)
{
    const float screen_width = (float)GetScreenWidth();
    const float screen_height = (float)GetScreenHeight();
    const float scale_x = floorf(screen_width / (float)virtual_width);
    const float scale_y = floorf(screen_height / (float)virtual_height);
    float scale = fminf(scale_x, scale_y);

    if (scale < 1.0f) {
        scale = 1.0f;
    }

    const float width = (float)virtual_width * scale;
    const float height = (float)virtual_height * scale;

    return (Rectangle) {
        floorf((screen_width - width) * 0.5f),
        floorf((screen_height - height) * 0.5f),
        width,
        height
    };
}

static void R2D_HandleWindowShortcuts(R2D_Context *ctx)
{
    const bool alt_enter = IsKeyPressed(KEY_ENTER) &&
        (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));

    if (IsKeyPressed(KEY_F11) || alt_enter) {
        R2D_ToggleFullscreen(ctx);
    }

    if (ctx != 0 && (IsKeyPressed(KEY_F12) || IsKeyPressed(KEY_PRINT_SCREEN))) {
        ctx->screenshot_requested = true;
    }
}

R2D_Config R2D_DefaultConfig(void)
{
    return (R2D_Config) {
        R2D_DEFAULT_VIRTUAL_WIDTH,
        R2D_DEFAULT_VIRTUAL_HEIGHT,
        R2D_DEFAULT_WINDOW_SCALE,
        "Retro2DFramework",
        BLACK
    };
}

bool R2D_Init(R2D_Context *ctx, R2D_Config config)
{
    if (ctx == 0) {
        return false;
    }

    if (config.virtual_width <= 0 || config.virtual_height <= 0) {
        config.virtual_width = R2D_DEFAULT_VIRTUAL_WIDTH;
        config.virtual_height = R2D_DEFAULT_VIRTUAL_HEIGHT;
    }

    if (config.window_scale <= 0) {
        config.window_scale = R2D_DEFAULT_WINDOW_SCALE;
    }

    if (config.title == 0) {
        config.title = "Retro2DFramework";
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(
        config.virtual_width * config.window_scale,
        config.virtual_height * config.window_scale,
        config.title
    );

    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);

    ctx->config = config;
    ctx->target = LoadRenderTexture(config.virtual_width, config.virtual_height);
    ctx->source = (Rectangle) {
        0.0f,
        0.0f,
        (float)config.virtual_width,
        -(float)config.virtual_height
    };
    ctx->destination = R2D_CalculateDestination(config.virtual_width, config.virtual_height);
    ctx->origin = (Vector2) { 0.0f, 0.0f };
    ctx->windowed_width = config.virtual_width * config.window_scale;
    ctx->windowed_height = config.virtual_height * config.window_scale;
    ctx->windowed_position = GetWindowPosition();
    ctx->crt = 0;
    ctx->screenshot_requested = false;
    ctx->is_ready = IsRenderTextureValid(ctx->target);

    SetTextureFilter(ctx->target.texture, TEXTURE_FILTER_POINT);
    SetTextureWrap(ctx->target.texture, TEXTURE_WRAP_MIRROR_REPEAT);
    return ctx->is_ready;
}

void R2D_Run(R2D_Context *ctx, R2D_App app)
{
    if (ctx == 0 || !ctx->is_ready) {
        return;
    }

    if (app.init != 0) {
        app.init(app.user_data);
    }

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();

        R2D_HandleWindowShortcuts(ctx);

        if (app.update != 0) {
            app.update(dt, app.user_data);
        }

        R2D_BeginFrame(ctx);

        if (app.draw != 0) {
            app.draw(app.user_data);
        }

        R2D_EndFrame(ctx);
    }

    if (app.shutdown != 0) {
        app.shutdown(app.user_data);
    }
}

void R2D_Close(R2D_Context *ctx)
{
    if (ctx == 0 || !ctx->is_ready) {
        return;
    }

    UnloadRenderTexture(ctx->target);
    CloseWindow();
    ctx->is_ready = false;
}

void R2D_BeginFrame(R2D_Context *ctx)
{
    BeginTextureMode(ctx->target);
    ClearBackground(ctx->config.clear_color);
}

void R2D_EndFrame(R2D_Context *ctx)
{
    EndTextureMode();

    ctx->destination = R2D_CalculateDestination(
        ctx->config.virtual_width,
        ctx->config.virtual_height
    );

    BeginDrawing();
    ClearBackground(BLACK);

    if (ctx->crt != 0 && ctx->crt->enabled && ctx->crt->is_ready) {
        const Vector2 resolution = { ctx->destination.width, ctx->destination.height };
        const Vector2 virtual_resolution = R2D_VirtualSize(ctx);
        const float random = (float)GetRandomValue(1, 100) + (float)GetRandomValue(1, 99) / 100.0f;

        BeginShaderMode(ctx->crt->shader);
        SetShaderValue(ctx->crt->shader, ctx->crt->resolution_loc, &resolution, SHADER_UNIFORM_VEC2);
        SetShaderValue(ctx->crt->shader, ctx->crt->virtual_resolution_loc, &virtual_resolution, SHADER_UNIFORM_VEC2);
        SetShaderValue(ctx->crt->shader, ctx->crt->random_loc, &random, SHADER_UNIFORM_FLOAT);
        SetShaderValueTexture(ctx->crt->shader, ctx->crt->noise_loc, ctx->crt->noise);
        DrawTexturePro(ctx->target.texture, ctx->source, ctx->destination, ctx->origin, 0.0f, WHITE);
        EndShaderMode();
    } else {
        DrawTexturePro(ctx->target.texture, ctx->source, ctx->destination, ctx->origin, 0.0f, WHITE);
    }

    EndDrawing();

    if (ctx->screenshot_requested) {
        R2D_TakeScreenshot();
        ctx->screenshot_requested = false;
    }
}

void R2D_ToggleFullscreen(R2D_Context *ctx)
{
    if (ctx == 0 || !ctx->is_ready) {
        return;
    }

    if (!IsWindowFullscreen()) {
        const Vector2 position = GetWindowPosition();

        ctx->windowed_width = GetScreenWidth();
        ctx->windowed_height = GetScreenHeight();
        ctx->windowed_position = position;

        const int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    } else {
        ToggleFullscreen();
        SetWindowSize(ctx->windowed_width, ctx->windowed_height);
        SetWindowPosition((int)ctx->windowed_position.x, (int)ctx->windowed_position.y);
    }
}

void R2D_TakeScreenshot(void)
{
    char directory[1024];
    char filename[1200];
    time_t now;
    struct tm local_time;
    bool has_time = false;

    snprintf(directory, sizeof(directory), "%sscreenshots", GetApplicationDirectory());

    if (!DirectoryExists(directory)) {
        MakeDirectory(directory);
    }

    now = time(0);
#if defined(_WIN32)
    has_time = localtime_s(&local_time, &now) == 0;
#else
    {
        struct tm *time_info = localtime(&now);
        if (time_info != 0) {
            local_time = *time_info;
            has_time = true;
        }
    }
#endif

    if (!has_time) {
        snprintf(filename, sizeof(filename), "%s/screenshot.png", directory);
    } else {
        snprintf(
            filename,
            sizeof(filename),
            "%s/screenshot_%04d%02d%02d_%02d%02d%02d.png",
            directory,
            local_time.tm_year + 1900,
            local_time.tm_mon + 1,
            local_time.tm_mday,
            local_time.tm_hour,
            local_time.tm_min,
            local_time.tm_sec
        );
    }

    Image screenshot = LoadImageFromScreen();

    if (ExportImage(screenshot, filename)) {
        TraceLog(LOG_INFO, "R2D: Screenshot saved: %s", filename);
    } else {
        TraceLog(LOG_WARNING, "R2D: Screenshot could not be saved: %s", filename);
    }

    UnloadImage(screenshot);
}

const char *R2D_AssetPath(const char *relative_path)
{
    static char path[1024];

    if (relative_path == 0) {
        relative_path = "";
    }

#ifdef R2D_DEVELOPMENT_ASSET_DIR
    snprintf(path, sizeof(path), "%s%s", R2D_DEVELOPMENT_ASSET_DIR, relative_path);
    if (FileExists(path) || DirectoryExists(path)) {
        return path;
    }
#endif

    snprintf(path, sizeof(path), "%sassets/%s", GetApplicationDirectory(), relative_path);
    return path;
}

int R2D_VirtualWidth(const R2D_Context *ctx)
{
    return ctx != 0 ? ctx->config.virtual_width : R2D_DEFAULT_VIRTUAL_WIDTH;
}

int R2D_VirtualHeight(const R2D_Context *ctx)
{
    return ctx != 0 ? ctx->config.virtual_height : R2D_DEFAULT_VIRTUAL_HEIGHT;
}

Vector2 R2D_VirtualSize(const R2D_Context *ctx)
{
    return (Vector2) {
        (float)R2D_VirtualWidth(ctx),
        (float)R2D_VirtualHeight(ctx)
    };
}

Vector2 R2D_MouseVirtualPosition(const R2D_Context *ctx)
{
    const Vector2 mouse = GetMousePosition();
    const float scale = ctx->destination.width / (float)ctx->config.virtual_width;

    return (Vector2) {
        (mouse.x - ctx->destination.x) / scale,
        (mouse.y - ctx->destination.y) / scale
    };
}

Rectangle R2D_Rect(float x, float y, float width, float height)
{
    return (Rectangle) { x, y, width, height };
}

Color R2D_ColorFromHex(unsigned int rgba)
{
    return (Color) {
        (unsigned char)((rgba >> 24) & 0xff),
        (unsigned char)((rgba >> 16) & 0xff),
        (unsigned char)((rgba >> 8) & 0xff),
        (unsigned char)(rgba & 0xff)
    };
}
