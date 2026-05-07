#include "r2d/r2d.h"

#include <math.h>

#define SFX_FIELD_COUNT 21
#define SFX_PRESET_COUNT 7

typedef struct SfxEditorPreset {
    const char *name;
    const char *path;
    R2D_Sfx (*fallback)(void);
} SfxEditorPreset;

typedef struct SfxEditorRange {
    float min;
    float max;
    bool centered;
} SfxEditorRange;

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

static float SfxEditor_Clamp(float value, float min, float max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

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
            return &sfx->filter_cutoff_slide;
        case 20:
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
            return "freq";
        case 3:
            return "vol";
        case 4:
            return "pan";
        case 5:
            return "atk";
        case 6:
            return "dec";
        case 7:
            return "sus";
        case 8:
            return "dur";
        case 9:
            return "rel";
        case 10:
            return "pitch";
        case 11:
            return "vib_dep";
        case 12:
            return "vib_rate";
        case 13:
            return "arp_1";
        case 14:
            return "arp_2";
        case 15:
            return "arp_rate";
        case 16:
            return "duty";
        case 17:
            return "duty_sl";
        case 18:
            return "cutoff";
        case 19:
            return "cut_sl";
        case 20:
            return "reson";
        default:
            return "";
    }
}

static const char *SfxEditor_FieldGroup(int field)
{
    if (field <= 4) {
        return "OSC";
    }

    if (field <= 9) {
        return "ENV";
    }

    if (field <= 17) {
        return "MOD";
    }

    return "FLT";
}

static Color SfxEditor_GroupColor(int field)
{
    if (field <= 4) {
        return R2D_ColorFromHex(0x8be9fdff);
    }

    if (field <= 9) {
        return R2D_ColorFromHex(0xf1fa8cff);
    }

    if (field <= 17) {
        return R2D_ColorFromHex(0xffb86cff);
    }

    return R2D_ColorFromHex(0x50fa7bff);
}

