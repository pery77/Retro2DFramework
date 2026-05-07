#include "r2d/r2d.h"

static bool R2D_CrtLoadShader(R2D_Crt *crt)
{
    Shader shader = LoadShader(0, R2D_AssetPath("shaders/crt.fs"));

    if (!IsShaderValid(shader)) {
        return false;
    }

    if (IsShaderValid(crt->shader)) {
        UnloadShader(crt->shader);
    }

    crt->shader = shader;
    crt->resolution_loc = GetShaderLocation(crt->shader, "resolution");
    crt->noise_loc = GetShaderLocation(crt->shader, "noiseTexture");
    crt->random_loc = GetShaderLocation(crt->shader, "rnd");
    crt->is_ready = IsTextureValid(crt->noise);

    return crt->is_ready;
}

bool R2D_CrtInit(R2D_Crt *crt)
{
    if (crt == 0) {
        return false;
    }

    Image noise_image = GenImageWhiteNoise(128, 128, 0.5f);

    crt->noise = LoadTextureFromImage(noise_image);
    UnloadImage(noise_image);

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
