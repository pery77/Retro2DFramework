#include "r2d/r2d.h"

#include <string.h>

static float R2D_ParticleClamp01(float value)
{
    if (value < 0.0f) {
        return 0.0f;
    }

    if (value > 1.0f) {
        return 1.0f;
    }

    return value;
}

static float R2D_ParticleLerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static float R2D_ParticleRandomRange(float min, float max)
{
    const int scale = 10000;
    const int value = GetRandomValue(0, scale);
    const float t = (float)value / (float)scale;

    if (max < min) {
        const float swap = min;
        min = max;
        max = swap;
    }

    return R2D_ParticleLerp(min, max, t);
}

static Color R2D_ParticleColorLerp(Color a, Color b, float t)
{
    return (Color) {
        (unsigned char)R2D_ParticleLerp((float)a.r, (float)b.r, t),
        (unsigned char)R2D_ParticleLerp((float)a.g, (float)b.g, t),
        (unsigned char)R2D_ParticleLerp((float)a.b, (float)b.b, t),
        (unsigned char)R2D_ParticleLerp((float)a.a, (float)b.a, t)
    };
}

void R2D_ParticleSystemInit(R2D_ParticleSystem *system)
{
    if (system == 0) {
        return;
    }

    memset(system, 0, sizeof(*system));
}

void R2D_ParticleSystemClear(R2D_ParticleSystem *system)
{
    R2D_ParticleSystemInit(system);
}

R2D_ParticleEmitter R2D_ParticleEmitterPreset(R2D_ParticlePreset preset, Vector2 position)
{
    R2D_ParticleEmitter emitter = {
        position,
        { -20.0f, -20.0f },
        { 20.0f, 20.0f },
        { 0.0f, 0.0f },
        0.35f,
        0.65f,
        1.0f,
        2.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        WHITE,
        BLANK,
        R2D_PARTICLE_SHAPE_PIXEL,
        true
    };

    switch (preset) {
    case R2D_PARTICLE_PRESET_DUST:
        emitter.velocity_min = (Vector2) { -18.0f, -32.0f };
        emitter.velocity_max = (Vector2) { 18.0f, -6.0f };
        emitter.acceleration = (Vector2) { 0.0f, 48.0f };
        emitter.life_min = 0.25f;
        emitter.life_max = 0.55f;
        emitter.size_min = 1.0f;
        emitter.size_max = 2.0f;
        emitter.start_color = R2D_ColorFromHex(0xd8c08d99);
        emitter.end_color = R2D_ColorFromHex(0x8d6e4a00);
        break;
    case R2D_PARTICLE_PRESET_HIT:
        emitter.velocity_min = (Vector2) { -80.0f, -70.0f };
        emitter.velocity_max = (Vector2) { 80.0f, 50.0f };
        emitter.acceleration = (Vector2) { 0.0f, 100.0f };
        emitter.life_min = 0.12f;
        emitter.life_max = 0.28f;
        emitter.size_min = 1.0f;
        emitter.size_max = 3.0f;
        emitter.start_color = R2D_ColorFromHex(0xfff1a8ff);
        emitter.end_color = R2D_ColorFromHex(0xef476f00);
        break;
    case R2D_PARTICLE_PRESET_SPARK:
        emitter.velocity_min = (Vector2) { -70.0f, -90.0f };
        emitter.velocity_max = (Vector2) { 70.0f, -20.0f };
        emitter.acceleration = (Vector2) { 0.0f, 140.0f };
        emitter.life_min = 0.18f;
        emitter.life_max = 0.42f;
        emitter.size_min = 1.0f;
        emitter.size_max = 2.0f;
        emitter.start_color = R2D_ColorFromHex(0xffd166ff);
        emitter.end_color = R2D_ColorFromHex(0xf77f0000);
        break;
    case R2D_PARTICLE_PRESET_SMOKE:
        emitter.velocity_min = (Vector2) { -10.0f, -32.0f };
        emitter.velocity_max = (Vector2) { 10.0f, -12.0f };
        emitter.acceleration = (Vector2) { 0.0f, -10.0f };
        emitter.life_min = 0.7f;
        emitter.life_max = 1.25f;
        emitter.size_min = 2.0f;
        emitter.size_max = 4.0f;
        emitter.end_size_min = 5.0f;
        emitter.end_size_max = 8.0f;
        emitter.start_color = R2D_ColorFromHex(0xb7bec899);
        emitter.end_color = R2D_ColorFromHex(0x4d5b6a00);
        emitter.shape = R2D_PARTICLE_SHAPE_CIRCLE;
        break;
    case R2D_PARTICLE_PRESET_COIN:
        emitter.velocity_min = (Vector2) { -34.0f, -82.0f };
        emitter.velocity_max = (Vector2) { 34.0f, -38.0f };
        emitter.acceleration = (Vector2) { 0.0f, 120.0f };
        emitter.life_min = 0.45f;
        emitter.life_max = 0.75f;
        emitter.size_min = 2.0f;
        emitter.size_max = 3.0f;
        emitter.start_color = R2D_ColorFromHex(0xffd166ff);
        emitter.end_color = R2D_ColorFromHex(0xf77f0000);
        break;
    case R2D_PARTICLE_PRESET_STAR:
        emitter.velocity_min = (Vector2) { -48.0f, -62.0f };
        emitter.velocity_max = (Vector2) { 48.0f, 20.0f };
        emitter.acceleration = (Vector2) { 0.0f, 42.0f };
        emitter.life_min = 0.35f;
        emitter.life_max = 0.7f;
        emitter.size_min = 1.0f;
        emitter.size_max = 2.0f;
        emitter.end_size_min = 1.0f;
        emitter.end_size_max = 1.0f;
        emitter.start_color = R2D_ColorFromHex(0xf8f8f2ff);
        emitter.end_color = R2D_ColorFromHex(0x8ecae600);
        break;
    default:
        break;
    }

    return emitter;
}

