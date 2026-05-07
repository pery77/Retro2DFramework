#ifndef R2D_H
#define R2D_H

#include "raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct R2D_Config {
    int virtual_width;
    int virtual_height;
    int window_scale;
    const char *title;
    Color clear_color;
} R2D_Config;

typedef struct R2D_App {
    void (*init)(void *user_data);
    void (*update)(float dt, void *user_data);
    void (*draw)(void *user_data);
    void (*shutdown)(void *user_data);
    void *user_data;
} R2D_App;

typedef struct R2D_Context {
    R2D_Config config;
    RenderTexture2D target;
    Rectangle source;
    Rectangle destination;
    Vector2 origin;
    bool is_ready;
} R2D_Context;

R2D_Config R2D_DefaultConfig(void);

bool R2D_Init(R2D_Context *ctx, R2D_Config config);
void R2D_Run(R2D_Context *ctx, R2D_App app);
void R2D_Close(R2D_Context *ctx);

void R2D_BeginFrame(R2D_Context *ctx);
void R2D_EndFrame(R2D_Context *ctx);

Vector2 R2D_MouseVirtualPosition(const R2D_Context *ctx);
Rectangle R2D_Rect(float x, float y, float width, float height);

void R2D_DrawSprite(Texture2D texture, Rectangle source, Vector2 position, bool flip_x);
Color R2D_ColorFromHex(unsigned int rgba);

#ifdef __cplusplus
}
#endif

#endif

