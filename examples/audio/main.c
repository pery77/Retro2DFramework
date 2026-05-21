#include "r2d/r2d.h"

#include <stdio.h>

typedef struct AudioExample {
    R2D_InputMap input;
    R2D_Sfx coin;
    R2D_Sfx hit;
    R2D_Sfx jump;
    R2D_Sfx laser;
    R2D_Music music;
    bool music_loaded;
    float flash;
} AudioExample;

static void AudioExample_InitInput(AudioExample *example)
{
    R2D_InputInit(&example->input);

    R2D_InputBindKey(&example->input, "coin", KEY_Z);
    R2D_InputBindGamepadButton(&example->input, "coin", GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

    R2D_InputBindKey(&example->input, "hit", KEY_X);
    R2D_InputBindGamepadButton(&example->input, "hit", GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);

    R2D_InputBindKey(&example->input, "jump", KEY_C);
    R2D_InputBindGamepadButton(&example->input, "jump", GAMEPAD_BUTTON_RIGHT_FACE_LEFT);

    R2D_InputBindKey(&example->input, "laser", KEY_V);
    R2D_InputBindGamepadButton(&example->input, "laser", GAMEPAD_BUTTON_RIGHT_FACE_UP);

    R2D_InputBindKey(&example->input, "music", KEY_P);
    R2D_InputBindGamepadButton(&example->input, "music", GAMEPAD_BUTTON_MIDDLE_RIGHT);

    R2D_InputBindKey(&example->input, "fade", KEY_F);
}

static R2D_Sfx AudioExample_LoadSfx(const char *path, R2D_Sfx fallback)
{
    R2D_Sfx sfx = fallback;

    R2D_LoadSfx(R2D_AssetPath(path), &sfx);
    return sfx;
}

static void AudioExample_Init(void *user_data)
{
    AudioExample *example = (AudioExample *)user_data;

    AudioExample_InitInput(example);

    example->coin = AudioExample_LoadSfx("audio/sfx/coin.r2sfx", R2D_SfxCoin());
    example->hit = AudioExample_LoadSfx("audio/sfx/hit.r2sfx", R2D_SfxHit());
    example->jump = AudioExample_LoadSfx("audio/sfx/jump.r2sfx", R2D_SfxJump());
    example->laser = AudioExample_LoadSfx("audio/sfx/laser.r2sfx", R2D_SfxLaser());

    example->music_loaded = R2D_MusicLoadSong(
        &example->music,
        R2D_AssetPath("audio/music/touhou-bad-apple.r2song")
    );
    if (example->music_loaded) {
        R2D_MusicSetVolume(&example->music, 0.16f);
        R2D_MusicSetGroup(&example->music, R2D_AUDIO_GROUP_MUSIC);
    }
}

static void AudioExample_Update(float dt, void *user_data)
{
    AudioExample *example = (AudioExample *)user_data;

    R2D_InputUpdate(&example->input);
    R2D_AudioMixerUpdate(dt);
    R2D_MusicUpdate(&example->music);

    if (R2D_InputPressed(&example->input, "coin")) {
        R2D_PlaySfxRandomPitch(example->coin, R2D_AUDIO_GROUP_UI, 0.8f);
        example->flash = 0.18f;
    }

    if (R2D_InputPressed(&example->input, "hit")) {
        R2D_PlaySfxGroup(example->hit, R2D_AUDIO_GROUP_SFX);
        example->flash = 0.18f;
    }

    if (R2D_InputPressed(&example->input, "jump")) {
        R2D_PlaySfxGroup(example->jump, R2D_AUDIO_GROUP_SFX);
        example->flash = 0.18f;
    }

    if (R2D_InputPressed(&example->input, "laser")) {
        R2D_PlaySfxGroup(example->laser, R2D_AUDIO_GROUP_AMBIENT);
        example->flash = 0.18f;
    }

    if (R2D_InputPressed(&example->input, "music") && example->music_loaded) {
        if (R2D_MusicIsPlaying(&example->music)) {
            R2D_MusicStop(&example->music);
        } else {
            R2D_MusicPlay(&example->music, true);
        }
    }

    if (R2D_InputPressed(&example->input, "fade")) {
        const float target = R2D_AudioGroupVolume(R2D_AUDIO_GROUP_MUSIC) > 0.5f ? 0.18f : 1.0f;

        R2D_AudioFadeGroup(R2D_AUDIO_GROUP_MUSIC, target, 0.8f);
    }

    if (example->flash > 0.0f) {
        example->flash -= dt;
    }
}

static void AudioExample_DrawPad(int x, const char *key, const char *label, bool active)
{
    R2D_UiStyle ui = R2D_DefaultUiStyle();

    R2D_DrawUiButton(R2D_Rect((float)x, 78.0f, 48.0f, 22.0f), key, false, active, ui);
    DrawText(label, x + 5, 108, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

static void AudioExample_Draw(void *user_data)
{
    const AudioExample *example = (const AudioExample *)user_data;
    const bool playing = example->music_loaded && R2D_MusicIsPlaying(&example->music);
    char text[96];

    DrawText("Audio example", 6, 14, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("SFX are tiny parameter presets;", 6, 34, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("music uses MIDI+SoundFont configs.", 6, 46, 8, R2D_ColorFromHex(0xf8f8f2ff));

    AudioExample_DrawPad(34, "Z", "coin", R2D_InputDown(&example->input, "coin"));
    AudioExample_DrawPad(92, "X", "hit", R2D_InputDown(&example->input, "hit"));
    AudioExample_DrawPad(150, "C", "jump", R2D_InputDown(&example->input, "jump"));
    AudioExample_DrawPad(208, "V", "laser", R2D_InputDown(&example->input, "laser"));

    R2D_DrawUiPanel(R2D_Rect(38.0f, 138.0f, 244.0f, 34.0f), R2D_DefaultUiStyle());
    DrawText(playing ? "P: stop music  F: fade music" : "P: play music  F: fade music", 50, 148, 10, playing ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0xffd166ff));
    snprintf(text, sizeof(text), "%.1fs / %.1fs", R2D_MusicPosition(&example->music), R2D_MusicLength(&example->music));
    DrawText(example->music_loaded ? text : "song not loaded", 178, 150, 8, R2D_ColorFromHex(0x8ecae6ff));
    snprintf(
        text,
        sizeof(text),
        "groups music %.2f  sfx %.2f  ui %.2f  ambient %.2f",
        R2D_AudioGroupVolume(R2D_AUDIO_GROUP_MUSIC),
        R2D_AudioGroupVolume(R2D_AUDIO_GROUP_SFX),
        R2D_AudioGroupVolume(R2D_AUDIO_GROUP_UI),
        R2D_AudioGroupVolume(R2D_AUDIO_GROUP_AMBIENT)
    );
    DrawText(text, 38, 177, 8, R2D_ColorFromHex(0xf8f8f2ff));

    if (example->flash > 0.0f) {
        DrawRectangleLinesEx(R2D_Rect(28.0f, 72.0f, 238.0f, 36.0f), 2.0f, R2D_ColorFromHex(0xffd166ff));
    }
}

static void AudioExample_Shutdown(void *user_data)
{
    AudioExample *example = (AudioExample *)user_data;

    R2D_MusicUnload(&example->music);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    AudioExample example = { 0 };

    config.title = "Retro2D Audio Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_AudioInit();
    R2D_Run(&context, (R2D_App) {
        AudioExample_Init,
        AudioExample_Update,
        AudioExample_Draw,
        AudioExample_Shutdown,
        &example
    });

    R2D_AudioClose();
    R2D_Close(&context);
    return 0;
}
