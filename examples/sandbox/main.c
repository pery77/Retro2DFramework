#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>

typedef enum SandboxPage {
    SANDBOX_PAGE_INPUT = 0,
    SANDBOX_PAGE_AUDIO,
    SANDBOX_PAGE_TILEMAP,
    SANDBOX_PAGE_UI,
    SANDBOX_PAGE_RUNTIME,
    SANDBOX_PAGE_COUNT
} SandboxPage;

typedef struct Sandbox {
    R2D_Context *context;
    R2D_Crt *crt;
    R2D_InputMap input;
    R2D_StateMachine states;
    R2D_Tilemap tilemap;
    Font font_alagard;
    Font font_mecha;
    Font font_pixantiqua;
    R2D_SpriteSheet hero_sheet;
    R2D_SpriteSheet coin_sheet;
    R2D_AnimPlayer hero_anim;
    R2D_AnimPlayer coin_anim;
    R2D_Sfx coin;
    R2D_Sfx hit;
    R2D_Sfx jump;
    R2D_Sfx laser;
    R2D_Music music;
    SandboxPage page;
    float time;
    float input_flash;
    float message_timer;
    bool music_loaded;
    bool music_was_playing_before_pause;
    bool debug_draw;
    bool reload_ok;
    bool ui_toggle;
    float ui_slider;
} Sandbox;

static R2D_State Sandbox_ShowcaseState(void);
static R2D_State Sandbox_PauseState(void);
static void Sandbox_ShowcaseDraw(void *state_data, void *user_data);

static Color Sandbox_Color(bool active, unsigned int on, unsigned int off)
{
    return R2D_ColorFromHex(active ? on : off);
}

static R2D_Sfx Sandbox_LoadSfx(const char *path, R2D_Sfx fallback)
{
    R2D_Sfx sfx = fallback;

    R2D_LoadSfx(R2D_AssetPath(path), &sfx);
    return sfx;
}

