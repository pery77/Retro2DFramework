#include "r2d/r2d.h"

#include <math.h>

#define SFX_FIELD_COUNT 21
#define SFX_PRESET_COUNT 7
#define SFX_HISTORY_COUNT 32

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
    R2D_Sfx undo_history[SFX_HISTORY_COUNT];
    R2D_Sfx redo_history[SFX_HISTORY_COUNT];
    R2D_Context *context;
    int selected_field;
    int selected_preset;
    int mouse_field;
    int undo_count;
    int redo_count;
    float message_timer;
    const char *message;
    bool auto_play;
    bool dirty;
    bool mouse_editing;
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

static const char *SfxEditor_PresetTabName(int preset)
{
    switch (preset) {
        case 0:
            return "coin";
        case 1:
            return "hit";
        case 2:
            return "jump";
        case 3:
            return "laser";
        case 4:
            return "boom";
        case 5:
            return "power";
        case 6:
            return "edit";
        default:
            return "";
    }
}

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
            return (SfxEditorRange) { 0.0f, 1.0f, false };
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
        case 19:
            return (SfxEditorRange) { -20000.0f, 20000.0f, true };
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

static bool SfxEditor_FieldIsNumeric(int field)
{
    return field >= 2 && field < SFX_FIELD_COUNT;
}

static int SfxEditor_FieldColumn(int field)
{
    return field / 7;
}

static int SfxEditor_FieldRow(int field)
{
    return field % 7;
}

static int SfxEditor_FieldX(int field)
{
    return 8 + SfxEditor_FieldColumn(field) * 104;
}

static int SfxEditor_FieldY(int field)
{
    return 60 + SfxEditor_FieldRow(field) * 14;
}

static Rectangle SfxEditor_FieldHitRect(int field)
{
    return (Rectangle) {
        (float)SfxEditor_FieldX(field),
        (float)SfxEditor_FieldY(field),
        100.0f,
        13.0f
    };
}

static Rectangle SfxEditor_FieldTrackRect(int field)
{
    return (Rectangle) {
        (float)SfxEditor_FieldX(field) + 34.0f,
        (float)SfxEditor_FieldY(field) + 9.0f,
        66.0f,
        3.0f
    };
}

static Rectangle SfxEditor_PresetTabRect(int preset)
{
    return (Rectangle) {
        8.0f + (float)preset * 44.0f,
        7.0f,
        40.0f,
        10.0f
    };
}

static void SfxEditor_SetFieldFromMouse(SfxEditor *editor, int field, Vector2 mouse)
{
    float *value = SfxEditor_FieldValue(&editor->sfx, field);
    const SfxEditorRange range = SfxEditor_FieldRange(field);
    const Rectangle track = SfxEditor_FieldTrackRect(field);
    const float normalized = SfxEditor_Clamp((mouse.x - track.x) / track.width, 0.0f, 1.0f);

    if (value != 0) {
        *value = range.min + (range.max - range.min) * normalized;
        SfxEditor_ClampField(&editor->sfx, field);
        editor->dirty = true;
    }
}

static void SfxEditor_SetMessage(SfxEditor *editor, const char *message)
{
    editor->message = message;
    editor->message_timer = 1.2f;
}

static void SfxEditor_PlayIfNeeded(SfxEditor *editor)
{
    if (editor->auto_play) {
        R2D_PlaySfx(editor->sfx);
    }
}

static void SfxEditor_ClearHistory(SfxEditor *editor)
{
    editor->undo_count = 0;
    editor->redo_count = 0;
}

static void SfxEditor_PushHistory(R2D_Sfx *history, int *count, R2D_Sfx sfx)
{
    if (*count >= SFX_HISTORY_COUNT) {
        for (int i = 1; i < SFX_HISTORY_COUNT; ++i) {
            history[i - 1] = history[i];
        }

        *count = SFX_HISTORY_COUNT - 1;
    }

    history[*count] = sfx;
    *count += 1;
}

static void SfxEditor_PushUndo(SfxEditor *editor)
{
    SfxEditor_PushHistory(editor->undo_history, &editor->undo_count, editor->sfx);
    editor->redo_count = 0;
}

static void SfxEditor_Undo(SfxEditor *editor)
{
    if (editor->undo_count <= 0) {
        SfxEditor_SetMessage(editor, "no undo");
        return;
    }

    SfxEditor_PushHistory(editor->redo_history, &editor->redo_count, editor->sfx);
    editor->undo_count -= 1;
    editor->sfx = editor->undo_history[editor->undo_count];
    editor->dirty = true;
    SfxEditor_SetMessage(editor, "undo");
    SfxEditor_PlayIfNeeded(editor);
}

