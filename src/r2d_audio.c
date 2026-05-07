#include "r2d/r2d.h"

#include <math.h>

#define R2D_AUDIO_CHANNELS 2
#define R2D_AUDIO_SAMPLE_SIZE 16
#define R2D_AUDIO_BUFFER_FRAMES 800

typedef struct R2D_AudioVoice {
    R2D_Sfx sfx;
    float phase;
    float elapsed;
    float noise_value;
    unsigned int noise_state;
    bool active;
} R2D_AudioVoice;

typedef struct R2D_AudioState {
    AudioStream stream;
    R2D_AudioVoice voices[R2D_AUDIO_MAX_VOICES];
    unsigned int next_noise_seed;
    bool owns_device;
    bool ready;
} R2D_AudioState;

static R2D_AudioState r2d_audio = { 0 };

static float R2D_AudioClamp(float value, float min, float max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

static float R2D_AudioFastSin(float phase)
{
    float x = phase + 0.5f;
    float result;

    x = x > 0.5f ? (x - 1.0f) * 6.28318531f : x * 6.28318531f;
    result = x < 0.0f
        ? 1.27323954f * x + 0.405284735f * x * x
        : 1.27323954f * x - 0.405284735f * x * x;

    result = result < 0.0f
        ? 0.225f * (result * -result - result) + result
        : 0.225f * (result * result - result) + result;

    return result;
}

static float R2D_AudioEnvelope(const R2D_Sfx *sfx, float elapsed)
{
    float t = elapsed;
    const float attack = fmaxf(0.0f, sfx->attack);
    const float decay = fmaxf(0.0f, sfx->decay);
    const float sustain = R2D_AudioClamp(sfx->sustain, 0.0f, 1.0f);
    const float duration = fmaxf(0.0f, sfx->duration);
    const float release = fmaxf(0.0f, sfx->release);

    if (attack > 0.0f && t < attack) {
        return t / attack;
    }

    t -= attack;

    if (decay > 0.0f && t < decay) {
        const float amount = t / decay;
        return 1.0f + (sustain - 1.0f) * amount;
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

static bool R2D_AudioVoiceFinished(const R2D_AudioVoice *voice)
{
    const R2D_Sfx *sfx = &voice->sfx;
    const float length = fmaxf(0.0f, sfx->attack) +
        fmaxf(0.0f, sfx->decay) +
        fmaxf(0.0f, sfx->duration) +
        fmaxf(0.0f, sfx->release);

    return voice->elapsed >= length;
}

static float R2D_AudioNextNoise(R2D_AudioVoice *voice)
{
    voice->noise_state = voice->noise_state * 1664525u + 1013904223u;
    return ((float)((voice->noise_state >> 16) & 0xffffu) / 32767.5f) - 1.0f;
}

static float R2D_AudioOscillate(R2D_AudioVoice *voice, float frequency)
{
    const R2D_Sfx *sfx = &voice->sfx;
    float sample = 0.0f;
    const float duty = R2D_AudioClamp(sfx->duty, 0.01f, 0.99f);
    const float phase = voice->phase;
    const float step = R2D_AudioClamp(frequency, 0.0f, 22000.0f) / (float)R2D_AUDIO_SAMPLE_RATE;

    switch (sfx->waveform) {
        case R2D_WAVE_TRIANGLE:
            sample = phase < 0.5f ? phase * 4.0f - 1.0f : 3.0f - phase * 4.0f;
            break;
        case R2D_WAVE_SAW:
            sample = phase * 2.0f - 1.0f;
            break;
        case R2D_WAVE_RAMP:
            sample = 1.0f - phase * 2.0f;
            break;
        case R2D_WAVE_NOISE:
            if (phase + step >= 1.0f) {
                voice->noise_value = R2D_AudioNextNoise(voice);
            }
            sample = voice->noise_value;
            break;
        case R2D_WAVE_SINE:
            sample = R2D_AudioFastSin(phase);
            break;
        case R2D_WAVE_SQUARE:
        default:
            sample = phase < duty ? 1.0f : -1.0f;
            break;
    }

    voice->phase += step;

    while (voice->phase >= 1.0f) {
        voice->phase -= 1.0f;
    }

    return sample;
}

static float R2D_AudioProcessVoice(R2D_AudioVoice *voice)
{
    const float envelope = R2D_AudioEnvelope(&voice->sfx, voice->elapsed);
    const float frequency = voice->sfx.frequency + voice->sfx.pitch_slide * voice->elapsed;
    const float sample = R2D_AudioOscillate(voice, frequency) * envelope * voice->sfx.volume;

    voice->elapsed += 1.0f / (float)R2D_AUDIO_SAMPLE_RATE;

    if (R2D_AudioVoiceFinished(voice)) {
        voice->active = false;
    }

    return sample;
}

static void R2D_AudioCallback(void *buffer, unsigned int frames)
{
    short *samples = (short *)buffer;

    for (unsigned int frame = 0; frame < frames; ++frame) {
        float left = 0.0f;
        float right = 0.0f;

        for (int voice_index = 0; voice_index < R2D_AUDIO_MAX_VOICES; ++voice_index) {
            R2D_AudioVoice *voice = &r2d_audio.voices[voice_index];

            if (voice->active) {
                const float pan = R2D_AudioClamp(voice->sfx.pan, 0.0f, 1.0f);
                const float sample = R2D_AudioProcessVoice(voice);

                left += sample * (1.0f - pan);
                right += sample * pan;
            }
        }

        left = R2D_AudioClamp(left, -1.0f, 1.0f);
        right = R2D_AudioClamp(right, -1.0f, 1.0f);

        samples[frame * R2D_AUDIO_CHANNELS] = (short)(left * 32767.0f);
        samples[frame * R2D_AUDIO_CHANNELS + 1] = (short)(right * 32767.0f);
    }
}

bool R2D_AudioInit(void)
{
    if (r2d_audio.ready) {
        return true;
    }

    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
        r2d_audio.owns_device = true;
    }

    if (!IsAudioDeviceReady()) {
        r2d_audio.owns_device = false;
        return false;
    }

    SetAudioStreamBufferSizeDefault(R2D_AUDIO_BUFFER_FRAMES);
    r2d_audio.stream = LoadAudioStream(R2D_AUDIO_SAMPLE_RATE, R2D_AUDIO_SAMPLE_SIZE, R2D_AUDIO_CHANNELS);

    if (!IsAudioStreamValid(r2d_audio.stream)) {
        if (r2d_audio.owns_device) {
            CloseAudioDevice();
        }

        r2d_audio = (R2D_AudioState) { 0 };
        return false;
    }

    r2d_audio.next_noise_seed = 0x12345678u;
    SetAudioStreamCallback(r2d_audio.stream, R2D_AudioCallback);
    PlayAudioStream(r2d_audio.stream);
    r2d_audio.ready = true;

    return true;
}

void R2D_AudioClose(void)
{
    if (!r2d_audio.ready) {
        return;
    }

    StopAudioStream(r2d_audio.stream);
    UnloadAudioStream(r2d_audio.stream);

    if (r2d_audio.owns_device && IsAudioDeviceReady()) {
        CloseAudioDevice();
    }

    r2d_audio = (R2D_AudioState) { 0 };
}

bool R2D_AudioIsReady(void)
{
    return r2d_audio.ready;
}

R2D_Sfx R2D_DefaultSfx(void)
{
    return (R2D_Sfx) {
        R2D_WAVE_SQUARE,
        440.0f,
        0.35f,
        0.5f,
        0.0f,
        0.04f,
        0.8f,
        0.05f,
        0.04f,
        0.0f,
        0.5f
    };
}

void R2D_PlaySfx(R2D_Sfx sfx)
{
    int selected = -1;
    float oldest_elapsed = -1.0f;

    if (!r2d_audio.ready) {
        return;
    }

    sfx.volume = R2D_AudioClamp(sfx.volume, 0.0f, 1.0f);
    sfx.pan = R2D_AudioClamp(sfx.pan, 0.0f, 1.0f);

    for (int i = 0; i < R2D_AUDIO_MAX_VOICES; ++i) {
        if (!r2d_audio.voices[i].active) {
            selected = i;
            break;
        }

        if (r2d_audio.voices[i].elapsed > oldest_elapsed) {
            oldest_elapsed = r2d_audio.voices[i].elapsed;
            selected = i;
        }
    }

    if (selected >= 0) {
        R2D_AudioVoice *voice = &r2d_audio.voices[selected];

        voice->sfx = sfx;
        voice->phase = 0.0f;
        voice->elapsed = 0.0f;
        voice->noise_state = r2d_audio.next_noise_seed++;
        voice->noise_value = R2D_AudioNextNoise(voice);
        voice->active = true;
    }
}

void R2D_PlayTone(R2D_Waveform waveform, float frequency, float duration)
{
    R2D_Sfx sfx = R2D_DefaultSfx();

    sfx.waveform = waveform;
    sfx.frequency = frequency;
    sfx.duration = duration;

    R2D_PlaySfx(sfx);
}
