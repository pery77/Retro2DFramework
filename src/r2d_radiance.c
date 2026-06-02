#include "r2d/r2d.h"

#include <math.h>

static Rectangle R2D_RadianceSourceRect(Texture2D texture)
{
    return (Rectangle) { 0.0f, 0.0f, (float)texture.width, -(float)texture.height };
}

static Rectangle R2D_RadianceDestRect(int width, int height)
{
    return (Rectangle) { 0.0f, 0.0f, (float)width, (float)height };
}

static Rectangle R2D_RadianceMaskViewportSourceRect(const R2D_Radiance *radiance)
{
    return (Rectangle) {
        (float)radiance->viewport_padding,
        (float)radiance->viewport_padding,
        (float)radiance->width,
        -(float)radiance->height
    };
}

static int R2D_RadianceCeilMultiple(int value, int multiple)
{
    return ((value + multiple - 1) / multiple) * multiple;
}

static int R2D_RadianceRayGroups(int base_rays)
{
    return (base_rays + 3) / 4;
}

static int R2D_RadianceNormalizeBaseRays(int base_rays)
{
    return R2D_RadianceRayGroups(base_rays) * 4;
}

static void R2D_RadianceConfigureTarget(RenderTexture2D target, int filter)
{
    if (IsRenderTextureValid(target) && IsTextureValid(target.texture)) {
        SetTextureFilter(target.texture, filter);
        SetTextureWrap(target.texture, TEXTURE_WRAP_CLAMP);
    }
}

static void R2D_RadianceClear(RenderTexture2D target, Color color)
{
    BeginTextureMode(target);
    ClearBackground(color);
    EndTextureMode();
}

static void R2D_RadianceUnloadTargets(R2D_Radiance *radiance)
{
    if (radiance == 0) {
        return;
    }

    if (IsRenderTextureValid(radiance->mask)) {
        UnloadRenderTexture(radiance->mask);
        radiance->mask = (RenderTexture2D) { 0 };
    }
    if (IsRenderTextureValid(radiance->cascade_a)) {
        UnloadRenderTexture(radiance->cascade_a);
        radiance->cascade_a = (RenderTexture2D) { 0 };
    }
    if (IsRenderTextureValid(radiance->cascade_b)) {
        UnloadRenderTexture(radiance->cascade_b);
        radiance->cascade_b = (RenderTexture2D) { 0 };
    }
    if (IsRenderTextureValid(radiance->color)) {
        UnloadRenderTexture(radiance->color);
        radiance->color = (RenderTexture2D) { 0 };
    }
}

static bool R2D_RadianceAllocTargets(R2D_Radiance *radiance)
{
    int multiple;
    int probe_width;
    int probe_height;
    int ray_groups;

    if (radiance == 0) {
        return false;
    }

    multiple = 1 << (radiance->cascade_count - 1);
    ray_groups = R2D_RadianceRayGroups(radiance->base_rays);
    radiance->mask_width = radiance->width + radiance->viewport_padding * 2;
    radiance->mask_height = radiance->height + radiance->viewport_padding * 2;
    probe_width = R2D_RadianceCeilMultiple((int)ceilf((float)radiance->mask_width / (float)radiance->base_spacing), multiple);
    probe_height = R2D_RadianceCeilMultiple((int)ceilf((float)radiance->mask_height / (float)radiance->base_spacing), multiple);
    radiance->cascade_width = probe_width * ray_groups;
    radiance->cascade_height = probe_height;

    radiance->mask = LoadRenderTexture(radiance->mask_width, radiance->mask_height);
    radiance->cascade_a = LoadRenderTexture(radiance->cascade_width, radiance->cascade_height);
    radiance->cascade_b = LoadRenderTexture(radiance->cascade_width, radiance->cascade_height);
    radiance->color = LoadRenderTexture(radiance->width, radiance->height);

    if (!IsRenderTextureValid(radiance->mask) ||
        !IsRenderTextureValid(radiance->cascade_a) ||
        !IsRenderTextureValid(radiance->cascade_b) ||
        !IsRenderTextureValid(radiance->color)) {
        R2D_RadianceUnloadTargets(radiance);
        return false;
    }

    R2D_RadianceConfigureTarget(radiance->mask, TEXTURE_FILTER_POINT);
    R2D_RadianceConfigureTarget(radiance->cascade_a, TEXTURE_FILTER_POINT);
    R2D_RadianceConfigureTarget(radiance->cascade_b, TEXTURE_FILTER_POINT);
    R2D_RadianceConfigureTarget(radiance->color, TEXTURE_FILTER_POINT);
    R2D_RadianceClear(radiance->mask, WHITE);
    R2D_RadianceClear(radiance->cascade_a, BLANK);
    R2D_RadianceClear(radiance->cascade_b, BLANK);
    radiance->mask_ready = false;
    return true;
}