static SfxEditorRange SfxEditor_FieldRange(int field)
{
    switch (field) {
        case 2:
            return (SfxEditorRange) { 20.0f, 4000.0f, false };
        case 3:
        case 4:
        case 7:
        case 16:
        case 19:
            return (SfxEditorRange) { -20000.0f, 20000.0f, true };
        case 20:
            return (SfxEditorRange) { 0.0f, 1.0f, false };
        case 5:
        case 6:
        case 8:
        case 9:
            return (SfxEditorRange) { 0.0f, 1.0f, false };
        case 10:
            return (SfxEditorRange) { -8000.0f, 8000.0f, true };
        case 11:
            return (SfxEditorRange) { 0.0f, 1000.0f, false };
        case 12:
            return (SfxEditorRange) { 0.0f, 100.0f, false };
        case 13:
        case 14:
            return (SfxEditorRange) { -24.0f, 24.0f, true };
        case 15:
            return (SfxEditorRange) { 0.0f, 80.0f, false };
        case 17:
            return (SfxEditorRange) { -5.0f, 5.0f, true };
        case 18:
            return (SfxEditorRange) { 20.0f, 20000.0f, false };
        default:
            return (SfxEditorRange) { 0.0f, 1.0f, false };
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

    {
        const float *value = SfxEditor_FieldValue(sfx, field);
        const float amount = fabsf(*value);

        if (amount >= 100.0f) {
            return TextFormat("%.0f", *value);
        }

        if (amount >= 1.0f) {
            return TextFormat("%.2f", *value);
        }

        return TextFormat("%.3f", *value);
    }
}

static float SfxEditor_FieldStep(int field)
{
    switch (field) {
        case 2:
        case 10:
        case 11:
        case 12:
        case 15:
        case 18:
        case 19:
            return 10.0f;
        case 17:
            return 0.1f;
        case 5:
        case 6:
        case 8:
        case 9:
            return 0.005f;
        case 3:
        case 4:
        case 7:
        case 16:
        case 20:
            return 0.025f;
        case 13:
        case 14:
            return 1.0f;
        default:
            return 1.0f;
    }
}

static void SfxEditor_ClampField(R2D_Sfx *sfx, int field)
{
    float *value = SfxEditor_FieldValue(sfx, field);
    const SfxEditorRange range = SfxEditor_FieldRange(field);

    if (value != 0) {
        *value = SfxEditor_Clamp(*value, range.min, range.max);
    }
}

static void SfxEditor_SetMessage(SfxEditor *editor, const char *message)
{
    editor->message = message;
    editor->message_timer = 1.2f;
}

static float SfxEditor_TotalDuration(const R2D_Sfx *sfx)
{
    return SfxEditor_Clamp(sfx->attack, 0.0f, 1.0f) +
        SfxEditor_Clamp(sfx->decay, 0.0f, 1.0f) +
        SfxEditor_Clamp(sfx->duration, 0.0f, 1.0f) +
        SfxEditor_Clamp(sfx->release, 0.0f, 1.0f);
}

static float SfxEditor_EnvelopeAt(const R2D_Sfx *sfx, float elapsed)
{
    float t = elapsed;
    const float attack = SfxEditor_Clamp(sfx->attack, 0.0f, 1.0f);
    const float decay = SfxEditor_Clamp(sfx->decay, 0.0f, 1.0f);
    const float sustain = SfxEditor_Clamp(sfx->sustain, 0.0f, 1.0f);
    const float duration = SfxEditor_Clamp(sfx->duration, 0.0f, 1.0f);
    const float release = SfxEditor_Clamp(sfx->release, 0.0f, 1.0f);

    if (attack > 0.0f && t < attack) {
        return t / attack;
    }

    t -= attack;

    if (decay > 0.0f && t < decay) {
        return 1.0f + (sustain - 1.0f) * (t / decay);
    }

    t -= decay;

    if (t < duration) {
        return sustain;
    }

    t -= duration;

    if (release > 0.0f && t < release) {
        return sustain * (1.0f - t / release);
    }

    return 0.0f;
}

static float SfxEditor_RandomRange(float amount)
{
    return ((float)GetRandomValue(-1000, 1000) / 1000.0f) * amount;
}

static void SfxEditor_Mutate(SfxEditor *editor)
{
    R2D_Sfx *sfx = &editor->sfx;

    sfx->frequency += SfxEditor_RandomRange(90.0f);
    sfx->volume += SfxEditor_RandomRange(0.05f);
    sfx->attack += SfxEditor_RandomRange(0.01f);
    sfx->decay += SfxEditor_RandomRange(0.02f);
    sfx->duration += SfxEditor_RandomRange(0.03f);
    sfx->release += SfxEditor_RandomRange(0.03f);
    sfx->pitch_slide += SfxEditor_RandomRange(650.0f);
    sfx->vibrato_depth += SfxEditor_RandomRange(18.0f);
    sfx->vibrato_rate += SfxEditor_RandomRange(6.0f);
    sfx->duty += SfxEditor_RandomRange(0.06f);
    sfx->duty_slide += SfxEditor_RandomRange(0.35f);
    sfx->filter_cutoff += SfxEditor_RandomRange(900.0f);
    sfx->filter_cutoff_slide += SfxEditor_RandomRange(1800.0f);
    sfx->filter_resonance += SfxEditor_RandomRange(0.08f);

    for (int i = 0; i < SFX_FIELD_COUNT; ++i) {
        SfxEditor_ClampField(sfx, i);
    }

    SfxEditor_SetMessage(editor, "mutated");
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
            SfxEditor_ClampField(&editor->sfx, editor->selected_field);
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

    if (IsKeyPressed(KEY_R)) {
        SfxEditor_Mutate(editor);
    }

    if (editor->message_timer > 0.0f) {
        editor->message_timer -= dt;
    }
}

static void SfxEditor_DrawCurve(
    Rectangle area,
    const R2D_Sfx *sfx,
    Color color,
    int mode
)
{
    const float total = fmaxf(SfxEditor_TotalDuration(sfx), 0.001f);
    Vector2 previous = { area.x, area.y + area.height };

    for (int i = 0; i < (int)area.width; ++i) {
        const float amount = (float)i / fmaxf(area.width - 1.0f, 1.0f);
        const float elapsed = total * amount;
        float value = 0.0f;

        if (mode == 0) {
            value = SfxEditor_EnvelopeAt(sfx, elapsed);
        } else if (mode == 1) {
            const float frequency = sfx->frequency + sfx->pitch_slide * elapsed;
            value = SfxEditor_Clamp((frequency - 20.0f) / 4000.0f, 0.0f, 1.0f);
        } else {
            const float cutoff = sfx->filter_cutoff + sfx->filter_cutoff_slide * elapsed;
            value = SfxEditor_Clamp((cutoff - 20.0f) / 8000.0f, 0.0f, 1.0f);
        }

        {
            const Vector2 point = {
                area.x + (float)i,
                area.y + area.height - value * area.height
            };

            if (i > 0) {
                DrawLineV(previous, point, color);
            }

            previous = point;
        }
    }
}

static void SfxEditor_DrawPreview(const R2D_Sfx *sfx)
{
    const Rectangle area = { 8.0f, 166.0f, 304.0f, 17.0f };

    DrawRectangleLinesEx(area, 1.0f, R2D_ColorFromHex(0x303040ff));
    SfxEditor_DrawCurve(area, sfx, R2D_ColorFromHex(0xf1fa8cff), 0);
    SfxEditor_DrawCurve(area, sfx, R2D_ColorFromHex(0xff79c6ff), 1);
    SfxEditor_DrawCurve(area, sfx, R2D_ColorFromHex(0x50fa7bff), 2);
}

static void SfxEditor_Draw(void *user_data)
{
    SfxEditor *editor = (SfxEditor *)user_data;
    const SfxEditorPreset *preset = SfxEditor_CurrentPreset(editor);
    const int row_height = 14;

    DrawText(TextFormat("Q/E preset: %s", preset->name), 8, 8, 8, R2D_ColorFromHex(0x8be9fdff));
    DrawText("UP/DOWN field  LEFT/RIGHT value  SHIFT fast", 8, 20, 8, R2D_ColorFromHex(0x8be9fdff));
    DrawText("SPACE play  R mutate  S save  L load  BACK reset", 8, 32, 8, R2D_ColorFromHex(0xffb86cff));

    for (int i = 0; i < SFX_FIELD_COUNT; ++i) {
        const int column = i / 7;
        const int row = i % 7;
        const int x = 8 + column * 104;
        const int y = 50 + row * row_height;
        const Color color = i == editor->selected_field ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0xd7d7e0ff);
        const Color group_color = SfxEditor_GroupColor(i);
        float *value = SfxEditor_FieldValue(&editor->sfx, i);

        DrawText(TextFormat("%c", i == editor->selected_field ? '>' : ' '), x, y, 7, color);
        DrawText(SfxEditor_FieldGroup(i), x + 10, y, 7, group_color);
        DrawText(SfxEditor_FieldName(i), x + 34, y, 7, color);
        DrawText(SfxEditor_FieldText(&editor->sfx, i), x + 72, y, 7, color);

        if (value != 0) {
            const SfxEditorRange range = SfxEditor_FieldRange(i);
            const Rectangle track = { (float)x + 34.0f, (float)y + 9.0f, 66.0f, 3.0f };
            const float normalized = SfxEditor_Clamp((*value - range.min) / (range.max - range.min), 0.0f, 1.0f);

            DrawRectangleRec(track, R2D_ColorFromHex(0x303040ff));

            if (range.centered) {
                const float center = track.x + track.width * 0.5f;
                const float knob = track.x + track.width * normalized;
                const float x = knob < center ? knob : center;
                const float width = knob < center ? center - knob : knob - center;

                DrawRectangleRec((Rectangle) { x, track.y, width, track.height }, group_color);
                DrawRectangle((int)center, (int)track.y - 1, 1, 5, R2D_ColorFromHex(0xf8f8f2ff));
            } else {
                DrawRectangleRec((Rectangle) { track.x, track.y, track.width * normalized, track.height }, group_color);
            }
        }
    }

    SfxEditor_DrawPreview(&editor->sfx);

    DrawText(TextFormat("assets/%s", preset->path), 8, 188, 7, R2D_ColorFromHex(0x6272a4ff));

    if (editor->message_timer > 0.0f) {
        DrawText(editor->message, 236, 188, 7, R2D_ColorFromHex(0xf1fa8cff));
    }
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Crt crt = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    SfxEditor editor = { 0 };

    config.title = "Retro2DFramework SFX Editor";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_AudioInit();
    R2D_CrtInit(&crt);
    R2D_SetCrt(&context, &crt);

    R2D_Run(&context, (R2D_App) {
        SfxEditor_Init,
        SfxEditor_Update,
        SfxEditor_Draw,
        0,
        &editor
    });

    R2D_CrtClose(&crt);
    R2D_AudioClose();
    R2D_Close(&context);
    return 0;
}
