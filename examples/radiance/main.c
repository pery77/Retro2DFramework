#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct RadianceExample {
    R2D_Context *context;
    R2D_Radiance *radiance;
    R2D_Crt *crt;
    R2D_InputMap input;
    float time;
    float intensity;
    float ambient;
    float falloff;
    float light_range;
    int quality;
    bool sky;
    R2D_RadianceDebugView debug;
} RadianceExample;

typedef struct RadianceExampleBlock {
    Rectangle rect;
    unsigned int edge;
} RadianceExampleBlock;

typedef struct RadianceExampleRectLight {
    Rectangle rect;
    unsigned int color;
} RadianceExampleRectLight;

typedef struct RadianceExampleCircleLight {
    Vector2 center;
    float radius;
    unsigned int color;
} RadianceExampleCircleLight;

static const RadianceExampleBlock RADIANCE_BLOCKS[] = {
    { { 18.0f, 168.0f, 118.0f, 14.0f }, 0x55627aff },
    { { 34.0f, 44.0f, 126.0f, 10.0f }, 0x758098ff },
    { { 54.0f, 76.0f, 22.0f, 74.0f }, 0x758098ff },
    { { 88.0f, 132.0f, 92.0f, 10.0f }, 0x758098ff },
    { { 138.0f, 54.0f, 12.0f, 12.0f }, 0x8a93a8ff },
    { { 154.0f, 56.0f, 10.0f, 22.0f }, 0x8a93a8ff },
    { { 168.0f, 58.0f, 8.0f, 10.0f }, 0x8a93a8ff },
    { { 178.0f, 74.0f, 10.0f, 90.0f }, 0x758098ff },
    { { 208.0f, 44.0f, 14.0f, 52.0f }, 0x758098ff },
    { { 224.0f, 114.0f, 62.0f, 12.0f }, 0x758098ff },
    { { 238.0f, 40.0f, 18.0f, 128.0f }, 0x758098ff },
    { { 266.0f, 56.0f, 10.0f, 20.0f }, 0x8a93a8ff },
    { { 280.0f, 60.0f, 10.0f, 20.0f }, 0x8a93a8ff },
    { { 294.0f, 64.0f, 8.0f, 20.0f }, 0x8a93a8ff }
};

static const RadianceExampleRectLight RADIANCE_RECT_LIGHTS[] = {
    { { 14.0f, 30.0f, 74.0f, 5.0f }, 0xff65baff },
    { { 95.0f, 156.0f, 58.0f, 4.0f }, 0xffb34eff },
    { { 190.0f, 42.0f, 34.0f, 4.0f }, 0x75f0ffff },
    { { 306.0f, 46.0f, 5.0f, 74.0f }, 0x6cff9fff }
};

static const RadianceExampleCircleLight RADIANCE_CIRCLE_LIGHTS[] = {
    { { 42.0f, 124.0f }, 5.0f, 0xffef7aff },
    { { 288.0f, 140.0f }, 9.0f, 0x72d7ffff },
    { { 116.0f, 88.0f }, 6.0f, 0xb28cffff }
};

static void RadianceExample_ApplyQuality(RadianceExample *example)
{
    if (example->quality == 0) {
        R2D_RadianceSetQuality(example->radiance, 1, 8, 5);
    } else if (example->quality == 1) {
        R2D_RadianceSetQuality(example->radiance, 1, 12, 6);
    } else {
        R2D_RadianceSetQuality(example->radiance, 1, 16, 6);
    }
}

static void RadianceExample_Init(void *user_data)
{
    RadianceExample *example = (RadianceExample *)user_data;

    R2D_InputInit(&example->input);
    R2D_InputBindKey(&example->input, "debug", KEY_D);
    R2D_InputBindKey(&example->input, "sky", KEY_S);
    R2D_InputBindKey(&example->input, "toggle", KEY_R);
    R2D_InputBindKey(&example->input, "crt", KEY_C);
    R2D_InputBindKey(&example->input, "intensity_down", KEY_ONE);
    R2D_InputBindKey(&example->input, "intensity_up", KEY_TWO);
    R2D_InputBindKey(&example->input, "ambient_down", KEY_THREE);
    R2D_InputBindKey(&example->input, "ambient_up", KEY_FOUR);
    R2D_InputBindKey(&example->input, "falloff_down", KEY_FIVE);
    R2D_InputBindKey(&example->input, "falloff_up", KEY_SIX);
    R2D_InputBindKey(&example->input, "range_down", KEY_SEVEN);
    R2D_InputBindKey(&example->input, "range_up", KEY_EIGHT);
    R2D_InputBindKey(&example->input, "quality", KEY_Q);
    example->intensity = 1.8f;
    example->ambient = 0.045f;
    example->falloff = 1.15f;
    example->light_range = 224.0f;
    example->quality = 1;
    RadianceExample_ApplyQuality(example);
}