static void Sandbox_InitInput(Sandbox *sandbox)
{
    R2D_InputInit(&sandbox->input);

    /* Each action can have several bindings. Keep game code action-driven. */
    R2D_InputBindKey(&sandbox->input, "left", KEY_LEFT);
    R2D_InputBindKey(&sandbox->input, "left", KEY_A);
    R2D_InputBindGamepadButton(&sandbox->input, "left", GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    R2D_InputBindGamepadAxis(&sandbox->input, "left", GAMEPAD_AXIS_LEFT_X, false);

    R2D_InputBindKey(&sandbox->input, "right", KEY_RIGHT);
    R2D_InputBindKey(&sandbox->input, "right", KEY_D);
    R2D_InputBindGamepadButton(&sandbox->input, "right", GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    R2D_InputBindGamepadAxis(&sandbox->input, "right", GAMEPAD_AXIS_LEFT_X, true);

    R2D_InputBindKey(&sandbox->input, "up", KEY_UP);
    R2D_InputBindKey(&sandbox->input, "up", KEY_W);
    R2D_InputBindGamepadButton(&sandbox->input, "up", GAMEPAD_BUTTON_LEFT_FACE_UP);
    R2D_InputBindGamepadAxis(&sandbox->input, "up", GAMEPAD_AXIS_LEFT_Y, false);

    R2D_InputBindKey(&sandbox->input, "down", KEY_DOWN);
    R2D_InputBindKey(&sandbox->input, "down", KEY_S);
    R2D_InputBindGamepadButton(&sandbox->input, "down", GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    R2D_InputBindGamepadAxis(&sandbox->input, "down", GAMEPAD_AXIS_LEFT_Y, true);

    R2D_InputBindKey(&sandbox->input, "primary", KEY_Z);
    R2D_InputBindMouseButton(&sandbox->input, "primary", MOUSE_BUTTON_LEFT);
    R2D_InputBindGamepadButton(&sandbox->input, "primary", GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

    R2D_InputBindKey(&sandbox->input, "secondary", KEY_X);
    R2D_InputBindGamepadButton(&sandbox->input, "secondary", GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);

    R2D_InputBindKey(&sandbox->input, "tertiary", KEY_V);
    R2D_InputBindGamepadButton(&sandbox->input, "tertiary", GAMEPAD_BUTTON_RIGHT_FACE_LEFT);

    R2D_InputBindKey(&sandbox->input, "laser", KEY_B);
    R2D_InputBindGamepadButton(&sandbox->input, "laser", GAMEPAD_BUTTON_RIGHT_FACE_UP);

    R2D_InputBindKey(&sandbox->input, "next_page", KEY_TAB);
    R2D_InputBindKey(&sandbox->input, "next_page", KEY_E);
    R2D_InputBindGamepadButton(&sandbox->input, "next_page", GAMEPAD_BUTTON_RIGHT_TRIGGER_1);

    R2D_InputBindKey(&sandbox->input, "prev_page", KEY_Q);
    R2D_InputBindGamepadButton(&sandbox->input, "prev_page", GAMEPAD_BUTTON_LEFT_TRIGGER_1);

    R2D_InputBindKey(&sandbox->input, "pause", KEY_ESCAPE);
    R2D_InputBindGamepadButton(&sandbox->input, "pause", GAMEPAD_BUTTON_MIDDLE_LEFT);

    R2D_InputBindKey(&sandbox->input, "music", KEY_P);
    R2D_InputBindKey(&sandbox->input, "crt", KEY_C);
    R2D_InputBindKey(&sandbox->input, "reload", KEY_R);
    R2D_InputBindKey(&sandbox->input, "debug", KEY_F3);
}

static void Sandbox_SetPage(Sandbox *sandbox, int page)
{
    if (page < 0) {
        page = SANDBOX_PAGE_COUNT - 1;
    } else if (page >= SANDBOX_PAGE_COUNT) {
        page = 0;
    }

    sandbox->page = (SandboxPage)page;
    sandbox->message_timer = 0.9f;
}

static const char *Sandbox_PageTitle(SandboxPage page)
{
    switch (page) {
        case SANDBOX_PAGE_INPUT: return "INPUT";
        case SANDBOX_PAGE_AUDIO: return "AUDIO";
        case SANDBOX_PAGE_TILEMAP: return "TILEMAP";
        case SANDBOX_PAGE_UI: return "UI";
        case SANDBOX_PAGE_RUNTIME: return "RUNTIME";
        default: return "";
    }
}

static void Sandbox_Init(void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;

    Sandbox_InitInput(sandbox);

    sandbox->page = SANDBOX_PAGE_INPUT;
    sandbox->time = 0.0f;
    sandbox->input_flash = 0.0f;
    sandbox->message_timer = 1.2f;
    sandbox->debug_draw = false;
    sandbox->reload_ok = false;
    sandbox->ui_toggle = true;
    sandbox->ui_slider = 0.65f;

    sandbox->hero_sheet = R2D_LoadSpriteSheet(R2D_AssetPath("textures/Hero/HeroSouth.png"), 16, 16);
    sandbox->coin_sheet = R2D_LoadSpriteSheet(R2D_AssetPath("textures/Dungeon/Coin Sheet.png"), 16, 16);
    sandbox->font_alagard = R2D_LoadBitmapFont("fonts/alagard.png");
    sandbox->font_mecha = R2D_LoadBitmapFont("fonts/mecha.png");
    sandbox->font_pixantiqua = R2D_LoadBitmapFont("fonts/pixantiqua.png");
    R2D_AnimPlay(&sandbox->hero_anim, R2D_AnimFrames(0, 7, 10.0f, true));
    R2D_AnimPlay(&sandbox->coin_anim, R2D_AnimFrames(0, 8, 10.0f, true));

    R2D_TilemapLoadTiledJson(&sandbox->tilemap, R2D_AssetPath("tilemaps/r2d_sandbox.json"));
    sandbox->coin = Sandbox_LoadSfx("audio/sfx/coin.r2sfx", R2D_SfxCoin());
    sandbox->hit = Sandbox_LoadSfx("audio/sfx/hit.r2sfx", R2D_SfxHit());
    sandbox->jump = Sandbox_LoadSfx("audio/sfx/jump.r2sfx", R2D_SfxJump());
    sandbox->laser = Sandbox_LoadSfx("audio/sfx/laser.r2sfx", R2D_SfxLaser());

    sandbox->music_loaded = R2D_MusicLoadSong(
        &sandbox->music,
        R2D_AssetPath("audio/music/touhou-bad-apple.r2song")
    );
    if (sandbox->music_loaded) {
        R2D_MusicSetVolume(&sandbox->music, 0.18f);
    }

    R2D_StateMachineInit(&sandbox->states, sandbox);
    R2D_StateMachineSet(&sandbox->states, Sandbox_ShowcaseState());
}

static void Sandbox_UpdateAudioPage(Sandbox *sandbox)
{
    if (R2D_InputPressed(&sandbox->input, "primary")) {
        R2D_PlaySfx(sandbox->coin);
    }

    if (R2D_InputPressed(&sandbox->input, "secondary")) {
        R2D_PlaySfx(sandbox->hit);
    }

    if (R2D_InputPressed(&sandbox->input, "tertiary")) {
        R2D_PlaySfx(sandbox->jump);
    }

    if (R2D_InputPressed(&sandbox->input, "laser")) {
        R2D_PlaySfx(sandbox->laser);
    }

    if (R2D_InputPressed(&sandbox->input, "music") && sandbox->music_loaded) {
        if (R2D_MusicIsPlaying(&sandbox->music)) {
            R2D_MusicStop(&sandbox->music);
        } else {
            R2D_MusicPlay(&sandbox->music, true);
        }
    }
}

static void Sandbox_UpdateRuntimePage(Sandbox *sandbox)
{
    if (R2D_InputPressed(&sandbox->input, "crt") && sandbox->crt != 0) {
        R2D_CrtSetEnabled(sandbox->crt, !sandbox->crt->enabled);
    }

    if (R2D_InputPressed(&sandbox->input, "reload") && sandbox->crt != 0) {
        sandbox->reload_ok = R2D_CrtReload(sandbox->crt);
        sandbox->message_timer = 1.1f;
    }

    if (R2D_InputPressed(&sandbox->input, "debug")) {
        sandbox->debug_draw = !sandbox->debug_draw;
    }
}

static void Sandbox_UpdateUiPage(Sandbox *sandbox)
{
    if (R2D_InputPressed(&sandbox->input, "primary")) {
        sandbox->ui_toggle = !sandbox->ui_toggle;
    }

    if (R2D_InputDown(&sandbox->input, "left")) {
        sandbox->ui_slider = fmaxf(0.0f, sandbox->ui_slider - 0.02f);
    }

    if (R2D_InputDown(&sandbox->input, "right")) {
        sandbox->ui_slider = fminf(1.0f, sandbox->ui_slider + 0.02f);
    }
}

static void Sandbox_ShowcaseUpdate(float dt, void *state_data, void *user_data)
{
    (void)state_data;

    Sandbox *sandbox = (Sandbox *)user_data;

    if (R2D_InputPressed(&sandbox->input, "pause")) {
        R2D_StateMachinePush(&sandbox->states, Sandbox_PauseState());
        return;
    }

    if (R2D_InputPressed(&sandbox->input, "next_page")) {
        Sandbox_SetPage(sandbox, (int)sandbox->page + 1);
    }

    if (R2D_InputPressed(&sandbox->input, "prev_page")) {
        Sandbox_SetPage(sandbox, (int)sandbox->page - 1);
    }

    if (R2D_InputPressed(&sandbox->input, "primary") ||
        R2D_InputPressed(&sandbox->input, "secondary") ||
        R2D_InputPressed(&sandbox->input, "tertiary") ||
        R2D_InputPressed(&sandbox->input, "laser")) {
        sandbox->input_flash = 0.18f;
    }

    if (sandbox->page == SANDBOX_PAGE_AUDIO) {
        Sandbox_UpdateAudioPage(sandbox);
    } else if (sandbox->page == SANDBOX_PAGE_UI) {
        Sandbox_UpdateUiPage(sandbox);
    } else if (sandbox->page == SANDBOX_PAGE_RUNTIME) {
        Sandbox_UpdateRuntimePage(sandbox);
    }

    sandbox->time += dt;
    sandbox->input_flash = fmaxf(0.0f, sandbox->input_flash - dt);
    sandbox->message_timer = fmaxf(0.0f, sandbox->message_timer - dt);
    R2D_AnimUpdate(&sandbox->hero_anim, dt);
    R2D_AnimUpdate(&sandbox->coin_anim, dt);
}

static void Sandbox_PauseEnter(void *state_data, void *user_data)
{
    (void)state_data;

    Sandbox *sandbox = (Sandbox *)user_data;

    sandbox->music_was_playing_before_pause = sandbox->music_loaded && R2D_MusicIsPlaying(&sandbox->music);
    if (sandbox->music_was_playing_before_pause) {
        R2D_MusicPause(&sandbox->music);
    }
}

static void Sandbox_PauseUpdate(float dt, void *state_data, void *user_data)
{
    (void)dt;
    (void)state_data;

    Sandbox *sandbox = (Sandbox *)user_data;

    if (R2D_InputPressed(&sandbox->input, "pause")) {
        R2D_StateMachinePop(&sandbox->states);
    }
}

static void Sandbox_PauseDraw(void *state_data, void *user_data)
{
    (void)state_data;

    const Sandbox *sandbox = (const Sandbox *)user_data;
    const int x = 105;
    const int y = 76;

    DrawRectangle(0, 0, R2D_VirtualWidth(sandbox->context), R2D_VirtualHeight(sandbox->context), R2D_ColorFromHex(0x00000099));
    DrawRectangle(x, y, 110, 42, R2D_ColorFromHex(0x101820ee));
    DrawRectangleLines(x, y, 110, 42, R2D_ColorFromHex(0xffd166ff));
    DrawText("PAUSED", x + 33, y + 9, 12, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("ESC / SELECT", x + 20, y + 26, 8, R2D_ColorFromHex(0x8ecae6ff));
}

static void Sandbox_PauseExit(void *state_data, void *user_data)
{
    (void)state_data;

    Sandbox *sandbox = (Sandbox *)user_data;

    if (sandbox->music_was_playing_before_pause) {
        R2D_MusicResume(&sandbox->music);
    }
    sandbox->music_was_playing_before_pause = false;
}

static R2D_State Sandbox_ShowcaseState(void)
{
    return (R2D_State) {
        "Showcase",
        0,
        Sandbox_ShowcaseUpdate,
        Sandbox_ShowcaseDraw,
        0,
        0
    };
}

static R2D_State Sandbox_PauseState(void)
{
    return (R2D_State) {
        "Pause",
        Sandbox_PauseEnter,
        Sandbox_PauseUpdate,
        Sandbox_PauseDraw,
        Sandbox_PauseExit,
        0
    };
}

static void Sandbox_DrawPanel(Rectangle rect)
{
    R2D_DrawUiPanel(rect, R2D_DefaultUiStyle());
}

static void Sandbox_DrawHeader(const Sandbox *sandbox)
{
    char text[80];
    R2D_TextStyle title = R2D_DefaultTextStyle(12, R2D_ColorFromHex(0xf8f8f2ff));

    title.use_shadow = true;

    R2D_DrawTextStyled("Retro2D Hello World", (Vector2) { 8.0f, 6.0f }, title);
    snprintf(text, sizeof(text), "%d/%d  %s", (int)sandbox->page + 1, SANDBOX_PAGE_COUNT, Sandbox_PageTitle(sandbox->page));
    R2D_DrawTextStyled(text, (Vector2) { 218.0f, 8.0f }, R2D_DefaultTextStyle(10, R2D_ColorFromHex(0xffd166ff)));
    DrawLine(8, 22, 312, 22, R2D_ColorFromHex(0x3a506bff));
}

static void Sandbox_DrawFooter(const Sandbox *sandbox)
{
    (void)sandbox;

    DrawLine(8, 178, 312, 178, R2D_ColorFromHex(0x3a506bff));
    DrawText("TAB/E next  Q prev  ESC pause", 8, 184, 8, R2D_ColorFromHex(0x8ecae6ff));
}

static void Sandbox_DrawButton(int x, int y, const char *label, bool active)
{
    R2D_DrawUiButton(R2D_Rect((float)x, (float)y, 38.0f, 18.0f), label, false, active, R2D_DefaultUiStyle());
}

static void Sandbox_DrawInputPage(const Sandbox *sandbox)
{
    const float axis_x = R2D_InputAxis(&sandbox->input, "left", "right");
    const float axis_y = R2D_InputAxis(&sandbox->input, "up", "down");
    const int center_x = 234;
    const int center_y = 98;
    const int dot_x = center_x + (int)(axis_x * 36.0f);
    const int dot_y = center_y + (int)(axis_y * 36.0f);
    char text[80];

    DrawText("Actions are names, not raw keys.", 16, 34, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("WASD / arrows / gamepad move the dot.", 16, 46, 8, R2D_ColorFromHex(0x8ecae6ff));
    DrawText("Z, X, V, B and mouse click flash buttons.", 16, 58, 8, R2D_ColorFromHex(0x8ecae6ff));

    Sandbox_DrawPanel(R2D_Rect(18, 82, 116, 66));
    Sandbox_DrawButton(57, 88, "UP", R2D_InputDown(&sandbox->input, "up"));
    Sandbox_DrawButton(57, 124, "DOWN", R2D_InputDown(&sandbox->input, "down"));
    Sandbox_DrawButton(18, 106, "LEFT", R2D_InputDown(&sandbox->input, "left"));
    Sandbox_DrawButton(96, 106, "RGT", R2D_InputDown(&sandbox->input, "right"));

    Sandbox_DrawPanel(R2D_Rect(170, 58, 112, 94));
    DrawCircleLines(center_x, center_y, 38.0f, R2D_ColorFromHex(0x8ecae6ff));
    DrawLine(center_x - 38, center_y, center_x + 38, center_y, R2D_ColorFromHex(0x3a506bff));
    DrawLine(center_x, center_y - 38, center_x, center_y + 38, R2D_ColorFromHex(0x3a506bff));
    DrawCircle(dot_x, dot_y, 5.0f, R2D_ColorFromHex(0xffd166ff));
    snprintf(text, sizeof(text), "axis %.2f %.2f", axis_x, axis_y);
    DrawText(text, 190, 136, 8, R2D_ColorFromHex(0xf8f8f2ff));

    Sandbox_DrawButton(36, 154, "Z", R2D_InputDown(&sandbox->input, "primary"));
    Sandbox_DrawButton(80, 154, "X", R2D_InputDown(&sandbox->input, "secondary"));
    Sandbox_DrawButton(124, 154, "V", R2D_InputDown(&sandbox->input, "tertiary"));
    Sandbox_DrawButton(168, 154, "B", R2D_InputDown(&sandbox->input, "laser"));
}

static void Sandbox_DrawAudioPage(const Sandbox *sandbox)
{
    const bool playing = sandbox->music_loaded && R2D_MusicIsPlaying(&sandbox->music);
    char text[96];

    DrawText("Tiny SFX presets and MIDI music live together.", 16, 34, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("Press the keys to trigger short retro sounds.", 16, 46, 8, R2D_ColorFromHex(0x8ecae6ff));

    Sandbox_DrawButton(28, 70, "COIN", R2D_InputDown(&sandbox->input, "primary"));
    Sandbox_DrawButton(76, 70, "HIT", R2D_InputDown(&sandbox->input, "secondary"));
    Sandbox_DrawButton(124, 70, "JUMP", R2D_InputDown(&sandbox->input, "tertiary"));
    Sandbox_DrawButton(172, 70, "LASR", R2D_InputDown(&sandbox->input, "laser"));

    Sandbox_DrawPanel(R2D_Rect(32, 106, 256, 46));
    DrawText(playing ? "MUSIC PLAYING" : "P: START MUSIC", 44, 116, 10, Sandbox_Color(playing, 0x50fa7bff, 0xffd166ff));
    snprintf(text, sizeof(text), "position %.1fs / %.1fs", R2D_MusicPosition(&sandbox->music), R2D_MusicLength(&sandbox->music));
    DrawText(sandbox->music_loaded ? text : "song file not loaded", 44, 134, 8, R2D_ColorFromHex(0xf8f8f2ff));

    if (sandbox->input_flash > 0.0f) {
        DrawRectangleLinesEx(R2D_Rect(24, 64, 202, 30), 2.0f, R2D_ColorFromHex(0xffd166ff));
    }
}

static void Sandbox_DrawTilemapPage(const Sandbox *sandbox)
{
    const float pan = sinf(sandbox->time * 0.35f) * 24.0f + 32.0f;
    const Rectangle view = R2D_Rect(pan, 18.0f, 150.0f, 92.0f);
    const Vector2 offset = { 22.0f - view.x, 58.0f - view.y };
    const int collision_layer = R2D_TilemapLayerIndex(&sandbox->tilemap, "Collision");

    DrawText("A loaded Tiled map can be drawn through a camera-sized view.", 16, 34, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("No player needed here: just the reusable bits.", 16, 46, 8, R2D_ColorFromHex(0x8ecae6ff));

    Sandbox_DrawPanel(R2D_Rect(18, 54, 164, 112));
    if (R2D_TilemapIsReady(&sandbox->tilemap)) {
        BeginScissorMode(22, 58, 154, 92);
        R2D_TilemapDrawVisible(&sandbox->tilemap, view, offset);
        if (sandbox->debug_draw && collision_layer >= 0) {
            R2D_TilemapDrawCollisionDebugVisible(&sandbox->tilemap, collision_layer, view, offset, R2D_ColorFromHex(0xff5555bb));
        }
        EndScissorMode();
    }
    DrawRectangleLines(22, 58, 154, 92, R2D_ColorFromHex(0xffd166ff));

    Sandbox_DrawPanel(R2D_Rect(202, 58, 84, 92));
    R2D_DrawAnim(&sandbox->hero_sheet, &sandbox->hero_anim, (Vector2) { 214.0f, 76.0f }, false);
    R2D_DrawAnim(&sandbox->coin_sheet, &sandbox->coin_anim, (Vector2) { 246.0f, 76.0f }, false);
    DrawText("sprites", 222, 106, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("anim players", 214, 120, 8, R2D_ColorFromHex(0x8ecae6ff));
    DrawText("F3 debug", 216, 136, 8, Sandbox_Color(sandbox->debug_draw, 0x50fa7bff, 0x8ecae6ff));
}

static void Sandbox_DrawUiPage(const Sandbox *sandbox)
{
    R2D_UiStyle ui = R2D_DefaultUiStyle();
    R2D_TextStyle text = R2D_DefaultTextStyle(10, R2D_ColorFromHex(0xf8f8f2ff));
    R2D_TextStyle title = R2D_DefaultTextStyle(18, R2D_ColorFromHex(0xffd166ff));
    R2D_TextStyle mecha = R2D_DefaultTextStyle(16, R2D_ColorFromHex(0x8ecae6ff));
    R2D_TextStyle pix = R2D_DefaultTextStyle(14, R2D_ColorFromHex(0x50fa7bff));
    const char *wrapped =
        "Text helpers provide shadow, outline, alignment and simple wrapping. "
        "The UI helpers only draw controls; your game still owns input and state.";

    title.font = sandbox->font_alagard;
    title.use_outline = true;
    mecha.font = sandbox->font_mecha;
    pix.font = sandbox->font_pixantiqua;

    R2D_DrawTextStyled("Bitmap fonts", (Vector2) { 16.0f, 32.0f }, title);
    R2D_DrawTextStyled("XNA PNG fonts loaded with LoadFont", (Vector2) { 16.0f, 52.0f }, mecha);
    R2D_DrawTextWrapped(wrapped, R2D_Rect(16.0f, 68.0f, 288.0f, 22.0f), text);

    R2D_DrawUiPanel(R2D_Rect(20.0f, 96.0f, 280.0f, 64.0f), ui);
    R2D_DrawUiButton(R2D_Rect(34.0f, 108.0f, 64.0f, 18.0f), "START", true, R2D_InputDown(&sandbox->input, "primary"), ui);
    R2D_DrawUiToggle(R2D_Rect(116.0f, 108.0f, 88.0f, 18.0f), "CRT ready", sandbox->ui_toggle, false, ui);
    R2D_DrawUiSlider(R2D_Rect(34.0f, 134.0f, 170.0f, 18.0f), "volume", sandbox->ui_slider, true, ui);
    R2D_DrawUiBar(R2D_Rect(220.0f, 134.0f, 54.0f, 10.0f), sandbox->ui_slider, R2D_ColorFromHex(0x50fa7bff), ui);

    R2D_DrawTextStyled("Z toggles  left/right slider", (Vector2) { 70.0f, 164.0f }, pix);
}

static void Sandbox_DrawRuntimePage(const Sandbox *sandbox)
{
    const bool crt_on = sandbox->crt != 0 && sandbox->crt->enabled;
    char text[80];

    DrawText("Core helpers stay small and explicit.", 16, 34, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("CRT, screenshots, states and overlays are opt-in pieces.", 16, 46, 8, R2D_ColorFromHex(0x8ecae6ff));

    Sandbox_DrawPanel(R2D_Rect(26, 66, 268, 78));
    DrawText(crt_on ? "C: CRT ON" : "C: CRT OFF", 42, 78, 10, Sandbox_Color(crt_on, 0x50fa7bff, 0xffd166ff));
    DrawText("R: reload CRT shader", 42, 96, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("F12 / PrintScreen: screenshot", 42, 108, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("ESC: state stack pause overlay", 42, 120, 8, R2D_ColorFromHex(0xf8f8f2ff));

    snprintf(text, sizeof(text), "current state: %s", R2D_StateMachineCurrentName(&sandbox->states));
    DrawText(text, 42, 154, 8, R2D_ColorFromHex(0x8ecae6ff));

    if (sandbox->message_timer > 0.0f && sandbox->page == SANDBOX_PAGE_RUNTIME) {
        DrawText(
            sandbox->reload_ok ? "shader reload ok" : "runtime page",
            198,
            154,
            8,
            sandbox->reload_ok ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0xffd166ff)
        );
    }
}

static void Sandbox_ShowcaseDraw(void *state_data, void *user_data)
{
    (void)state_data;

    const Sandbox *sandbox = (const Sandbox *)user_data;

    Sandbox_DrawHeader(sandbox);

    switch (sandbox->page) {
        case SANDBOX_PAGE_INPUT:
            Sandbox_DrawInputPage(sandbox);
            break;
        case SANDBOX_PAGE_AUDIO:
            Sandbox_DrawAudioPage(sandbox);
            break;
        case SANDBOX_PAGE_TILEMAP:
            Sandbox_DrawTilemapPage(sandbox);
            break;
        case SANDBOX_PAGE_UI:
            Sandbox_DrawUiPage(sandbox);
            break;
        case SANDBOX_PAGE_RUNTIME:
            Sandbox_DrawRuntimePage(sandbox);
            break;
        default:
            break;
    }

    Sandbox_DrawFooter(sandbox);
}

static void Sandbox_Update(float dt, void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;

    R2D_InputUpdate(&sandbox->input);
    R2D_MusicUpdate(&sandbox->music);
    R2D_StateMachineUpdate(&sandbox->states, dt);
}

static void Sandbox_Draw(void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;

    R2D_StateMachineDrawStack(&sandbox->states);
}

static void Sandbox_Shutdown(void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;

    R2D_StateMachineClear(&sandbox->states);
    R2D_MusicUnload(&sandbox->music);
    R2D_TilemapUnload(&sandbox->tilemap);
    R2D_UnloadBitmapFont(&sandbox->font_alagard);
    R2D_UnloadBitmapFont(&sandbox->font_mecha);
    R2D_UnloadBitmapFont(&sandbox->font_pixantiqua);
    R2D_UnloadSpriteSheet(&sandbox->hero_sheet);
    R2D_UnloadSpriteSheet(&sandbox->coin_sheet);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Crt crt = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    Sandbox sandbox = { 0 };

    config.title = "Retro2DFramework Hello World";
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
