#include "r2d/r2d.h"

#include <stdio.h>

static R2D_CinematicStep *R2D_CinematicPushStep(R2D_Cinematic *cinematic, R2D_CinematicStepType type)
{
    R2D_CinematicStep *step;

    if (cinematic == 0 || cinematic->step_count >= R2D_CINEMATIC_MAX_STEPS) {
        return 0;
    }

    step = &cinematic->steps[cinematic->step_count++];
    *step = (R2D_CinematicStep) { 0 };
    step->type = type;
    step->audio_group = R2D_AUDIO_GROUP_SFX;

    return step;
}

static void R2D_CinematicAdvance(R2D_Cinematic *cinematic)
{
    if (cinematic == 0) {
        return;
    }

    cinematic->current_step++;
    cinematic->elapsed = 0.0f;
    cinematic->step_started = false;
    cinematic->dialog_visible = false;
    cinematic->dialog_text[0] = '\0';

    if (cinematic->current_step >= cinematic->step_count) {
        R2D_CinematicStop(cinematic);
    }
}

void R2D_CinematicInit(R2D_Cinematic *cinematic)
{
    if (cinematic == 0) {
        return;
    }

    *cinematic = (R2D_Cinematic) { 0 };
}

bool R2D_CinematicAddWait(R2D_Cinematic *cinematic, float duration)
{
    R2D_CinematicStep *step = R2D_CinematicPushStep(cinematic, R2D_CINEMATIC_STEP_WAIT);

    if (step == 0) {
        return false;
    }

    step->duration = duration;
    return true;
}

bool R2D_CinematicAddDialog(R2D_Cinematic *cinematic, const char *text, float duration)
{
    R2D_CinematicStep *step = R2D_CinematicPushStep(cinematic, R2D_CINEMATIC_STEP_DIALOG);

    if (step == 0) {
        return false;
    }

    if (text == 0) {
        text = "";
    }

    snprintf(step->text, sizeof(step->text), "%s", text);
    step->duration = duration;
    return true;
}

bool R2D_CinematicAddMoveCamera(R2D_Cinematic *cinematic, Vector2 target, float duration)
{
    R2D_CinematicStep *step = R2D_CinematicPushStep(cinematic, R2D_CINEMATIC_STEP_MOVE_CAMERA);

    if (step == 0) {
        return false;
    }

    step->target = target;
    step->duration = duration;
    return true;
}

bool R2D_CinematicAddSetFlag(R2D_Cinematic *cinematic, unsigned int *flags, unsigned int flag_mask, bool value)
{
    R2D_CinematicStep *step = R2D_CinematicPushStep(cinematic, R2D_CINEMATIC_STEP_SET_FLAG);

    if (step == 0) {
        return false;
    }

    step->flags = flags;
    step->flag_mask = flag_mask;
    step->flag_value = value;
    return true;
}

bool R2D_CinematicAddSfx(R2D_Cinematic *cinematic, R2D_Sfx sfx, R2D_AudioGroup group)
{
    R2D_CinematicStep *step = R2D_CinematicPushStep(cinematic, R2D_CINEMATIC_STEP_PLAY_SFX);

    if (step == 0) {
        return false;
    }

    step->sfx = sfx;
    step->audio_group = group;
    return true;
}

bool R2D_CinematicAddMusic(R2D_Cinematic *cinematic, R2D_Music *music, float volume, bool loop)
{
    R2D_CinematicStep *step = R2D_CinematicPushStep(cinematic, R2D_CINEMATIC_STEP_PLAY_MUSIC);

    if (step == 0) {
        return false;
    }

    step->music = music;
    step->music_volume = R2D_Clamp01(volume);
    step->music_loop = loop;
    return true;
}

void R2D_CinematicStart(R2D_Cinematic *cinematic, Vector2 camera_position)
{
    if (cinematic == 0 || cinematic->step_count <= 0) {
        return;
    }

    cinematic->current_step = 0;
    cinematic->elapsed = 0.0f;
    cinematic->camera_start = camera_position;
    cinematic->camera_position = camera_position;
    cinematic->step_started = false;
    cinematic->active = true;
    cinematic->input_locked = true;
    cinematic->dialog_visible = false;
    cinematic->dialog_text[0] = '\0';
}