static void RadianceExample_Update(float dt, void *user_data)
{
    RadianceExample *example = (RadianceExample *)user_data;

    example->time += dt;
    R2D_InputUpdate(&example->input);

    if (R2D_InputPressed(&example->input, "debug")) {
        example->debug = (R2D_RadianceDebugView)(((int)example->debug + 1) % 3);
    }
    if (R2D_InputPressed(&example->input, "sky")) {
        example->sky = !example->sky;
    }
    if (R2D_InputPressed(&example->input, "toggle")) {
        R2D_RadianceSetEnabled(example->radiance, !example->radiance->enabled);
    }
    if (R2D_InputPressed(&example->input, "crt") && example->crt != 0) {
        R2D_CrtSetEnabled(example->crt, !example->crt->enabled);
    }
    if (R2D_InputPressed(&example->input, "quality")) {
        example->quality = (example->quality + 1) % 3;
        RadianceExample_ApplyQuality(example);
    }
    if (R2D_InputDown(&example->input, "intensity_down")) {
        example->intensity = fmaxf(0.1f, example->intensity - dt * 1.0f);
    }
    if (R2D_InputDown(&example->input, "intensity_up")) {
        example->intensity = fminf(8.0f, example->intensity + dt * 1.0f);
    }
    if (R2D_InputDown(&example->input, "ambient_down")) {
        example->ambient = fmaxf(0.0f, example->ambient - dt * 0.08f);
    }
    if (R2D_InputDown(&example->input, "ambient_up")) {
        example->ambient = fminf(0.5f, example->ambient + dt * 0.08f);
    }
    if (R2D_InputDown(&example->input, "falloff_down")) {
        example->falloff = fmaxf(0.2f, example->falloff - dt * 0.7f);
    }
    if (R2D_InputDown(&example->input, "falloff_up")) {
        example->falloff = fminf(3.0f, example->falloff + dt * 0.7f);
    }
    if (R2D_InputDown(&example->input, "range_down")) {
        example->light_range = fmaxf(24.0f, example->light_range - dt * 96.0f);
    }
    if (R2D_InputDown(&example->input, "range_up")) {
        example->light_range = fminf(512.0f, example->light_range + dt * 96.0f);
    }

    R2D_RadianceSetDebugView(example->radiance, example->debug);
    R2D_RadianceSetLight(example->radiance, example->intensity, example->ambient);
    R2D_RadianceSetFalloff(example->radiance, example->falloff);
    R2D_RadianceSetLightRange(example->radiance, example->light_range);
    R2D_RadianceSetSky(example->radiance, example->sky, R2D_ColorFromHex(0x244a7dff));
}

static void RadianceExample_DrawScene(const RadianceExample *example)
{
    Vector2 lamp = { 64.0f + sinf(example->time * 0.9f) * 20.0f, 92.0f + cosf(example->time * 0.7f) * 12.0f };
    Vector2 mouse = R2D_MouseVirtualPosition(example->context);

    ClearBackground(R2D_ColorFromHex(0x5f6878ff));
    DrawRectangle(0, 0, 320, 26, R2D_ColorFromHex(0x252d3cff));
    DrawRectangle(0, 164, 320, 36, R2D_ColorFromHex(0x303949ff));
    DrawRectangle(0, 184, 320, 16, R2D_ColorFromHex(0x202837ff));
    DrawRectangle(0, 44, 320, 2, R2D_ColorFromHex(0x6f7786ff));
    DrawRectangle(0, 96, 320, 2, R2D_ColorFromHex(0x535c6cff));
    DrawRectangle(0, 142, 320, 2, R2D_ColorFromHex(0x535c6cff));
    DrawRectangle(32, 18, 2, 146, R2D_ColorFromHex(0x535c6cff));
    DrawRectangle(92, 18, 2, 146, R2D_ColorFromHex(0x535c6cff));
    DrawRectangle(154, 18, 2, 146, R2D_ColorFromHex(0x535c6cff));
    DrawRectangle(222, 18, 2, 146, R2D_ColorFromHex(0x535c6cff));
    DrawRectangle(282, 18, 2, 146, R2D_ColorFromHex(0x535c6cff));

    for (int i = 0; i < (int)(sizeof(RADIANCE_RECT_LIGHTS) / sizeof(RADIANCE_RECT_LIGHTS[0])); ++i) {
        Rectangle rect = RADIANCE_RECT_LIGHTS[i].rect;
        DrawRectangleRec((Rectangle) { rect.x - 2.0f, rect.y - 2.0f, rect.width + 4.0f, rect.height + 4.0f }, R2D_ColorFromHex(0x10131aff));
        DrawRectangleRec(rect, R2D_ColorFromHex(RADIANCE_RECT_LIGHTS[i].color));
    }

    for (int i = 0; i < (int)(sizeof(RADIANCE_BLOCKS) / sizeof(RADIANCE_BLOCKS[0])); ++i) {
        DrawRectangleRec(RADIANCE_BLOCKS[i].rect, R2D_ColorFromHex(0x0d1016ff));
        DrawRectangleLinesEx(RADIANCE_BLOCKS[i].rect, 1.0f, R2D_ColorFromHex(RADIANCE_BLOCKS[i].edge));
    }

    for (int i = 0; i < (int)(sizeof(RADIANCE_CIRCLE_LIGHTS) / sizeof(RADIANCE_CIRCLE_LIGHTS[0])); ++i) {
        DrawCircleV(RADIANCE_CIRCLE_LIGHTS[i].center, RADIANCE_CIRCLE_LIGHTS[i].radius + 2.0f, R2D_ColorFromHex(0x10131aff));
        DrawCircleV(RADIANCE_CIRCLE_LIGHTS[i].center, RADIANCE_CIRCLE_LIGHTS[i].radius, R2D_ColorFromHex(RADIANCE_CIRCLE_LIGHTS[i].color));
    }

    DrawCircleV(lamp, 7.0f, R2D_ColorFromHex(0x10131aff));
    DrawCircleV(lamp, 5.0f, R2D_ColorFromHex(0xffba64ff));
    DrawCircleV(mouse, 6.0f, R2D_ColorFromHex(0xff7ab6ff));
}