bool R2D_ParticleEmit(R2D_ParticleSystem *system, const R2D_ParticleEmitter *emitter)
{
    R2D_Particle *particle = 0;

    if (system == 0 || emitter == 0 || !emitter->active || system->count >= R2D_PARTICLE_MAX) {
        return false;
    }

    for (int i = 0; i < R2D_PARTICLE_MAX; ++i) {
        if (!system->particles[i].active) {
            particle = &system->particles[i];
            break;
        }
    }

    if (particle == 0) {
        return false;
    }

    memset(particle, 0, sizeof(*particle));
    particle->position = emitter->position;
    particle->velocity = (Vector2) {
        R2D_ParticleRandomRange(emitter->velocity_min.x, emitter->velocity_max.x),
        R2D_ParticleRandomRange(emitter->velocity_min.y, emitter->velocity_max.y)
    };
    particle->acceleration = emitter->acceleration;
    particle->life = R2D_ParticleRandomRange(emitter->life_min, emitter->life_max);
    particle->start_size = R2D_ParticleRandomRange(emitter->size_min, emitter->size_max);
    particle->end_size = R2D_ParticleRandomRange(emitter->end_size_min, emitter->end_size_max);
    particle->start_color = emitter->start_color;
    particle->end_color = emitter->end_color;
    particle->shape = emitter->shape;
    particle->active = particle->life > 0.0f;

    if (particle->active) {
        system->count++;
    }

    return particle->active;
}

int R2D_ParticleBurst(R2D_ParticleSystem *system, const R2D_ParticleEmitter *emitter, int count)
{
    int emitted = 0;

    if (count <= 0) {
        return 0;
    }

    for (int i = 0; i < count; ++i) {
        if (R2D_ParticleEmit(system, emitter)) {
            emitted++;
        }
    }

    return emitted;
}

void R2D_ParticleEmitterUpdate(R2D_ParticleSystem *system, R2D_ParticleEmitter *emitter, float dt)
{
    if (system == 0 || emitter == 0 || !emitter->active || emitter->emit_rate <= 0.0f) {
        return;
    }

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    emitter->emit_timer += dt * emitter->emit_rate;

    while (emitter->emit_timer >= 1.0f) {
        if (!R2D_ParticleEmit(system, emitter)) {
            emitter->emit_timer = 0.0f;
            break;
        }

        emitter->emit_timer -= 1.0f;
    }
}

void R2D_ParticleSystemUpdate(R2D_ParticleSystem *system, float dt)
{
    if (system == 0) {
        return;
    }

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    for (int i = 0; i < R2D_PARTICLE_MAX; ++i) {
        R2D_Particle *particle = &system->particles[i];

        if (!particle->active) {
            continue;
        }

        particle->age += dt;
        if (particle->age >= particle->life) {
            particle->active = false;
            system->count--;
            continue;
        }

        particle->velocity.x += particle->acceleration.x * dt;
        particle->velocity.y += particle->acceleration.y * dt;
        particle->position.x += particle->velocity.x * dt;
        particle->position.y += particle->velocity.y * dt;
    }
}

void R2D_ParticleSystemDraw(const R2D_ParticleSystem *system)
{
    if (system == 0) {
        return;
    }

    for (int i = 0; i < R2D_PARTICLE_MAX; ++i) {
        const R2D_Particle *particle = &system->particles[i];
        float t;
        float size;
        Color color;

        if (!particle->active) {
            continue;
        }

        t = particle->life > 0.0f ? R2D_ParticleClamp01(particle->age / particle->life) : 1.0f;
        size = R2D_ParticleLerp(particle->start_size, particle->end_size, t);
        color = R2D_ParticleColorLerp(particle->start_color, particle->end_color, t);

        if (size <= 0.0f || color.a == 0) {
            continue;
        }

        if (particle->shape == R2D_PARTICLE_SHAPE_CIRCLE) {
            DrawCircleV(particle->position, size, color);
        } else {
            DrawRectangle(
                (int)(particle->position.x - size * 0.5f),
                (int)(particle->position.y - size * 0.5f),
                (int)(size < 1.0f ? 1.0f : size),
                (int)(size < 1.0f ? 1.0f : size),
                color
            );
        }
    }
}

int R2D_ParticleSystemAliveCount(const R2D_ParticleSystem *system)
{
    return system != 0 ? system->count : 0;
}
