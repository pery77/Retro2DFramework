#ifndef R2D_H
#define R2D_H

#include "raylib.h"

#define R2D_DEFAULT_VIRTUAL_WIDTH 320
#define R2D_DEFAULT_VIRTUAL_HEIGHT 200
#define R2D_DEFAULT_WINDOW_SCALE 4

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

typedef struct R2D_Crt {
    Shader shader;
    Texture2D noise;
    int resolution_loc;
    int virtual_resolution_loc;
    int noise_loc;
    int random_loc;
    bool enabled;
    bool is_ready;
} R2D_Crt;

typedef struct R2D_Context {
    R2D_Config config;
    RenderTexture2D target;
    Rectangle source;
    Rectangle destination;
    Vector2 origin;
    R2D_Crt *crt;
    bool is_ready;
} R2D_Context;

R2D_Config R2D_DefaultConfig(void);

bool R2D_Init(R2D_Context *ctx, R2D_Config config);
void R2D_Run(R2D_Context *ctx, R2D_App app);
void R2D_Close(R2D_Context *ctx);

void R2D_BeginFrame(R2D_Context *ctx);
void R2D_EndFrame(R2D_Context *ctx);

bool R2D_CrtInit(R2D_Crt *crt);
bool R2D_CrtReload(R2D_Crt *crt);
void R2D_CrtClose(R2D_Crt *crt);
void R2D_CrtSetEnabled(R2D_Crt *crt, bool enabled);
void R2D_SetCrt(R2D_Context *ctx, R2D_Crt *crt);

const char *R2D_AssetPath(const char *relative_path);

int R2D_VirtualWidth(const R2D_Context *ctx);
int R2D_VirtualHeight(const R2D_Context *ctx);
Vector2 R2D_VirtualSize(const R2D_Context *ctx);
Vector2 R2D_MouseVirtualPosition(const R2D_Context *ctx);
Rectangle R2D_Rect(float x, float y, float width, float height);

void R2D_DrawSprite(Texture2D texture, Rectangle source, Vector2 position, bool flip_x);
Color R2D_ColorFromHex(unsigned int rgba);

#ifdef __cplusplus
}
#endif

#endif