static void RadianceExample_DrawHUD(const RadianceExample *example)
{
    char status[96];
    char params[96];

    snprintf(
        status,
        sizeof(status),
        "RC R:%s CRT:%s V:%d S:%s Q%d",
        example->radiance->enabled ? "on" : "off",
        example->crt != 0 && example->crt->enabled ? "on" : "off",
        (int)example->debug,
        example->sky ? "on" : "off",
        example->quality
    );
    snprintf(
        params,
        sizeof(params),
        "P%d Rays%d Cas%d I%.1f A%.2f F%.1f L%.0f",
        example->radiance->base_spacing,
        example->radiance->base_rays,
        example->radiance->cascade_count,
        example->intensity,
        example->ambient,
        example->falloff,
        example->light_range
    );
    DrawText(status, 4, 4, 5, R2D_ColorFromHex(0xe6edf3ff));
    DrawText(params, 4, 16, 5, R2D_ColorFromHex(0xe6edf3ff));
}

static void RadianceExample_DrawMask(R2D_Context *context, R2D_Radiance *radiance, float time)
{
    Vector2 lamp = { 64.0f + sinf(time * 0.9f) * 20.0f, 92.0f + cosf(time * 0.7f) * 12.0f };
    Vector2 mouse = R2D_MouseVirtualPosition(context);

    R2D_RadianceBeginMask(context, radiance);
    for (int i = 0; i < (int)(sizeof(RADIANCE_BLOCKS) / sizeof(RADIANCE_BLOCKS[0])); ++i) {
        R2D_RadianceDrawOccluderRect(RADIANCE_BLOCKS[i].rect);
    }
    for (int i = 0; i < (int)(sizeof(RADIANCE_RECT_LIGHTS) / sizeof(RADIANCE_RECT_LIGHTS[0])); ++i) {
        R2D_RadianceDrawEmitterRect(RADIANCE_RECT_LIGHTS[i].rect, R2D_ColorFromHex(RADIANCE_RECT_LIGHTS[i].color));
    }
    for (int i = 0; i < (int)(sizeof(RADIANCE_CIRCLE_LIGHTS) / sizeof(RADIANCE_CIRCLE_LIGHTS[0])); ++i) {
        R2D_RadianceDrawEmitterCircle(
            RADIANCE_CIRCLE_LIGHTS[i].center,
            RADIANCE_CIRCLE_LIGHTS[i].radius,
            R2D_ColorFromHex(RADIANCE_CIRCLE_LIGHTS[i].color)
        );
    }
    R2D_RadianceDrawEmitterCircle(lamp, 5.0f, R2D_ColorFromHex(0xffba64ff));
    R2D_RadianceDrawEmitterCircle(mouse, 7.0f, R2D_ColorFromHex(0xff7ab6ff));
    R2D_RadianceEndMask(context, radiance);
}

static void RadianceExample_Draw(void *user_data)
{
    RadianceExample *example = (RadianceExample *)user_data;

    RadianceExample_DrawScene(example);
    RadianceExample_DrawMask(example->context, example->radiance, example->time);
    R2D_BeginOverlay(example->context);
    RadianceExample_DrawHUD(example);
    R2D_EndOverlay(example->context);

}

static void RadianceExample_Shutdown(void *user_data)
{
    RadianceExample *example = (RadianceExample *)user_data;
    R2D_InputClear(&example->input);
}

int main(int argc, char **argv)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    R2D_Radiance radiance = { 0 };
    R2D_Crt crt = { 0 };
    RadianceExample example = { 0 };

    config.title = "Retro2D Flatland Radiance Cascades";
    config.clear_color = BLACK;
    example.sky = true;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--sky") == 0) {
            example.sky = true;
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
        RadianceExample_Init,
        RadianceExample_Update,
        RadianceExample_Draw,
        RadianceExample_Shutdown,
        &example
    });

    R2D_CrtClose(&crt);
    R2D_RadianceClose(&radiance);
    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