bool R2D_RadianceInit(R2D_Radiance *radiance, int width, int height)
{
    if (radiance == 0 || width <= 0 || height <= 0) {
        return false;
    }

    *radiance = (R2D_Radiance) { 0 };
    radiance->width = width;
    radiance->height = height;
    radiance->base_spacing = 1;
    radiance->base_rays = 12;
    radiance->cascade_count = 6;
    radiance->intensity = 1.8f;
    radiance->ambient = 0.04f;
    radiance->edge_force = 10.0f;
    radiance->body_force = 1.0f;
    radiance->falloff = 1.15f;
    radiance->light_range = 224.0f;
    radiance->sky_color = R2D_ColorFromHex(0x20385fff);
    radiance->sky_enabled = false;
    radiance->enabled = true;

    if (!R2D_RadianceAllocTargets(radiance)) {
        return false;
    }

    return R2D_RadianceReload(radiance);
}

bool R2D_RadianceReload(R2D_Radiance *radiance)
{
    if (radiance == 0) {
        return false;
    }

    if (IsShaderValid(radiance->cascade_shader)) {
        UnloadShader(radiance->cascade_shader);
    }
    if (IsShaderValid(radiance->compose_shader)) {
        UnloadShader(radiance->compose_shader);
    }

    radiance->cascade_shader = R2D_LoadFragmentShader(R2D_AssetPath("shaders/radiance_flatland_cascade.fs"));
    radiance->compose_shader = R2D_LoadFragmentShader(R2D_AssetPath("shaders/radiance_flatland_compose.fs"));

    if (!IsShaderValid(radiance->cascade_shader) || !IsShaderValid(radiance->compose_shader)) {
        TraceLog(LOG_WARNING, "R2D: Radiance shaders failed to load");
        radiance->is_ready = false;
        return false;
    }

    radiance->cascade_scene_loc = GetShaderLocation(radiance->cascade_shader, "sceneTexture");
    radiance->cascade_prev_loc = GetShaderLocation(radiance->cascade_shader, "prevCascade");
    radiance->cascade_resolution_loc = GetShaderLocation(radiance->cascade_shader, "resolution");
    radiance->cascade_base_spacing_loc = GetShaderLocation(radiance->cascade_shader, "baseSpacing");
    radiance->cascade_base_rays_loc = GetShaderLocation(radiance->cascade_shader, "baseRays");
    radiance->cascade_probe_count_loc = GetShaderLocation(radiance->cascade_shader, "probeCount");
    radiance->cascade_index_loc = GetShaderLocation(radiance->cascade_shader, "cascadeIndex");
    radiance->cascade_count_loc = GetShaderLocation(radiance->cascade_shader, "cascadeCount");
    radiance->cascade_sky_enabled_loc = GetShaderLocation(radiance->cascade_shader, "skyEnabled");
    radiance->cascade_sky_color_loc = GetShaderLocation(radiance->cascade_shader, "skyColor");
    radiance->cascade_falloff_loc = GetShaderLocation(radiance->cascade_shader, "falloff");
    radiance->cascade_light_range_loc = GetShaderLocation(radiance->cascade_shader, "lightRange");
    radiance->compose_scene_loc = GetShaderLocation(radiance->compose_shader, "sceneTexture");
    radiance->compose_cascade_loc = GetShaderLocation(radiance->compose_shader, "cascadeTexture");
    radiance->compose_mask_loc = GetShaderLocation(radiance->compose_shader, "maskTexture");
    radiance->compose_resolution_loc = GetShaderLocation(radiance->compose_shader, "resolution");
    radiance->compose_base_spacing_loc = GetShaderLocation(radiance->compose_shader, "baseSpacing");
    radiance->compose_base_rays_loc = GetShaderLocation(radiance->compose_shader, "baseRays");
    radiance->compose_probe_count_loc = GetShaderLocation(radiance->compose_shader, "probeCount");
    radiance->compose_intensity_loc = GetShaderLocation(radiance->compose_shader, "intensity");
    radiance->compose_ambient_loc = GetShaderLocation(radiance->compose_shader, "ambient");
    radiance->compose_edge_force_loc = GetShaderLocation(radiance->compose_shader, "edgeForce");
    radiance->compose_body_force_loc = GetShaderLocation(radiance->compose_shader, "bodyForce");
    radiance->compose_viewport_resolution_loc = GetShaderLocation(radiance->compose_shader, "viewportResolution");
    radiance->compose_mask_offset_loc = GetShaderLocation(radiance->compose_shader, "maskOffset");
    radiance->is_ready = true;
    return true;
}

