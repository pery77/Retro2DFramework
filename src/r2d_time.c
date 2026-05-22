#include "r2d/r2d.h"

#include <string.h>

float R2D_Clamp(float value, float min, float max)
{
    if (min > max) {
        const float swap = min;
        min = max;
        max = swap;
    }

    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

float R2D_Clamp01(float value)
{
    return R2D_Clamp(value, 0.0f, 1.0f);
}

float R2D_Lerp(float a, float b, float t)
{
    return a + (b - a) * R2D_Clamp01(t);
}

Color R2D_LerpColor(Color a, Color b, float t)
{
    t = R2D_Clamp01(t);

    return (Color) {
        (unsigned char)R2D_Lerp((float)a.r, (float)b.r, t),
        (unsigned char)R2D_Lerp((float)a.g, (float)b.g, t),
        (unsigned char)R2D_Lerp((float)a.b, (float)b.b, t),
        (unsigned char)R2D_Lerp((float)a.a, (float)b.a, t)
    };
}

float R2D_EaseValue(R2D_Ease ease, float t)
{
    t = R2D_Clamp01(t);

    switch (ease) {
    case R2D_EASE_IN:
        return t * t;
    case R2D_EASE_OUT:
        return 1.0f - (1.0f - t) * (1.0f - t);
    case R2D_EASE_IN_OUT:
        return t < 0.5f ? 2.0f * t * t : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
    case R2D_EASE_LINEAR:
    default:
        return t;
    }
}

void R2D_TimerSystemInit(R2D_TimerSystem *system)
{
    if (system == 0) {
        return;
    }

    memset(system, 0, sizeof(*system));
}

void R2D_TimerSystemClear(R2D_TimerSystem *system)
{
    R2D_TimerSystemInit(system);
}

static int R2D_TimerAdd(R2D_TimerSystem *system, float duration, int repeat_count, R2D_TimerCallback callback, void *user_data)
{
    if (system == 0 || system->count >= R2D_TIMER_MAX || duration <= 0.0f) {
        return -1;
    }

    for (int i = 0; i < R2D_TIMER_MAX; ++i) {
        R2D_Timer *timer = &system->timers[i];

        if (timer->active) {
            continue;
        }

        *timer = (R2D_Timer) {
            duration,
            0.0f,
            repeat_count,
            0,
            true,
            callback,
            user_data
        };
        system->count++;
        return i;
    }

    return -1;
}

int R2D_TimerAfter(R2D_TimerSystem *system, float delay, R2D_TimerCallback callback, void *user_data)
{
    return R2D_TimerAdd(system, delay, 1, callback, user_data);
}

int R2D_TimerEvery(R2D_TimerSystem *system, float interval, int repeat_count, R2D_TimerCallback callback, void *user_data)
{
    return R2D_TimerAdd(system, interval, repeat_count, callback, user_data);
}

bool R2D_TimerCancel(R2D_TimerSystem *system, int timer_index)
{
    if (system == 0 || timer_index < 0 || timer_index >= R2D_TIMER_MAX || !system->timers[timer_index].active) {
        return false;
    }

    system->timers[timer_index] = (R2D_Timer) { 0 };
    system->count--;
    return true;
}

void R2D_TimerSystemUpdate(R2D_TimerSystem *system, float dt)
{
    if (system == 0) {
        return;
    }

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    for (int i = 0; i < R2D_TIMER_MAX; ++i) {
        R2D_Timer *timer = &system->timers[i];

        if (!timer->active) {
            continue;
        }

        timer->elapsed += dt;

        while (timer->active && timer->elapsed >= timer->duration) {
            timer->elapsed -= timer->duration;
            timer->fired_count++;

            if (timer->callback != 0) {
                timer->callback(timer->user_data);
            }

            if (timer->repeat_count > 0 && timer->fired_count >= timer->repeat_count) {
                R2D_TimerCancel(system, i);
                break;
            }
        }
    }
}

int R2D_TimerSystemActiveCount(const R2D_TimerSystem *system)
{
    return system != 0 ? system->count : 0;
}

void R2D_TweenSystemInit(R2D_TweenSystem *system)
{
    if (system == 0) {
        return;
    }

    memset(system, 0, sizeof(*system));
}

void R2D_TweenSystemClear(R2D_TweenSystem *system)
{
    R2D_TweenSystemInit(system);
}

int R2D_TweenFloat(R2D_TweenSystem *system, float *target, float end, float duration, R2D_Ease ease)
{
    if (system == 0 || target == 0 || system->count >= R2D_TWEEN_MAX || duration <= 0.0f) {
        return -1;
    }

    for (int i = 0; i < R2D_TWEEN_MAX; ++i) {
        R2D_Tween *tween = &system->tweens[i];

        if (tween->active) {
            continue;
        }

        *tween = (R2D_Tween) {
            target,
            *target,
            end,
            duration,
            0.0f,
            ease,
            true
        };
        system->count++;
        return i;
    }

    return -1;
}

bool R2D_TweenCancel(R2D_TweenSystem *system, int tween_index)
{
    if (system == 0 || tween_index < 0 || tween_index >= R2D_TWEEN_MAX || !system->tweens[tween_index].active) {
        return false;
    }

    system->tweens[tween_index] = (R2D_Tween) { 0 };
    system->count--;
    return true;
}

void R2D_TweenSystemUpdate(R2D_TweenSystem *system, float dt)
{
    if (system == 0) {
        return;
    }

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    for (int i = 0; i < R2D_TWEEN_MAX; ++i) {
        R2D_Tween *tween = &system->tweens[i];
        float t;

        if (!tween->active) {
            continue;
        }

        tween->elapsed += dt;
        t = tween->duration > 0.0f ? tween->elapsed / tween->duration : 1.0f;
        *tween->target = R2D_Lerp(tween->start, tween->end, R2D_EaseValue(tween->ease, t));

        if (t >= 1.0f) {
            *tween->target = tween->end;
            R2D_TweenCancel(system, i);
        }
    }
}

int R2D_TweenSystemActiveCount(const R2D_TweenSystem *system)
{
    return system != 0 ? system->count : 0;
}

void R2D_ShakeStart(R2D_Shake *shake, float duration, float strength)
{
    if (shake == 0) {
        return;
    }

    shake->duration = duration > 0.0f ? duration : 0.0f;
    shake->elapsed = 0.0f;
    shake->strength = strength > 0.0f ? strength : 0.0f;
    shake->offset = (Vector2) { 0.0f, 0.0f };
    shake->active = shake->duration > 0.0f && shake->strength > 0.0f;
}

void R2D_ShakeUpdate(R2D_Shake *shake, float dt)
{
    float amount;

    if (shake == 0 || !shake->active) {
        return;
    }

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    shake->elapsed += dt;
    if (shake->elapsed >= shake->duration) {
        shake->offset = (Vector2) { 0.0f, 0.0f };
        shake->active = false;
        return;
    }

    amount = shake->strength * (1.0f - R2D_Clamp01(shake->elapsed / shake->duration));
    shake->offset = (Vector2) {
        (float)GetRandomValue((int)(-amount * 100.0f), (int)(amount * 100.0f)) / 100.0f,
        (float)GetRandomValue((int)(-amount * 100.0f), (int)(amount * 100.0f)) / 100.0f
    };
}

Vector2 R2D_ShakeOffset(const R2D_Shake *shake)
{
    return shake != 0 ? shake->offset : (Vector2) { 0.0f, 0.0f };
}

void R2D_TimeEffectsInit(R2D_TimeEffects *effects)
{
    if (effects == 0) {
        return;
    }

    *effects = (R2D_TimeEffects) {
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        WHITE
    };
}

float R2D_TimeEffectsUpdate(R2D_TimeEffects *effects, float dt)
{
    float scaled_dt;

    if (effects == 0) {
        return dt;
    }

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    if (effects->hitstop_timer > 0.0f) {
        effects->hitstop_timer -= dt;
        if (effects->hitstop_timer < 0.0f) {
            effects->hitstop_timer = 0.0f;
        }
        scaled_dt = 0.0f;
    } else if (effects->slow_timer > 0.0f) {
        effects->slow_timer -= dt;
        if (effects->slow_timer < 0.0f) {
            effects->slow_timer = 0.0f;
        }
        scaled_dt = dt * effects->slow_scale;
    } else {
        scaled_dt = dt;
    }

    if (effects->flash_timer > 0.0f) {
        effects->flash_timer -= dt;
        if (effects->flash_timer < 0.0f) {
            effects->flash_timer = 0.0f;
        }
    }

    return scaled_dt;
}

void R2D_TimeEffectsHitstop(R2D_TimeEffects *effects, float duration)
{
    if (effects != 0) {
        effects->hitstop_timer = duration > 0.0f ? duration : 0.0f;
    }
}

void R2D_TimeEffectsSlowMotion(R2D_TimeEffects *effects, float duration, float scale)
{
    if (effects == 0) {
        return;
    }

    effects->slow_timer = duration > 0.0f ? duration : 0.0f;
    effects->slow_scale = scale > 0.0f ? scale : 1.0f;
}

void R2D_TimeEffectsFlash(R2D_TimeEffects *effects, float duration, Color color)
{
    if (effects == 0) {
        return;
    }

    effects->flash_duration = duration > 0.0f ? duration : 0.0f;
    effects->flash_timer = effects->flash_duration;
    effects->flash_color = color;
}

void R2D_TimeEffectsFade(R2D_TimeEffects *effects, float alpha)
{
    if (effects != 0) {
        effects->fade_alpha = R2D_Clamp01(alpha);
    }
}

Color R2D_TimeEffectsFlashColor(const R2D_TimeEffects *effects)
{
    Color color;
    float alpha;

    if (effects == 0 || effects->flash_duration <= 0.0f || effects->flash_timer <= 0.0f) {
        return BLANK;
    }

    alpha = R2D_Clamp01(effects->flash_timer / effects->flash_duration);
    color = effects->flash_color;
    color.a = (unsigned char)R2D_Lerp(0.0f, (float)effects->flash_color.a, alpha);
    return color;
}

float R2D_TimeEffectsFadeAlpha(const R2D_TimeEffects *effects)
{
    return effects != 0 ? effects->fade_alpha : 0.0f;
}