static void SfxEditor_Redo(SfxEditor *editor)
{
    if (editor->redo_count <= 0) {
        SfxEditor_SetMessage(editor, "no redo");
        return;
    }

    SfxEditor_PushHistory(editor->undo_history, &editor->undo_count, editor->sfx);
    editor->redo_count -= 1;
    editor->sfx = editor->redo_history[editor->redo_count];
    editor->dirty = true;
    SfxEditor_SetMessage(editor, "redo");
    SfxEditor_PlayIfNeeded(editor);
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

    SfxEditor_PushUndo(editor);

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
    editor->dirty = true;
    SfxEditor_PlayIfNeeded(editor);
}

static const SfxEditorPreset *SfxEditor_CurrentPreset(const SfxEditor *editor)
{
    return &sfx_editor_presets[editor->selected_preset];
}

static bool SfxEditor_CanLeavePreset(SfxEditor *editor)
{
    if (!editor->dirty) {
        return true;
    }

    SfxEditor_SetMessage(editor, "save/clone/load first");
    return false;
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

    editor->dirty = false;
}

static void SfxEditor_SavePreset(SfxEditor *editor)
{
    const SfxEditorPreset *preset = SfxEditor_CurrentPreset(editor);

    if (R2D_SaveSfx(R2D_AssetPath(preset->path), editor->sfx)) {
        editor->dirty = false;
        SfxEditor_SetMessage(editor, "saved");
    } else {
        SfxEditor_SetMessage(editor, "save failed");
    }
}

static void SfxEditor_SelectPreset(SfxEditor *editor, int direction)
{
    if (!SfxEditor_CanLeavePreset(editor)) {
        return;
    }

    editor->selected_preset = (editor->selected_preset + direction + SFX_PRESET_COUNT) % SFX_PRESET_COUNT;
    SfxEditor_LoadPreset(editor);
    SfxEditor_ClearHistory(editor);
    SfxEditor_PlayIfNeeded(editor);
}

static void SfxEditor_SelectPresetIndex(SfxEditor *editor, int preset)
{
    if (preset == editor->selected_preset) {
        return;
    }

    if (!SfxEditor_CanLeavePreset(editor)) {
        return;
    }

    editor->selected_preset = preset;
    SfxEditor_LoadPreset(editor);
    SfxEditor_ClearHistory(editor);
    SfxEditor_PlayIfNeeded(editor);
}

static void SfxEditor_CloneToEditorPreset(SfxEditor *editor)
{
    const int editor_preset = SFX_PRESET_COUNT - 1;
    const SfxEditorPreset *preset = &sfx_editor_presets[editor_preset];

    editor->selected_preset = editor_preset;

    if (R2D_SaveSfx(R2D_AssetPath(preset->path), editor->sfx)) {
        editor->dirty = false;
        SfxEditor_ClearHistory(editor);
        SfxEditor_SetMessage(editor, "cloned");
    } else {
        editor->dirty = true;
        SfxEditor_SetMessage(editor, "clone failed");
    }
}

static void SfxEditor_ResetPreset(SfxEditor *editor)
{
    SfxEditor_PushUndo(editor);
    editor->sfx = SfxEditor_CurrentPreset(editor)->fallback();
    editor->dirty = true;
    SfxEditor_SetMessage(editor, "reset");
    SfxEditor_PlayIfNeeded(editor);
}

static void SfxEditor_Init(void *user_data)
{
    SfxEditor *editor = (SfxEditor *)user_data;

    editor->selected_preset = 0;
    editor->selected_field = 0;
    editor->message_timer = 0.0f;
    editor->message = "";
    editor->auto_play = false;
    editor->mouse_field = -1;
    editor->mouse_editing = false;
    SfxEditor_LoadPreset(editor);
    SfxEditor_ClearHistory(editor);
}