void R2D_RadianceClose(R2D_Radiance *radiance)
{
    if (radiance == 0) {
        return;
    }

    R2D_RadianceUnloadTargets(radiance);
    if (IsShaderValid(radiance->cascade_shader)) {
        UnloadShader(radiance->cascade_shader);
    }
    if (IsShaderValid(radiance->compose_shader)) {
        UnloadShader(radiance->compose_shader);
    }

    *radiance = (R2D_Radiance) { 0 };
}

void R2D_RadianceSetEnabled(R2D_Radiance *radiance, bool enabled)
{
    if (radiance != 0) {
        radiance->enabled = enabled;
    }
}

void R2D_RadianceSetDebugView(R2D_Radiance *radiance, R2D_RadianceDebugView debug_view)
{
    if (radiance != 0) {
        radiance->debug_view = debug_view;
    }
}

void R2D_RadianceSetLight(R2D_Radiance *radiance, float intensity, float ambient)
{
    if (radiance != 0) {
        radiance->intensity = intensity;
        radiance->ambient = ambient;
    }
}

void R2D_RadianceSetOccluderLight(R2D_Radiance *radiance, float edge_force, float body_force)
{
    if (radiance != 0) {
        radiance->edge_force = edge_force;
        radiance->body_force = body_force;
    }
}

void R2D_RadianceSetFalloff(R2D_Radiance *radiance, float falloff)
{
    if (radiance != 0) {
        radiance->falloff = falloff;
    }
}

void R2D_RadianceSetLightRange(R2D_Radiance *radiance, float light_range)
{
    if (radiance != 0) {
        radiance->light_range = light_range;
    }
}

void R2D_RadianceSetSky(R2D_Radiance *radiance, bool enabled, Color color)
{
    if (radiance != 0) {
        radiance->sky_enabled = enabled;
        radiance->sky_color = color;
    }
}

bool R2D_RadianceSetViewportPadding(R2D_Radiance *radiance, int padding)
{
    if (radiance == 0 || padding < 0 || padding > 512) {
        return false;
    }

    if (radiance->viewport_padding == padding) {
        return true;
    }

    R2D_RadianceUnloadTargets(radiance);
    radiance->viewport_padding = padding;

    if (!R2D_RadianceAllocTargets(radiance)) {
        radiance->is_ready = false;
        return false;
    }

    return true;
}

