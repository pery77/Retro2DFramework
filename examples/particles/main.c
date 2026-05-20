#include "r2d/r2d.h"

#include <stdio.h>

#define PARTICLE_PRESET_COUNT 6

typedef struct ParticleExample {
    R2D_Context *context;
    R2D_ParticleSystem particles;
    R2D_ParticleEmitter stream;
    R2D_ParticlePreset preset;
    float burst_timer;
} ParticleExample;

static const char *ParticleExample_PresetName(R2D_ParticlePreset preset)
{
    switch (preset) {
    case R2D_PARTICLE_PRESET_DUST:
        return "dust";
    case R2D_PARTICLE_PRESET_HIT:
        return "hit";
    case R2D_PARTICLE_PRESET_SPARK:
        return "spark";
    case R2D_PARTICLE_PRESET_SMOKE:
        return "smoke";
    case R2D_PARTICLE_PRESET_COIN:
        return "coin";
    case R2D_PARTICLE_PRESET_STAR:
        return "star";
    default:
        return "unknown";
    }
}

static void ParticleExample_SelectPreset(ParticleExample *example, R2D_ParticlePreset preset)
{
    const Vector2 center = {
        (float)R2D_VirtualWidth(example->context) * 0.5f,
        116.0f
    };

    example->preset = preset;
    example->stream = R2D_ParticleEmitterPreset(preset, center);
    example->stream.emit_rate = preset == R2D_PARTICLE_PRESET_SMOKE ? 24.0f : 36.0f;
}

static void ParticleExample_BurstAt(ParticleExample *example, Vector2 position, int count)
{
    R2D_ParticleEmitter emitter = R2D_ParticleEmitterPreset(example->preset, position);

    R2D_ParticleBurst(&example->particles, &emitter, count);
}

static void ParticleExample_Init(void *user_data)
{
    ParticleExample *example = (ParticleExample *)user_data;

    R2D_ParticleSystemInit(&example->particles);
    ParticleExample_SelectPreset(example, R2D_PARTICLE_PRESET_DUST);
    ParticleExample_BurstAt(example, example->stream.position, 36);
    HideCursor();
}

static void ParticleExample_Update(float dt, void *user_data)
{
    ParticleExample *example = (ParticleExample *)user_data;
    Vector2 mouse = R2D_MouseVirtualPosition(example->context);

    if (mouse.x < 0.0f || mouse.y < 0.0f ||
        mouse.x >= (float)R2D_VirtualWidth(example->context) ||
        mouse.y >= (float)R2D_VirtualHeight(example->context)) {
        mouse = example->stream.position;
    }

    example->stream.position = mouse;

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        const int next = ((int)example->preset + 1) % PARTICLE_PRESET_COUNT;
        ParticleExample_SelectPreset(example, (R2D_ParticlePreset)next);
    }

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        const int next = ((int)example->preset + PARTICLE_PRESET_COUNT - 1) % PARTICLE_PRESET_COUNT;
        ParticleExample_SelectPreset(example, (R2D_ParticlePreset)next);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)) {
        ParticleExample_BurstAt(example, mouse, 42);
    }

    example->burst_timer += dt;
    if (example->burst_timer >= 1.8f) {
        example->burst_timer = 0.0f;
        ParticleExample_BurstAt(example, example->stream.position, 24);
    }

    R2D_ParticleEmitterUpdate(&example->particles, &example->stream, dt);
    R2D_ParticleSystemUpdate(&example->particles, dt);
}

static void ParticleExample_DrawGrid(void)
{
    for (int x = 0; x <= 320; x += 16) {
        DrawLine(x, 0, x, 200, R2D_ColorFromHex(0x24344755));
    }

    for (int y = 0; y <= 200; y += 16) {
        DrawLine(0, y, 320, y, R2D_ColorFromHex(0x24344755));
    }
}

static void ParticleExample_Draw(void *user_data)
{
    const ParticleExample *example = (const ParticleExample *)user_data;
    char text[96];

    ParticleExample_DrawGrid();
    DrawText("Particle example", 2, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Move mouse. Click/Space bursts. Left/Right changes preset.", 2, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    R2D_ParticleSystemDraw(&example->particles);

    DrawRectangle(10, 184, 300, 12, R2D_ColorFromHex(0x101820cc));
    snprintf(
        text,
        sizeof(text),
        "preset:%s  alive:%d/%d",
        ParticleExample_PresetName(example->preset),
        R2D_ParticleSystemAliveCount(&example->particles),
        R2D_PARTICLE_MAX
    );
    DrawText(text, 14, 187, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

static void ParticleExample_Shutdown(void *user_data)
{
    (void)user_data;
    ShowCursor();
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    ParticleExample example = { 0 };

    config.title = "Retro2D Particle Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    example.context = &context;
    R2D_Run(&context, (R2D_App) {
        ParticleExample_Init,
        ParticleExample_Update,
        ParticleExample_Draw,
        ParticleExample_Shutdown,
        &example
    });

    R2D_Close(&context);
    return 0;
}
