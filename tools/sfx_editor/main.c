#include "r2d/r2d.h"

#define SFX_FIELD_COUNT 20
#define SFX_PRESET_COUNT 7

typedef struct SfxEditorPreset {
    const char *name;
    const char *path;
    R2D_Sfx (*fallback)(void);
} SfxEditorPreset;

typedef struct SfxEditor {
    R2D_Sfx sfx;
    int selected_field;
    int selected_preset;
    float message_timer;
    const char *message;
} SfxEditor;

static const SfxEditorPreset sfx_editor_presets[SFX_PRESET_COUNT] = {
    { "coin", "audio/sfx/coin.r2sfx", R2D_SfxCoin },
    { "hit", "audio/sfx/hit.r2sfx", R2D_SfxHit },
    { "jump", "audio/sfx/jump.r2sfx", R2D_SfxJump },
    { "laser", "audio/sfx/laser.r2sfx", R2D_SfxLaser },
    { "explosion", "audio/sfx/explosion.r2sfx", R2D_SfxExplosion },
    { "powerup", "audio/sfx/powerup.r2sfx", R2D_SfxPowerup },
    { "editor", "audio/sfx/editor.r2sfx", R2D_SfxPowerup }
};

static const char *SfxEditor_WaveformName(R2D_Waveform waveform)
{
    switch (waveform) {
        case R2D_WAVE_TRIANGLE:
            return "triangle";
        case R2D_WAVE_SAW:
            return "saw";
        case R2D_WAVE_RAMP:
            return "ramp";
        case R2D_WAVE_NOISE:
            return "noise";
        case R2D_WAVE_SINE:
            return "sine";
        case R2D_WAVE_SQUARE:
        default:
            return "square";
    }
}

static const char *SfxEditor_FilterName(R2D_Filter filter)
{
    switch (filter) {
        case R2D_FILTER_LOWPASS:
            return "lowpass";
        case R2D_FILTER_HIGHPASS:
            return "highpass";
        case R2D_FILTER_BANDPASS:
            return "bandpass";
        case R2D_FILTER_NONE:
        default:
            return "none";
    }
}

static float *SfxEditor_FieldValue(R2D_Sfx *sfx, int field)
{
    switch (field) {
        case 2:
            return &sfx->frequency;
        case 3:
            return &sfx->volume;
        case 4:
            return &sfx->pan;
        case 5:
            return &sfx->attack;
        case 6:
            return &sfx->decay;
        case 7:
            return &sfx->sustain;
        case 8:
            return &sfx->duration;
        case 9:
            return &sfx->release;
        case 10:
            return &sfx->pitch_slide;
        case 11:
            return &sfx->vibrato_depth;
        case 12:
            return &sfx->vibrato_rate;
        case 13:
            return &sfx->arpeggio_step_1;
        case 14:
            return &sfx->arpeggio_step_2;
        case 15:
            return &sfx->arpeggio_rate;
        case 16:
            return &sfx->duty;
        case 17:
            return &sfx->duty_slide;
        case 18:
            return &sfx->filter_cutoff;
        case 19:
            return &sfx->filter_resonance;
        default:
            return 0;
    }
}

static const char *SfxEditor_FieldName(int field)
{
    switch (field) {
        case 0:
            return "waveform";
        case 1:
            return "filter";
        case 2:
            return "frequency";
        case 3:
            return "volume";
        case 4:
            return "pan";
        case 5:
            return "attack";
        case 6:
            return "decay";
        case 7:
            return "sustain";
        case 8:
            return "duration";
        case 9:
            return "release";
        case 10:
            return "pitch_slide";
        case 11:
            return "vibrato_depth";
        case 12:
            return "vibrato_rate";
        case 13:
            return "arpeggio_step_1";
        case 14:
            return "arpeggio_step_2";
        case 15:
            return "arpeggio_rate";
        case 16:
            return "duty";
        case 17:
            return "duty_slide";
        case 18:
            return "filter_cutoff";
        case 19:
            return "filter_resonance";
        default:
            return "";
    }
}

static const char *SfxEditor_FieldText(R2D_Sfx *sfx, int field)
{
    if (field == 0) {
        return TextFormat("%s", SfxEditor_WaveformName(sfx->waveform));
    }

    if (field == 1) {
        return TextFormat("%s", SfxEditor_FilterName(sfx->filter));
    }

    return TextFormat("%.3f", *SfxEditor_FieldValue(sfx, field));
}

static float SfxEditor_FieldStep(int field)
{
    switch (field) {
        case 2:
        case 10:
        case 11:
        case 12:
        case 15:
        case 17:
        case 18:
            return 10.0f;
        case 5:
        case 6:
        case 8:
        case 9:
            return 0.005f;
        case 3:
        case 4:
        case 7:
        case 16:
        case 19:
            return 0.025f;
        case 13:
        case 14:
            return 1.0f;
        default:
            return 1.0f;
    }
}

static void SfxEditor_SetMessage(SfxEditor *editor, const char *message)
{
    editor->message = message;
    editor->message_timer = 1.2f;
}

static const SfxEditorPreset *SfxEditor_CurrentPreset(const SfxEditor *editor)
{
    return &sfx_editor_presets[editor->selected_preset];
}