bool R2D_RadianceSetQuality(R2D_Radiance *radiance, int base_spacing, int base_rays, int cascade_count)
{
    if (radiance == 0 ||
        base_spacing <= 0 ||
        base_rays <= 0 ||
        cascade_count <= 0 ||
        cascade_count > 10) {
        return false;
    }

    base_rays = R2D_RadianceNormalizeBaseRays(base_rays);
    if (base_rays > 64) {
        return false;
    }

    if (radiance->base_spacing == base_spacing &&
        radiance->base_rays == base_rays &&
        radiance->cascade_count == cascade_count) {
        return true;
    }

    R2D_RadianceUnloadTargets(radiance);
    radiance->base_spacing = base_spacing;
    radiance->base_rays = base_rays;
    radiance->cascade_count = cascade_count;

    if (!R2D_RadianceAllocTargets(radiance)) {
        radiance->is_ready = false;
        return false;
    }

    return true;
}

void R2D_RadianceBeginMask(R2D_Context *ctx, R2D_Radiance *radiance)
{
    if (ctx == 0 || radiance == 0 || !radiance->is_ready) {
        return;
    }

    EndTextureMode();
    BeginTextureMode(radiance->mask);
    ClearBackground(WHITE);
    radiance->mask_ready = false;
}

void R2D_RadianceEndMask(R2D_Context *ctx, R2D_Radiance *radiance)
{
    if (ctx == 0 || radiance == 0 || !radiance->is_ready) {
        return;
    }

    EndTextureMode();
    BeginTextureMode(ctx->target);
    radiance->mask_ready = true;
}

void R2D_RadianceDrawOccluderRect(Rectangle rect)
{
    DrawRectangleRec(rect, BLACK);
}

void R2D_RadianceDrawOccluderCircle(Vector2 center, float radius)
{
    DrawCircleV(center, radius, BLACK);
}

void R2D_RadianceDrawEmitterRect(Rectangle rect, Color color)
{
    DrawRectangleRec(rect, color);
}

void R2D_RadianceDrawEmitterCircle(Vector2 center, float radius, Color color)
{
    DrawCircleV(center, radius, color);
}