void R2D_CinematicStop(R2D_Cinematic *cinematic)
{
    if (cinematic == 0) {
        return;
    }

    cinematic->active = false;
    cinematic->input_locked = false;
    cinematic->dialog_visible = false;
    cinematic->dialog_text[0] = '\0';
}

bool R2D_CinematicUpdate(R2D_Cinematic *cinematic, float dt, Vector2 camera_position)
{
    R2D_CinematicStep *step;

    if (cinematic == 0 || !cinematic->active) {
        return false;
    }

    if (cinematic->current_step < 0 || cinematic->current_step >= cinematic->step_count) {
        R2D_CinematicStop(cinematic);
        return false;
    }

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    step = &cinematic->steps[cinematic->current_step];
    if (!cinematic->step_started) {
        cinematic->step_started = true;
        cinematic->elapsed = 0.0f;

        if (step->type == R2D_CINEMATIC_STEP_DIALOG) {
            snprintf(cinematic->dialog_text, sizeof(cinematic->dialog_text), "%s", step->text);
            cinematic->dialog_visible = true;
        } else if (step->type == R2D_CINEMATIC_STEP_MOVE_CAMERA) {
            cinematic->camera_start = camera_position;
            cinematic->camera_position = camera_position;
        } else if (step->type == R2D_CINEMATIC_STEP_SET_FLAG) {
            if (step->flags != 0) {
                if (step->flag_value) {
                    *step->flags |= step->flag_mask;
                } else {
                    *step->flags &= ~step->flag_mask;
                }
            }
            R2D_CinematicAdvance(cinematic);
            return cinematic->active;
        } else if (step->type == R2D_CINEMATIC_STEP_PLAY_SFX) {
            R2D_PlaySfxGroup(step->sfx, step->audio_group);
            R2D_CinematicAdvance(cinematic);
            return cinematic->active;
        } else if (step->type == R2D_CINEMATIC_STEP_PLAY_MUSIC) {
            if (step->music != 0) {
                R2D_MusicSetVolume(step->music, step->music_volume);
                R2D_MusicPlay(step->music, step->music_loop);
            }
            R2D_CinematicAdvance(cinematic);
            return cinematic->active;
        }
    }

    cinematic->elapsed += dt;

    if (step->type == R2D_CINEMATIC_STEP_MOVE_CAMERA) {
        const float duration = step->duration > 0.0f ? step->duration : 0.0001f;
        float t = R2D_Clamp01(cinematic->elapsed / duration);

        t = t * t * (3.0f - 2.0f * t);
        cinematic->camera_position = (Vector2) {
            cinematic->camera_start.x + (step->target.x - cinematic->camera_start.x) * t,
            cinematic->camera_start.y + (step->target.y - cinematic->camera_start.y) * t
        };
    }

    if (step->duration <= 0.0f || cinematic->elapsed >= step->duration) {
        if (step->type == R2D_CINEMATIC_STEP_MOVE_CAMERA) {
            cinematic->camera_position = step->target;
        }

        R2D_CinematicAdvance(cinematic);
    }

    return cinematic->active;
}

bool R2D_CinematicActive(const R2D_Cinematic *cinematic)
{
    return cinematic != 0 && cinematic->active;
}

bool R2D_CinematicInputLocked(const R2D_Cinematic *cinematic)
{
    return cinematic != 0 && cinematic->input_locked;
}

Vector2 R2D_CinematicCameraPosition(const R2D_Cinematic *cinematic, Vector2 fallback)
{
    if (cinematic == 0 || !cinematic->active) {
        return fallback;
    }

    return cinematic->camera_position;
}

void R2D_CinematicDrawDialog(const R2D_Cinematic *cinematic, Rectangle rect, R2D_UiStyle style)
{
    if (cinematic == 0 || !cinematic->dialog_visible || cinematic->dialog_text[0] == '\0') {
        return;
    }

    R2D_DrawUiPanel(rect, style);
    R2D_DrawTextWrapped(
        cinematic->dialog_text,
        R2D_Rect(rect.x + 9.0f, rect.y + 8.0f, rect.width - 18.0f, rect.height - 12.0f),
        R2D_DefaultTextStyle(8, style.text)
    );
}
