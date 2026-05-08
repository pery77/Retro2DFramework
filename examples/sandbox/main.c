#include "r2d/r2d.h"

#include <stdio.h>

typedef struct Sandbox {
    Vector2 player;
    float blink_timer;
    float reload_message_timer;
    R2D_Sfx coin;
    R2D_Sfx hit;
    R2D_Sfx jump;
    R2D_Sfx laser;
    R2D_Sfx explosion;
    R2D_Sfx powerup;
    R2D_Music music;
    bool reload_ok;
    bool music_loaded;
    R2D_Context *context;
    R2D_Crt *crt;
} Sandbox;

static R2D_Sfx Sandbox_LoadSfx(const char *path, R2D_Sfx fallback)
{
    R2D_Sfx sfx = fallback;

    R2D_LoadSfx(R2D_AssetPath(path), &sfx);
    return sfx;
}

static void Sandbox_Init(void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;
    char midi_path[1024];
    char soundfont_path[1024];

    sandbox->player = (Vector2) { 152.0f, 82.0f };
    sandbox->blink_timer = 0.0f;
    sandbox->reload_message_timer = 0.0f;
    sandbox->coin = Sandbox_LoadSfx("audio/sfx/coin.r2sfx", R2D_SfxCoin());
    sandbox->hit = Sandbox_LoadSfx("audio/sfx/hit.r2sfx", R2D_SfxHit());
    sandbox->jump = Sandbox_LoadSfx("audio/sfx/jump.r2sfx", R2D_SfxJump());
    sandbox->laser = Sandbox_LoadSfx("audio/sfx/laser.r2sfx", R2D_SfxLaser());
    sandbox->explosion = Sandbox_LoadSfx("audio/sfx/explosion.r2sfx", R2D_SfxExplosion());
    sandbox->powerup = Sandbox_LoadSfx("audio/sfx/powerup.r2sfx", R2D_SfxPowerup());
    sandbox->reload_ok = false;

    snprintf(midi_path, sizeof(midi_path), "%s", R2D_AssetPath("audio/music/touhou-bad-apple.mid"));
    snprintf(soundfont_path, sizeof(soundfont_path), "%s", R2D_AssetPath("audio/soundfonts/8-Bit_Sounds.sf2"));

    sandbox->music_loaded = FileExists(midi_path) &&
        FileExists(soundfont_path) &&
        R2D_MusicLoad(&sandbox->music, midi_path, soundfont_path);

    if (sandbox->music_loaded) {
        R2D_MusicPlay(&sandbox->music, true);
    }
}

static void Sandbox_Update(float dt, void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;
    const float speed = 80.0f;

    R2D_MusicUpdate(&sandbox->music);

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        sandbox->player.x -= speed * dt;
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        sandbox->player.x += speed * dt;
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        sandbox->player.y -= speed * dt;
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        sandbox->player.y += speed * dt;
    }

    if (IsKeyPressed(KEY_C) && sandbox->crt != 0) {
        R2D_CrtSetEnabled(sandbox->crt, !sandbox->crt->enabled);
    }

    if (IsKeyPressed(KEY_R) && sandbox->crt != 0) {
        sandbox->reload_ok = R2D_CrtReload(sandbox->crt);
        sandbox->reload_message_timer = 1.25f;
    }

    if (sandbox->reload_message_timer > 0.0f) {
        sandbox->reload_message_timer -= dt;
    }

    if (IsKeyPressed(KEY_Z)) {
        R2D_PlaySfx(sandbox->coin);
    }

    if (IsKeyPressed(KEY_X)) {
        R2D_PlaySfx(sandbox->hit);
    }

    if (IsKeyPressed(KEY_V)) {
        R2D_PlaySfx(sandbox->jump);
    }

    if (IsKeyPressed(KEY_B)) {
        R2D_PlaySfx(sandbox->laser);
    }

    if (IsKeyPressed(KEY_N)) {
        R2D_PlaySfx(sandbox->explosion);
    }

    if (IsKeyPressed(KEY_M)) {
        R2D_PlaySfx(sandbox->powerup);
    }

    if (IsKeyPressed(KEY_P) && sandbox->music_loaded) {
        if (R2D_MusicIsPlaying(&sandbox->music)) {
            R2D_MusicStop(&sandbox->music);
        } else {
            R2D_MusicPlay(&sandbox->music, true);
        }
    }

    sandbox->blink_timer += dt;
}

static void Sandbox_DrawGrid(const R2D_Context *context)
{
    const Color grid = R2D_ColorFromHex(0x2a2a3aff);
    const int width = R2D_VirtualWidth(context);
    const int height = R2D_VirtualHeight(context);

    for (int x = 0; x <= width; x += 16) {
        DrawLine(x, 0, x, height, grid);
    }

    for (int y = 0; y <= height; y += 16) {
        DrawLine(0, y, width, y, grid);
    }
}

static void Sandbox_Draw(void *user_data)
{
    const Sandbox *sandbox = (const Sandbox *)user_data;
    const bool blink = ((int)(sandbox->blink_timer * 4.0f) % 2) == 0;

    Sandbox_DrawGrid(sandbox->context);
    DrawText("Retro2DFramework", 8, 8, 10, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("WASD / Arrows", 8, 22, 8, R2D_ColorFromHex(0x8be9fdff));
    if (sandbox->crt != 0) {
        DrawText(sandbox->crt->enabled ? "C: CRT ON" : "C: CRT OFF", 8, 34, 8, R2D_ColorFromHex(0x50fa7bff));
        DrawText("R: RELOAD CRT", 8, 46, 8, R2D_ColorFromHex(0xffb86cff));
        DrawText("Z/X/V/B/N/M: SFX", 8, 58, 8, R2D_ColorFromHex(0xffb86cff));
        DrawText(sandbox->music_loaded ? "P: MUSIC" : "MUSIC: ADD theme.mid + chiptune.sf2", 8, 70, 8, R2D_ColorFromHex(0xffb86cff));
    }

    if (sandbox->reload_message_timer > 0.0f) {
        DrawText(
            sandbox->reload_ok ? "SHADER RELOADED" : "SHADER ERROR",
            8,
            82,
            8,
            sandbox->reload_ok ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0xff5555ff)
        );
    }

    DrawRectangleRec(R2D_Rect(sandbox->player.x, sandbox->player.y, 16.0f, 16.0f), R2D_ColorFromHex(0xff5555ff));
    DrawRectangleLinesEx(R2D_Rect(sandbox->player.x, sandbox->player.y, 16.0f, 16.0f), 1.0f, R2D_ColorFromHex(0xf1fa8cff));

    if (blink) {
        DrawPixel((int)sandbox->player.x + 11, (int)sandbox->player.y + 5, BLACK);
    }
}

static void Sandbox_Shutdown(void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;

    R2D_MusicUnload(&sandbox->music);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Crt crt = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    Sandbox sandbox = { 0 };

    config.title = "Retro2DFramework Sandbox";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_AudioInit();
    R2D_CrtInit(&crt);
    R2D_SetCrt(&context, &crt);
    sandbox.context = &context;
    sandbox.crt = &crt;

    R2D_Run(&context, (R2D_App) {
        Sandbox_Init,
        Sandbox_Update,
        Sandbox_Draw,
        Sandbox_Shutdown,
        &sandbox
    });

    R2D_CrtClose(&crt);
    R2D_AudioClose();
    R2D_Close(&context);
    return 0;
}
