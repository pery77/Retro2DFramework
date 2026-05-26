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
    int quality;
    bool sky;
    R2D_RadianceDebugView debug;
} RadianceExample;

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
    R2D_InputBindKey(&example->input, "quality", KEY_Q);
    example->intensity = 1.3f;
    example->ambient = 0.06f;
    example->quality = 2;
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
        if (example->quality == 0) {
            R2D_RadianceSetQuality(example->radiance, 2, 8, 4);
        } else if (example->quality == 1) {
            R2D_RadianceSetQuality(example->radiance, 1, 16, 8);
        } else {
            R2D_RadianceSetQuality(example->radiance, 0.44f, 64, 10);
        }
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

    R2D_RadianceSetDebugView(example->radiance, example->debug);
    R2D_RadianceSetLight(example->radiance, example->intensity, example->ambient);
    R2D_RadianceSetSky(example->radiance, example->sky, R2D_ColorFromHex(0x244a7dff));
}

static void RadianceExample_DrawScene(const RadianceExample *example)
{
    Vector2 lamp = { 78.0f + sinf(example->time * 0.9f) * 22.0f, 70.0f + cosf(example->time * 0.7f) * 8.0f };
    Vector2 mouse = R2D_MouseVirtualPosition(example->context);
    
    ClearBackground(R2D_ColorFromHex(0xccccccff));
    DrawRectangle(0, 154, 320, 46, R2D_ColorFromHex(0x3c4658ff));
    DrawRectangle(0, 0, 320, 16, R2D_ColorFromHex(0x273244ff));
    DrawRectangle(38, 42, 42, 96, R2D_ColorFromHex(0x101217ff));
    DrawRectangle(140, 78, 36, 76, R2D_ColorFromHex(0x101217ff));
    DrawRectangle(224, 48, 52, 108, R2D_ColorFromHex(0x101217ff));
    DrawRectangleLines(38, 42, 42, 96, R2D_ColorFromHex(0x68758cff));
    DrawRectangleLines(140, 78, 36, 76, R2D_ColorFromHex(0x68758cff));
    DrawRectangleLines(224, 48, 52, 108, R2D_ColorFromHex(0x68758cff));
    //DrawCircleV(lamp, 5.0f, R2D_ColorFromHex(0xffca5cff));
    //DrawCircle(296, 88, 5.0f, R2D_ColorFromHex(0x75d7ffff));
    //DrawCircleV(mouse, 4.0f, R2D_ColorFromHex(0xff7ab6ff));



}

static void RadianceExample_DrawHUD(const RadianceExample *example)
{
    char text[96];
    snprintf(
        text,
        sizeof(text),
        "Flatland RC  R:%s C:%s D:%d S:%s Q:%d 1/2 %.1f 3/4 %.2f",
        example->radiance->enabled ? "on" : "off",
        example->crt != 0 && example->crt->enabled ? "on" : "off",
        (int)example->debug,
        example->sky ? "on" : "off",
        example->quality,
        example->intensity,
        example->ambient
    );
    DrawText(text, 8, 8, 8, R2D_ColorFromHex(0xe6edf3ff));
}

static void RadianceExample_DrawMask(R2D_Context *context, R2D_Radiance *radiance, float time)
{
    //Vector2 lamp = { 78.0f + sinf(time * 0.9f) * 22.0f, 70.0f + cosf(time * 0.7f) * 8.0f };
    Vector2 mouse = R2D_MouseVirtualPosition(context);

    R2D_RadianceBeginMask(context, radiance);
    R2D_RadianceDrawOccluderRect(R2D_Rect(38.0f, 42.0f, 42.0f, 96.0f));
    R2D_RadianceDrawOccluderRect(R2D_Rect(140.0f, 78.0f, 36.0f, 76.0f));
    R2D_RadianceDrawOccluderRect(R2D_Rect(224.0f, 48.0f, 52.0f, 108.0f));
    //R2D_RadianceDrawEmitterCircle(lamp, 10.0f, R2D_ColorFromHex(0xffca5cff));
    R2D_RadianceDrawEmitterCircle((Vector2) { 296.0f, 88.0f }, 9.0f, R2D_ColorFromHex(0x75d7ffff));
    R2D_RadianceDrawEmitterCircle(mouse, 8.0f, R2D_ColorFromHex(0xff7ab6ff));
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

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--sky") == 0) {
            example.sky = true;
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
