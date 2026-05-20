#include "r2d/r2d.h"

#include <stdio.h>

typedef struct SaveExample {
    R2D_SaveData save;
    const char *path;
    bool loaded;
    bool saved;
    float message_timer;
} SaveExample;

static void SaveExample_Save(SaveExample *example)
{
    example->saved = R2D_SaveDataSave(example->path, example->save);
    example->message_timer = 1.2f;
}

static void SaveExample_Init(void *user_data)
{
    SaveExample *example = (SaveExample *)user_data;

    example->path = R2D_UserDataPath("Retro2DFramework", "save_example.ini");
    example->loaded = R2D_SaveDataLoad(example->path, &example->save);
    example->saved = false;
    example->message_timer = 1.2f;
}

static void SaveExample_Update(float dt, void *user_data)
{
    SaveExample *example = (SaveExample *)user_data;

    if (example->message_timer > 0.0f) {
        example->message_timer -= dt;
    }

    if (IsKeyPressed(KEY_P)) {
        example->save.progress++;
    }

    if (IsKeyPressed(KEY_H)) {
        example->save.high_score += 100;
    }

    if (IsKeyPressed(KEY_F)) {
        example->save.fullscreen = !example->save.fullscreen;
    }

    if (IsKeyPressed(KEY_UP)) {
        example->save.master_volume = R2D_Clamp01(example->save.master_volume + 0.1f);
    }

    if (IsKeyPressed(KEY_DOWN)) {
        example->save.master_volume = R2D_Clamp01(example->save.master_volume - 0.1f);
    }

    if (IsKeyPressed(KEY_S)) {
        SaveExample_Save(example);
    }

    if (IsKeyPressed(KEY_R)) {
        example->loaded = R2D_SaveDataLoad(example->path, &example->save);
        example->message_timer = 1.2f;
    }

    if (IsKeyPressed(KEY_D)) {
        example->save = R2D_SaveDataDefault();
        SaveExample_Save(example);
    }
}

static void SaveExample_Draw(void *user_data)
{
    const SaveExample *example = (const SaveExample *)user_data;
    R2D_TextStyle path_style = R2D_DefaultTextStyle(10, R2D_ColorFromHex(0xf8f8f2ff));
    char text[256];

    DrawText("Save data example", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("P progress  H score  Up/Down volume  F fullscreen flag", 12, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("S save  R reload  D defaults", 12, 42, 8, R2D_ColorFromHex(0xf8f8f2ff));

    DrawRectangle(12, 60, 296, 94, R2D_ColorFromHex(0x101820ff));
    DrawRectangleLines(12, 60, 296, 94, R2D_ColorFromHex(0x3a506bff));

    snprintf(text, sizeof(text), "version: %d", example->save.version);
    DrawText(text, 22, 70, 8, R2D_ColorFromHex(0xf8f8f2ff));
    snprintf(text, sizeof(text), "progress: %d", example->save.progress);
    DrawText(text, 22, 84, 8, R2D_ColorFromHex(0xf8f8f2ff));
    snprintf(text, sizeof(text), "high score: %d", example->save.high_score);
    DrawText(text, 22, 98, 8, R2D_ColorFromHex(0xf8f8f2ff));
    snprintf(text, sizeof(text), "master volume: %.1f", example->save.master_volume);
    DrawText(text, 22, 112, 8, R2D_ColorFromHex(0xf8f8f2ff));
    snprintf(text, sizeof(text), "fullscreen flag: %s", example->save.fullscreen ? "true" : "false");
    DrawText(text, 22, 126, 8, R2D_ColorFromHex(0xf8f8f2ff));

    DrawText("file:", 2, 158, 8, R2D_ColorFromHex(0x8ecae6ff));
    R2D_DrawTextWrapped(example->path, R2D_Rect(22.0f, 158.0f, 280.0f, 32.0f), path_style);

    if (example->message_timer > 0.0f) {
        if (example->saved) {
            DrawText("saved", 236, 144, 8, R2D_ColorFromHex(0x06d6a0ff));
        } else if (example->loaded) {
            DrawText("loaded", 232, 144, 8, R2D_ColorFromHex(0x8ecae6ff));
        } else {
            DrawText("defaults", 220, 144, 8, R2D_ColorFromHex(0xffd166ff));
        }
    }

    DrawRectangle(10, 184, 300, 12, R2D_ColorFromHex(0x101820cc));
    DrawText("Close and reopen after saving to verify persistence.", 14, 187, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    SaveExample example = { 0 };

    config.title = "Retro2D Save Data Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_Run(&context, (R2D_App) {
        SaveExample_Init,
        SaveExample_Update,
        SaveExample_Draw,
        0,
        &example
    });

    R2D_Close(&context);
    return 0;
}
