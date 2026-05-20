#include "r2d/r2d.h"

#include <stdio.h>

typedef struct TimeExample {
    R2D_TimerSystem timers;
    R2D_TweenSystem tweens;
    R2D_TimeEffects effects;
    R2D_Shake shake;
    Vector2 block;
    float block_size;
    float pulse;
    float fade;
    int ticks;
    int delayed;
} TimeExample;

static void TimeExample_OnTick(void *user_data)
{
    TimeExample *example = (TimeExample *)user_data;

    example->ticks++;
    example->pulse = 1.0f;
    R2D_TweenFloat(&example->tweens, &example->pulse, 0.0f, 0.35f, R2D_EASE_OUT);
}

static void TimeExample_OnAfter(void *user_data)
{
    TimeExample *example = (TimeExample *)user_data;

    example->delayed++;
    R2D_TimeEffectsFlash(&example->effects, 0.18f, R2D_ColorFromHex(0xffd166aa));
}

static void TimeExample_RestartTweens(TimeExample *example)
{
    example->block.x = 34.0f;
    example->block.y = 86.0f;
    example->block_size = 14.0f;
    example->fade = 0.0f;

    R2D_TweenSystemClear(&example->tweens);
    R2D_TweenFloat(&example->tweens, &example->block.x, 242.0f, 1.1f, R2D_EASE_IN_OUT);
    R2D_TweenFloat(&example->tweens, &example->block.y, 128.0f, 0.75f, R2D_EASE_OUT);
    R2D_TweenFloat(&example->tweens, &example->block_size, 28.0f, 0.55f, R2D_EASE_OUT);
    R2D_TweenFloat(&example->tweens, &example->fade, 0.45f, 1.2f, R2D_EASE_IN_OUT);
}

static void TimeExample_Init(void *user_data)
{
    TimeExample *example = (TimeExample *)user_data;

    R2D_TimerSystemInit(&example->timers);
    R2D_TweenSystemInit(&example->tweens);
    R2D_TimeEffectsInit(&example->effects);
    TimeExample_RestartTweens(example);

    R2D_TimerEvery(&example->timers, 0.5f, 0, TimeExample_OnTick, example);
    R2D_TimerAfter(&example->timers, 1.4f, TimeExample_OnAfter, example);
}

static void TimeExample_Update(float dt, void *user_data)
{
    TimeExample *example = (TimeExample *)user_data;
    const float scaled_dt = R2D_TimeEffectsUpdate(&example->effects, dt);

    R2D_TimerSystemUpdate(&example->timers, dt);
    R2D_TweenSystemUpdate(&example->tweens, scaled_dt);
    R2D_ShakeUpdate(&example->shake, dt);
    R2D_TimeEffectsFade(&example->effects, example->fade);

    if (IsKeyPressed(KEY_SPACE)) {
        TimeExample_RestartTweens(example);
        R2D_TimerAfter(&example->timers, 0.4f, TimeExample_OnAfter, example);
    }

    if (IsKeyPressed(KEY_H)) {
        R2D_TimeEffectsHitstop(&example->effects, 0.18f);
        R2D_TimeEffectsFlash(&example->effects, 0.12f, R2D_ColorFromHex(0xf8f8f2cc));
    }

    if (IsKeyPressed(KEY_S)) {
        R2D_TimeEffectsSlowMotion(&example->effects, 1.2f, 0.25f);
    }

    if (IsKeyPressed(KEY_C)) {
        R2D_ShakeStart(&example->shake, 0.45f, 5.0f);
    }
}

static void TimeExample_DrawTrack(Vector2 offset)
{
    DrawRectangle(30 + (int)offset.x, 82 + (int)offset.y, 260, 50, R2D_ColorFromHex(0x101820ff));
    DrawRectangleLines(30 + (int)offset.x, 82 + (int)offset.y, 260, 50, R2D_ColorFromHex(0x3a506bff));
    DrawLine(42 + (int)offset.x, 118 + (int)offset.y, 278 + (int)offset.x, 118 + (int)offset.y, R2D_ColorFromHex(0x4d5b6aff));
}

static void TimeExample_Draw(void *user_data)
{
    const TimeExample *example = (const TimeExample *)user_data;
    const Vector2 shake = R2D_ShakeOffset(&example->shake);
    const int pulse_radius = 10 + (int)(example->pulse * 12.0f);
    const Rectangle block = {
        example->block.x + shake.x,
        example->block.y + shake.y,
        example->block_size,
        example->block_size
    };
    Color flash = R2D_TimeEffectsFlashColor(&example->effects);
    char text[128];

    DrawText("Time effects example", 2, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Space tween/reset, H hitstop, S slow motion, C camera shake", 2, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    TimeExample_DrawTrack(shake);
    DrawCircleLines(54 + (int)shake.x, 58 + (int)shake.y, (float)pulse_radius, R2D_ColorFromHex(0x06d6a0ff));
    DrawCircle(54 + (int)shake.x, 58 + (int)shake.y, 4.0f, R2D_ColorFromHex(0x06d6a0ff));
    DrawRectangleRec(block, R2D_ColorFromHex(0xef476fff));
    DrawRectangleLinesEx(block, 1.0f, R2D_ColorFromHex(0xf8f8f2ff));

    snprintf(
        text,
        sizeof(text),
        "timers:%d  tweens:%d  every ticks:%d  after fired:%d",
        R2D_TimerSystemActiveCount(&example->timers),
        R2D_TweenSystemActiveCount(&example->tweens),
        example->ticks,
        example->delayed
    );
    DrawText(text, 14, 158, 8, R2D_ColorFromHex(0xf8f8f2ff));

    if (R2D_TimeEffectsFadeAlpha(&example->effects) > 0.0f) {
        DrawRectangle(
            0,
            0,
            320,
            200,
            (Color) { 0, 0, 0, (unsigned char)(R2D_TimeEffectsFadeAlpha(&example->effects) * 255.0f) }
        );
    }

    if (flash.a > 0) {
        DrawRectangle(0, 0, 320, 200, flash);
    }

    DrawRectangle(10, 184, 300, 12, R2D_ColorFromHex(0x101820cc));
    DrawText("Timers drive ticks. Tweens and effects use scaled game time.", 2, 187, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    TimeExample example = { 0 };

    config.title = "Retro2D Time Effects Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_Run(&context, (R2D_App) {
        TimeExample_Init,
        TimeExample_Update,
        TimeExample_Draw,
        0,
        &example
    });

    R2D_Close(&context);
    return 0;
}