Texture2D R2D_RadianceRender(R2D_Radiance *radiance, Texture2D color_texture)
{
    Vector2 resolution;
    Vector2 viewport_resolution;
    Vector2 mask_offset;
    Vector2 probe_count;
    Vector3 sky_color;
    int ray_groups;
    int sky_enabled;
    int i;

    if (radiance == 0 || !radiance->is_ready || !radiance->mask_ready) {
        return color_texture;
    }

    resolution = (Vector2) { (float)radiance->mask_width, (float)radiance->mask_height };
    viewport_resolution = (Vector2) { (float)radiance->width, (float)radiance->height };
    mask_offset = (Vector2) { (float)radiance->viewport_padding, (float)radiance->viewport_padding };
    ray_groups = R2D_RadianceRayGroups(radiance->base_rays);
    probe_count = (Vector2) {
        (float)(radiance->cascade_width / ray_groups),
        (float)radiance->cascade_height
    };
    sky_color = (Vector3) {
        (float)radiance->sky_color.r / 255.0f,
        (float)radiance->sky_color.g / 255.0f,
        (float)radiance->sky_color.b / 255.0f
    };
    sky_enabled = radiance->sky_enabled ? 1 : 0;

    if (radiance->debug_view == R2D_RADIANCE_DEBUG_MASK) {
        BeginTextureMode(radiance->color);
        ClearBackground(BLANK);
        DrawTexturePro(radiance->mask.texture, R2D_RadianceMaskViewportSourceRect(radiance), R2D_RadianceDestRect(radiance->width, radiance->height), (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
        EndTextureMode();
        return radiance->color.texture;
    }

    if ((radiance->cascade_count - 1) % 2 == 0) {
        R2D_RadianceClear(radiance->cascade_a, BLANK);
    } else {
        R2D_RadianceClear(radiance->cascade_b, BLANK);
    }

    for (i = radiance->cascade_count - 1; i >= 0; --i) {
        RenderTexture2D source = (i % 2 == 0) ? radiance->cascade_a : radiance->cascade_b;
        RenderTexture2D destination = (i % 2 == 0) ? radiance->cascade_b : radiance->cascade_a;

        BeginTextureMode(destination);
        ClearBackground(BLANK);
        BeginShaderMode(radiance->cascade_shader);
        SetShaderValueTexture(radiance->cascade_shader, radiance->cascade_scene_loc, radiance->mask.texture);
        SetShaderValueTexture(radiance->cascade_shader, radiance->cascade_prev_loc, source.texture);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_resolution_loc, &resolution, SHADER_UNIFORM_VEC2);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_base_spacing_loc, &radiance->base_spacing, SHADER_UNIFORM_INT);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_base_rays_loc, &radiance->base_rays, SHADER_UNIFORM_INT);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_probe_count_loc, &probe_count, SHADER_UNIFORM_VEC2);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_index_loc, &i, SHADER_UNIFORM_INT);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_count_loc, &radiance->cascade_count, SHADER_UNIFORM_INT);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_sky_enabled_loc, &sky_enabled, SHADER_UNIFORM_INT);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_sky_color_loc, &sky_color, SHADER_UNIFORM_VEC3);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_falloff_loc, &radiance->falloff, SHADER_UNIFORM_FLOAT);
        SetShaderValue(radiance->cascade_shader, radiance->cascade_light_range_loc, &radiance->light_range, SHADER_UNIFORM_FLOAT);
        DrawTexturePro(source.texture, R2D_RadianceSourceRect(source.texture), R2D_RadianceDestRect(radiance->cascade_width, radiance->cascade_height), (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
        EndShaderMode();
        EndTextureMode();
    }

    if (radiance->debug_view == R2D_RADIANCE_DEBUG_CASCADE) {
        BeginTextureMode(radiance->color);
        ClearBackground(BLANK);
        DrawTexturePro(radiance->cascade_b.texture, R2D_RadianceSourceRect(radiance->cascade_b.texture), R2D_RadianceDestRect(radiance->width, radiance->height), (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
        EndTextureMode();
        return radiance->color.texture;
    }

    BeginTextureMode(radiance->color);
    ClearBackground(BLANK);
    BeginShaderMode(radiance->compose_shader);
    SetShaderValueTexture(radiance->compose_shader, radiance->compose_scene_loc, color_texture);
    SetShaderValueTexture(radiance->compose_shader, radiance->compose_cascade_loc, radiance->cascade_b.texture);
    SetShaderValueTexture(radiance->compose_shader, radiance->compose_mask_loc, radiance->mask.texture);
    SetShaderValue(radiance->compose_shader, radiance->compose_resolution_loc, &resolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(radiance->compose_shader, radiance->compose_base_spacing_loc, &radiance->base_spacing, SHADER_UNIFORM_INT);
    SetShaderValue(radiance->compose_shader, radiance->compose_base_rays_loc, &radiance->base_rays, SHADER_UNIFORM_INT);
    SetShaderValue(radiance->compose_shader, radiance->compose_probe_count_loc, &probe_count, SHADER_UNIFORM_VEC2);
    SetShaderValue(radiance->compose_shader, radiance->compose_intensity_loc, &radiance->intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(radiance->compose_shader, radiance->compose_ambient_loc, &radiance->ambient, SHADER_UNIFORM_FLOAT);
    SetShaderValue(radiance->compose_shader, radiance->compose_edge_force_loc, &radiance->edge_force, SHADER_UNIFORM_FLOAT);
    SetShaderValue(radiance->compose_shader, radiance->compose_body_force_loc, &radiance->body_force, SHADER_UNIFORM_FLOAT);
    SetShaderValue(radiance->compose_shader, radiance->compose_viewport_resolution_loc, &viewport_resolution, SHADER_UNIFORM_VEC2);
    SetShaderValue(radiance->compose_shader, radiance->compose_mask_offset_loc, &mask_offset, SHADER_UNIFORM_VEC2);
    DrawTexturePro(color_texture, R2D_RadianceSourceRect(color_texture), R2D_RadianceDestRect(radiance->width, radiance->height), (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
    EndShaderMode();
    EndTextureMode();

    return radiance->color.texture;
}

void R2D_SetRadiance(R2D_Context *ctx, R2D_Radiance *radiance)
{
    if (ctx != 0) {
        ctx->radiance = radiance;
    }
}