static void SfxEditor_LoadPreset(SfxEditor *editor)
{
    const SfxEditorPreset *preset = SfxEditor_CurrentPreset(editor);

    editor->sfx = preset->fallback();

    if (R2D_LoadSfx(R2D_AssetPath(preset->path), &editor->sfx)) {
        SfxEditor_SetMessage(editor, "loaded");
    } else {
        SfxEditor_SetMessage(editor, "fallback");
    }
}

static void SfxEditor_SavePreset(SfxEditor *editor)
{
    const SfxEditorPreset *preset = SfxEditor_CurrentPreset(editor);

    SfxEditor_SetMessage(
        editor,
        R2D_SaveSfx(R2D_AssetPath(preset->path), editor->sfx) ? "saved" : "save failed"
    );
}

static void SfxEditor_SelectPreset(SfxEditor *editor, int direction)
{
    editor->selected_preset = (editor->selected_preset + direction + SFX_PRESET_COUNT) % SFX_PRESET_COUNT;
    SfxEditor_LoadPreset(editor);
}

static void SfxEditor_ResetPreset(SfxEditor *editor)
{
    editor->sfx = SfxEditor_CurrentPreset(editor)->fallback();
    SfxEditor_SetMessage(editor, "reset");
}

static void SfxEditor_Init(void *user_data)
{
    SfxEditor *editor = (SfxEditor *)user_data;

    editor->selected_preset = 0;
    editor->selected_field = 0;
    editor->message_timer = 0.0f;
    editor->message = "";
    SfxEditor_LoadPreset(editor);
}

static void SfxEditor_Update(float dt, void *user_data)
{
    SfxEditor *editor = (SfxEditor *)user_data;
    const bool fast = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    const int direction = IsKeyPressed(KEY_RIGHT) ? 1 : (IsKeyPressed(KEY_LEFT) ? -1 : 0);

    if (IsKeyPressed(KEY_DOWN)) {
        editor->selected_field = (editor->selected_field + 1) % SFX_FIELD_COUNT;
    }

    if (IsKeyPressed(KEY_UP)) {
        editor->selected_field = (editor->selected_field + SFX_FIELD_COUNT - 1) % SFX_FIELD_COUNT;
    }

    if (IsKeyPressed(KEY_Q)) {
        SfxEditor_SelectPreset(editor, -1);
    }

    if (IsKeyPressed(KEY_E)) {
        SfxEditor_SelectPreset(editor, 1);
    }

    if (direction != 0) {
        if (editor->selected_field == 0) {
            editor->sfx.waveform = (R2D_Waveform)((editor->sfx.waveform + direction + 6) % 6);
        } else if (editor->selected_field == 1) {
            editor->sfx.filter = (R2D_Filter)((editor->sfx.filter + direction + 4) % 4);
        } else {
            float *value = SfxEditor_FieldValue(&editor->sfx, editor->selected_field);
            *value += (float)direction * SfxEditor_FieldStep(editor->selected_field) * (fast ? 10.0f : 1.0f);
        }
    }

    if (IsKeyPressed(KEY_SPACE)) {
        R2D_PlaySfx(editor->sfx);
    }

    if (IsKeyPressed(KEY_S)) {
        SfxEditor_SavePreset(editor);
    }

    if (IsKeyPressed(KEY_L)) {
        SfxEditor_LoadPreset(editor);
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        SfxEditor_ResetPreset(editor);
    }

    if (editor->message_timer > 0.0f) {
        editor->message_timer -= dt;
    }
}

static void SfxEditor_Draw(void *user_data)
{
    SfxEditor *editor = (SfxEditor *)user_data;
    const SfxEditorPreset *preset = SfxEditor_CurrentPreset(editor);

    DrawText("R2D SFX EDITOR", 8, 8, 10, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText(TextFormat("Q/E preset: %s", preset->name), 8, 22, 8, R2D_ColorFromHex(0x8be9fdff));
    DrawText("UP/DOWN field  LEFT/RIGHT value  SHIFT fast", 8, 34, 8, R2D_ColorFromHex(0x8be9fdff));
    DrawText("SPACE play  S save  L load  BACK reset", 8, 46, 8, R2D_ColorFromHex(0xffb86cff));

    for (int i = 0; i < SFX_FIELD_COUNT; ++i) {
        const int y = 60 + i * 6;
        const Color color = i == editor->selected_field ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0xd7d7e0ff);

        DrawText(TextFormat("%c", i == editor->selected_field ? '>' : ' '), 8, y, 6, color);
        DrawText(SfxEditor_FieldName(i), 18, y, 6, color);
        DrawText(SfxEditor_FieldText(&editor->sfx, i), 132, y, 6, color);
    }

    DrawText(TextFormat("assets/%s", preset->path), 8, 190, 7, R2D_ColorFromHex(0x6272a4ff));

    if (editor->message_timer > 0.0f) {
        DrawText(editor->message, 236, 190, 7, R2D_ColorFromHex(0xf1fa8cff));
    }
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    SfxEditor editor = { 0 };

    config.title = "Retro2DFramework SFX Editor";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_AudioInit();

    R2D_Run(&context, (R2D_App) {
        SfxEditor_Init,
        SfxEditor_Update,
        SfxEditor_Draw,
        0,
        &editor
    });

    R2D_AudioClose();
    R2D_Close(&context);
    return 0;
}
