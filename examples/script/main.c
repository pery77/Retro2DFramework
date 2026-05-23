#include "r2d/r2d.h"

#include <stdio.h>

typedef struct ScriptExample {
    R2D_Script script;
    float time;
    Vector2 position;
    char status[128];
} ScriptExample;

static void ScriptExample_Init(void *user_data)
{
    ScriptExample *example = (ScriptExample *)user_data;

    example->position = (Vector2) { 160.0f, 100.0f };

    if (!R2D_ScriptAvailable()) {
        snprintf(example->status, sizeof(example->status), "LuaJIT disabled. Configure with R2D_ENABLE_LUAJIT=ON.");
        return;
    }

    if (!R2D_ScriptInit(&example->script)) {
        snprintf(example->status, sizeof(example->status), "LuaJIT init failed.");
        return;
    }

    if (!R2D_ScriptLoadFile(&example->script, R2D_AssetPath("scripts/demo.lua"))) {
        snprintf(example->status, sizeof(example->status), "Could not load assets/scripts/demo.lua.");
        return;
    }

    snprintf(example->status, sizeof(example->status), "LuaJIT script loaded.");
}

static void ScriptExample_Update(float dt, void *user_data)
{
    ScriptExample *example = (ScriptExample *)user_data;

    example->time += dt;

    if (!R2D_ScriptIsReady(&example->script)) {
        return;
    }

    R2D_ScriptPushNumber(&example->script, dt);
    if (R2D_ScriptCall(&example->script, "update", 1, 3)) {
        example->position.x = (float)R2D_ScriptToNumber(&example->script, -3, example->position.x);
        example->position.y = (float)R2D_ScriptToNumber(&example->script, -2, example->position.y);
        snprintf(
            example->status,
            sizeof(example->status),
            "%s",
            R2D_ScriptToString(&example->script, -1, "Lua update returned.")
        );
        R2D_ScriptPop(&example->script, 3);
    }
}

static void ScriptExample_Draw(void *user_data)
{
    const ScriptExample *example = (const ScriptExample *)user_data;
    R2D_TextStyle title = R2D_DefaultTextStyle(18, R2D_ColorFromHex(0xffd166ff));
    R2D_TextStyle body = R2D_DefaultTextStyle(10, R2D_ColorFromHex(0xf8f8f2ff));

    title.use_shadow = true;

    R2D_DrawTextAligned("LuaJIT scripting", R2D_Rect(0.0f, 10.0f, 320.0f, 24.0f), title, R2D_TEXT_ALIGN_CENTER);
    R2D_DrawTextWrapped(example->status, R2D_Rect(34.0f, 42.0f, 252.0f, 36.0f), body);

    DrawCircleV(example->position, 12.0f, R2D_ColorFromHex(0xf8f8f2ff));
    DrawCircleV(example->position, 9.0f, R2D_ColorFromHex(0x06d6a0ff));
    DrawCircleV(example->position, 3.0f, R2D_ColorFromHex(0x15151fff));

    DrawText("Lua returns the dot position each frame.", 54, 180, 8, R2D_ColorFromHex(0x8ecae6ff));
}

static void ScriptExample_Shutdown(void *user_data)
{
    ScriptExample *example = (ScriptExample *)user_data;

    R2D_ScriptClose(&example->script);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    ScriptExample example = { 0 };

    config.title = "Retro2D LuaJIT Script Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_Run(&context, (R2D_App) {
        ScriptExample_Init,
        ScriptExample_Update,
        ScriptExample_Draw,
        ScriptExample_Shutdown,
        &example
    });

    R2D_Close(&context);
    return 0;
}
