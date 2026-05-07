#include "r2d/r2d.h"

static const char *R2D_CRT_FRAGMENT_SHADER =
    "#version 330\n"
    "\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "\n"
    "uniform sampler2D texture0;\n"
    "uniform sampler2D noiseTexture;\n"
    "uniform vec2 resolution;\n"
    "uniform float rnd;\n"
    "\n"
    "out vec4 finalColor;\n"
    "\n"
    "vec2 curve_uv(vec2 uv)\n"
    "{\n"
    "    vec2 centered = uv * 2.0 - 1.0;\n"
    "    centered *= 1.0 + dot(centered, centered) * 0.045;\n"
    "    return centered * 0.5 + 0.5;\n"
    "}\n"
    "\n"
    "float vignette(vec2 uv)\n"
    "{\n"
    "    vec2 edge = uv * (1.0 - uv.yx);\n"
    "    return clamp(pow(edge.x * edge.y * 18.0, 0.28), 0.0, 1.0);\n"
    "}\n"
    "\n"
    "void main()\n"
    "{\n"
    "    vec2 uv = curve_uv(fragTexCoord);\n"
    "    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {\n"
    "        finalColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "        return;\n"
    "    }\n"
    "\n"
    "    vec2 chroma = vec2(0.75 / max(resolution.x, 1.0), 0.0);\n"
    "    float red = texture(texture0, uv - chroma).r;\n"
    "    float green = texture(texture0, uv).g;\n"
    "    float blue = texture(texture0, uv + chroma).b;\n"
    "    vec3 color = vec3(red, green, blue);\n"
    "\n"
    "    float scanline = 0.92 + 0.08 * sin(uv.y * resolution.y * 3.14159265);\n"
    "    vec3 noise = texture(noiseTexture, uv * 2.0 + vec2(rnd, rnd * 0.37)).rgb;\n"
    "\n"
    "    color *= scanline;\n"
    "    color += (noise - 0.5) * 0.045;\n"
    "    color *= vignette(uv);\n"
    "\n"
    "    finalColor = vec4(color, 1.0) * fragColor;\n"
    "}\n";

bool R2D_CrtInit(R2D_Crt *crt)
{
    if (crt == 0) {
        return false;
    }

    Image noise_image = GenImageWhiteNoise(128, 128, 0.5f);

    crt->shader = LoadShaderFromMemory(0, R2D_CRT_FRAGMENT_SHADER);
    crt->noise = LoadTextureFromImage(noise_image);
    UnloadImage(noise_image);

    crt->resolution_loc = GetShaderLocation(crt->shader, "resolution");
    crt->noise_loc = GetShaderLocation(crt->shader, "noiseTexture");
    crt->random_loc = GetShaderLocation(crt->shader, "rnd");
    crt->enabled = true;
    crt->is_ready = IsShaderValid(crt->shader) && IsTextureValid(crt->noise);

    if (crt->is_ready) {
        SetTextureFilter(crt->noise, TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(crt->noise, TEXTURE_WRAP_REPEAT);
    }

    return crt->is_ready;
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