static void SfxEditor_HandleMouse(SfxEditor *editor)
{
    const Vector2 mouse = R2D_MouseVirtualPosition(editor->context);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < SFX_PRESET_COUNT; ++i) {
            if (CheckCollisionPointRec(mouse, SfxEditor_PresetTabRect(i))) {
                SfxEditor_SelectPresetIndex(editor, i);
                return;
            }
        }

        for (int i = 0; i < SFX_FIELD_COUNT; ++i) {
            if (CheckCollisionPointRec(mouse, SfxEditor_FieldHitRect(i))) {
                editor->selected_field = i;
                editor->mouse_field = i;

                if (SfxEditor_FieldIsNumeric(i)) {
                    SfxEditor_PushUndo(editor);
                    SfxEditor_SetFieldFromMouse(editor, i, mouse);
                    editor->mouse_editing = true;
                } else {
                    SfxEditor_PushUndo(editor);

                    if (i == 0) {
                        editor->sfx.waveform = (R2D_Waveform)((editor->sfx.waveform + 1) % 6);
                    } else {
                        editor->sfx.filter = (R2D_Filter)((editor->sfx.filter + 1) % 4);
                    }

                    editor->dirty = true;
                    SfxEditor_PlayIfNeeded(editor);
                }

                break;
            }
        }
    }

    if (editor->mouse_editing && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        SfxEditor_SetFieldFromMouse(editor, editor->mouse_field, mouse);
    }

    if (editor->mouse_editing && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        editor->mouse_editing = false;
        SfxEditor_PlayIfNeeded(editor);
    }
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

    SfxEditor_HandleMouse(editor);

    if (direction != 0) {
        SfxEditor_PushUndo(editor);

        if (editor->selected_field == 0) {
            editor->sfx.waveform = (R2D_Waveform)((editor->sfx.waveform + direction + 6) % 6);
        } else if (editor->selected_field == 1) {
            editor->sfx.filter = (R2D_Filter)((editor->sfx.filter + direction + 4) % 4);
        } else {
            float *value = SfxEditor_FieldValue(&editor->sfx, editor->selected_field);
            *value += (float)direction * SfxEditor_FieldStep(editor->selected_field) * (fast ? 10.0f : 1.0f);
            SfxEditor_ClampField(&editor->sfx, editor->selected_field);
        }

        editor->dirty = true;
        SfxEditor_PlayIfNeeded(editor);
    }

    if (IsKeyPressed(KEY_SPACE)) {
        R2D_PlaySfx(editor->sfx);
    }

    if (IsKeyPressed(KEY_S)) {
        SfxEditor_SavePreset(editor);
    }

    if (IsKeyPressed(KEY_L)) {
        SfxEditor_PushUndo(editor);
        SfxEditor_LoadPreset(editor);
        SfxEditor_PlayIfNeeded(editor);
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        SfxEditor_ResetPreset(editor);
    }

    if (IsKeyPressed(KEY_R)) {
        SfxEditor_Mutate(editor);
    }

    if (IsKeyPressed(KEY_A)) {
        editor->auto_play = !editor->auto_play;
        SfxEditor_SetMessage(editor, editor->auto_play ? "auto on" : "auto off");
    }

    if (IsKeyPressed(KEY_C)) {
        SfxEditor_CloneToEditorPreset(editor);
    }

    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Z)) {
        SfxEditor_Undo(editor);
    }

    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Y)) {
        SfxEditor_Redo(editor);
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

static void SfxEditor_DrawPresetTabs(const SfxEditor *editor)
{
    for (int i = 0; i < SFX_PRESET_COUNT; ++i) {
        const Rectangle rect = SfxEditor_PresetTabRect(i);
        const bool active = i == editor->selected_preset;
        const Color border = active ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0x6272a4ff);
        const Color text = active ? R2D_ColorFromHex(0xf8f8f2ff) : R2D_ColorFromHex(0x8be9fdff);

        if (active) {
            DrawRectangleRec(rect, R2D_ColorFromHex(0x243a34ff));
        }

        DrawRectangleLinesEx(rect, 1.0f, border);
        DrawText(
            TextFormat("%s%s", SfxEditor_PresetTabName(i), active && editor->dirty ? "*" : ""),
            (int)rect.x + 3,
            (int)rect.y + 2,
            6,
            text
        );
    }
}

static void SfxEditor_Draw(void *user_data)
{
    SfxEditor *editor = (SfxEditor *)user_data;
    const SfxEditorPreset *preset = SfxEditor_CurrentPreset(editor);

    SfxEditor_DrawPresetTabs(editor);
    DrawText("UP/DOWN field  LEFT/RIGHT value  SHIFT fast", 8, 20, 8, R2D_ColorFromHex(0x8be9fdff));
    DrawText(
        TextFormat("SPACE play  A auto:%s  R mut  S save  L load", editor->auto_play ? "on" : "off"),
        8,
        32,
        8,
        R2D_ColorFromHex(0xffb86cff)
    );
    DrawText("C clone  BACK reset  Ctrl+Z/Y undo/redo", 8, 44, 8, R2D_ColorFromHex(0xffb86cff));

    for (int i = 0; i < SFX_FIELD_COUNT; ++i) {
        const int x = SfxEditor_FieldX(i);
        const int y = SfxEditor_FieldY(i);
        const Color color = i == editor->selected_field ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0xd7d7e0ff);
        const Color group_color = SfxEditor_GroupColor(i);
        float *value = SfxEditor_FieldValue(&editor->sfx, i);

        DrawText(TextFormat("%c", i == editor->selected_field ? '>' : ' '), x, y, 7, color);
        DrawText(SfxEditor_FieldGroup(i), x + 10, y, 7, group_color);
        DrawText(SfxEditor_FieldName(i), x + 34, y, 7, color);
        DrawText(SfxEditor_FieldText(&editor->sfx, i), x + 72, y, 7, color);

        if (value != 0) {
            const SfxEditorRange range = SfxEditor_FieldRange(i);
            const Rectangle track = SfxEditor_FieldTrackRect(i);
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
    editor.context = &context;

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
