#include "r2d/r2d.h"

static bool R2D_CrtLoadShader(R2D_Crt *crt)
{
    const char *shader_path = R2D_AssetPath("shaders/crt.fs");
    Shader shader = R2D_LoadFragmentShader(shader_path);

    if (!IsShaderValid(shader)) {
        TraceLog(LOG_WARNING, "R2D: Failed to load CRT shader: %s", shader_path);
        return false;
    }

    if (IsShaderValid(crt->shader)) {
        UnloadShader(crt->shader);
    }

    crt->shader = shader;
    crt->resolution_loc = GetShaderLocation(crt->shader, "resolution");
    crt->virtual_resolution_loc = GetShaderLocation(crt->shader, "virtualResolution");
    crt->noise_loc = GetShaderLocation(crt->shader, "noiseTexture");
    crt->random_loc = GetShaderLocation(crt->shader, "rnd");
    crt->is_ready = IsTextureValid(crt->noise);

    TraceLog(
        LOG_INFO,
        "R2D: CRT shader loaded: %s noiseLoc=%d virtualResolutionLoc=%d textureId=%u",
        shader_path,
        crt->noise_loc,
        crt->virtual_resolution_loc,
        crt->noise.id
    );

    return crt->is_ready;
}

bool R2D_CrtInit(R2D_Crt *crt)
{
    if (crt == 0) {
        return false;
    }

    const char *noise_path = R2D_AssetPath("textures/noise.png");
    crt->noise = R2D_LoadTexture(noise_path);

    if (!IsTextureValid(crt->noise)) {
        TraceLog(LOG_WARNING, "R2D: Failed to load CRT noise texture: %s", noise_path);
        Image noise_image = GenImageWhiteNoise(128, 128, 0.3f);
        crt->noise = LoadTextureFromImage(noise_image);
        UnloadImage(noise_image);
    } else {
        TraceLog(LOG_INFO, "R2D: CRT noise texture loaded: %s id=%u", noise_path, crt->noise.id);
    }

    crt->enabled = true;
    crt->is_ready = false;

    if (IsTextureValid(crt->noise)) {
        SetTextureFilter(crt->noise, TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(crt->noise, TEXTURE_WRAP_REPEAT);
    }

    R2D_CrtReload(crt);
    return crt->is_ready;
}

bool R2D_CrtReload(R2D_Crt *crt)
{
    if (crt == 0 || !IsTextureValid(crt->noise)) {
        return false;
    }

    return R2D_CrtLoadShader(crt);
}

void R2D_CrtClose(R2D_Crt *crt)
{
    if (crt == 0) {
        return;
    }

    if (IsTextureValid(crt->noise)) {
        UnloadTexture(crt->noise);
    }

    if (IsShaderValid(crt->shader)) {
        UnloadShader(crt->shader);
    }

    crt->is_ready = false;
    crt->enabled = false;
}

void R2D_CrtSetEnabled(R2D_Crt *crt, bool enabled)
{
    if (crt != 0) {
        crt->enabled = enabled;
    }
}

void R2D_SetCrt(R2D_Context *ctx, R2D_Crt *crt)
{
    if (ctx != 0) {
        ctx->crt = crt;
    }
}
