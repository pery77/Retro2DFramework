#include "r2d/r2d.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R2D_AUDIO_CHANNELS 2
#define R2D_AUDIO_SAMPLE_SIZE 16
#define R2D_AUDIO_BUFFER_FRAMES 800

typedef struct R2D_AudioVoice {
    R2D_Sfx sfx;
    float phase;
    float elapsed;
    float noise_value;
    float filter_low;
    float filter_band;
    unsigned int noise_state;
    bool active;
} R2D_AudioVoice;

typedef struct R2D_AudioState {
    AudioStream stream;
    R2D_AudioVoice voices[R2D_AUDIO_MAX_VOICES];
    unsigned int next_noise_seed;
    float master_volume;
    bool owns_device;
    bool ready;
} R2D_AudioState;

static R2D_AudioState r2d_audio = { 0 };

static bool R2D_AudioOpenFile(FILE **file, const char *path, const char *mode)
{
    if (file == 0 || path == 0 || mode == 0) {
        return false;
    }

#if defined(_MSC_VER)
    return fopen_s(file, path, mode) == 0 && *file != 0;
#else
    *file = fopen(path, mode);
    return *file != 0;
#endif
}

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

static char *R2D_AudioTrim(char *text)
{
    char *end;

    while (*text != '\0' && isspace((unsigned char)*text)) {
        ++text;
    }

    end = text + strlen(text);

    while (end > text && isspace((unsigned char)*(end - 1))) {
        --end;
    }

    *end = '\0';
    return text;
}

static bool R2D_AudioStringEquals(const char *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) {
            return false;
        }

        ++left;
        ++right;
    }

    return *left == '\0' && *right == '\0';
}

static const char *R2D_AudioWaveformName(R2D_Waveform waveform)
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

static bool R2D_AudioParseWaveform(const char *text, R2D_Waveform *waveform)
{
    if (R2D_AudioStringEquals(text, "square")) {
        *waveform = R2D_WAVE_SQUARE;
    } else if (R2D_AudioStringEquals(text, "triangle")) {
        *waveform = R2D_WAVE_TRIANGLE;
    } else if (R2D_AudioStringEquals(text, "saw")) {
        *waveform = R2D_WAVE_SAW;
    } else if (R2D_AudioStringEquals(text, "ramp")) {
        *waveform = R2D_WAVE_RAMP;
    } else if (R2D_AudioStringEquals(text, "noise")) {
        *waveform = R2D_WAVE_NOISE;
    } else if (R2D_AudioStringEquals(text, "sine")) {
        *waveform = R2D_WAVE_SINE;
    } else {
        return false;
    }

    return true;
}

static const char *R2D_AudioFilterName(R2D_Filter filter)
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

static bool R2D_AudioParseFilter(const char *text, R2D_Filter *filter)
{
    if (R2D_AudioStringEquals(text, "none")) {
        *filter = R2D_FILTER_NONE;
    } else if (R2D_AudioStringEquals(text, "lowpass")) {
        *filter = R2D_FILTER_LOWPASS;
    } else if (R2D_AudioStringEquals(text, "highpass")) {
        *filter = R2D_FILTER_HIGHPASS;
    } else if (R2D_AudioStringEquals(text, "bandpass")) {
        *filter = R2D_FILTER_BANDPASS;
    } else {
        return false;
    }

    return true;
}

static bool R2D_AudioReadFloat(const char *text, float *value)
{
    char *end = 0;
    const float parsed = strtof(text, &end);

    if (end == text) {
        return false;
    }

    *value = parsed;
    return true;
}

static bool R2D_AudioApplySfxField(R2D_Sfx *sfx, const char *key, const char *value)
{
    float number = 0.0f;

    if (R2D_AudioStringEquals(key, "waveform")) {
        return R2D_AudioParseWaveform(value, &sfx->waveform);
    }

    if (R2D_AudioStringEquals(key, "filter")) {
        return R2D_AudioParseFilter(value, &sfx->filter);
    }

    if (!R2D_AudioReadFloat(value, &number)) {
        return false;
    }

    if (R2D_AudioStringEquals(key, "frequency")) {
        sfx->frequency = number;
    } else if (R2D_AudioStringEquals(key, "volume")) {
        sfx->volume = number;
    } else if (R2D_AudioStringEquals(key, "pan")) {
        sfx->pan = number;
    } else if (R2D_AudioStringEquals(key, "attack")) {
        sfx->attack = number;
    } else if (R2D_AudioStringEquals(key, "decay")) {
        sfx->decay = number;
    } else if (R2D_AudioStringEquals(key, "sustain")) {
        sfx->sustain = number;
    } else if (R2D_AudioStringEquals(key, "duration")) {
        sfx->duration = number;
    } else if (R2D_AudioStringEquals(key, "release")) {
        sfx->release = number;
    } else if (R2D_AudioStringEquals(key, "pitch_slide")) {
        sfx->pitch_slide = number;
    } else if (R2D_AudioStringEquals(key, "vibrato_depth")) {
        sfx->vibrato_depth = number;
    } else if (R2D_AudioStringEquals(key, "vibrato_rate")) {
        sfx->vibrato_rate = number;
    } else if (R2D_AudioStringEquals(key, "arpeggio_step_1")) {
        sfx->arpeggio_step_1 = number;
    } else if (R2D_AudioStringEquals(key, "arpeggio_step_2")) {
        sfx->arpeggio_step_2 = number;
    } else if (R2D_AudioStringEquals(key, "arpeggio_rate")) {
        sfx->arpeggio_rate = number;
    } else if (R2D_AudioStringEquals(key, "duty")) {
        sfx->duty = number;
    } else if (R2D_AudioStringEquals(key, "duty_slide")) {
        sfx->duty_slide = number;
    } else if (R2D_AudioStringEquals(key, "filter_cutoff")) {
        sfx->filter_cutoff = number;
    } else if (R2D_AudioStringEquals(key, "filter_cutoff_slide")) {
        sfx->filter_cutoff_slide = number;
    } else if (R2D_AudioStringEquals(key, "filter_resonance")) {
        sfx->filter_resonance = number;
    } else {
        return false;
    }

    return true;
}

static float R2D_AudioFastSin(float phase)
{
    float x = phase - floorf(phase);
    float result;

    x += 0.5f;
    x = x > 0.5f ? (x - 1.0f) * 6.28318531f : x * 6.28318531f;
    result = x < 0.0f
        ? 1.27323954f * x + 0.405284735f * x * x
        : 1.27323954f * x - 0.405284735f * x * x;

    result = result < 0.0f
        ? 0.225f * (result * -result - result) + result
        : 0.225f * (result * result - result) + result;

    return result;
}

static float R2D_AudioSoftClip(float sample)
{
    const float amount = fabsf(sample);

    return sample / (1.0f + amount * 0.35f);
}

static float R2D_AudioSemitoneRatio(float semitones)
{
    return powf(2.0f, semitones / 12.0f);
}

static float R2D_AudioArpeggioOffset(const R2D_Sfx *sfx, float elapsed)
{
    int step;

    if (sfx->arpeggio_rate <= 0.0f) {
        return 0.0f;
    }

    step = (int)(elapsed * sfx->arpeggio_rate) % 3;

    if (step == 1) {
        return sfx->arpeggio_step_1;
    }

    if (step == 2) {
        return sfx->arpeggio_step_2;
    }

    return 0.0f;
}

static float R2D_AudioApplyFilter(R2D_AudioVoice *voice, float input)
{
    const R2D_Sfx *sfx = &voice->sfx;
    const float cutoff = R2D_AudioClamp(
        sfx->filter_cutoff + sfx->filter_cutoff_slide * voice->elapsed,
        20.0f,
        18000.0f
    );
    const float resonance = R2D_AudioClamp(sfx->filter_resonance, 0.0f, 1.0f);
    const float g = tanf(3.14159265f * cutoff / (float)R2D_AUDIO_SAMPLE_RATE);
    const float k = 2.0f - resonance * 1.7f;
    const float a1 = 1.0f / (1.0f + g * (g + k));
    const float v3 = input - voice->filter_low;
    const float band = (voice->filter_band + g * v3) * a1;
    const float low = voice->filter_low + g * band;
    float high;

    if (sfx->filter == R2D_FILTER_NONE) {
        return input;
    }

    high = input - k * band - low;
    voice->filter_band = 2.0f * band - voice->filter_band;
    voice->filter_low = 2.0f * low - voice->filter_low;

    switch (sfx->filter) {
        case R2D_FILTER_LOWPASS:
            return low;
        case R2D_FILTER_HIGHPASS:
            return high;
        case R2D_FILTER_BANDPASS:
            return band;
        case R2D_FILTER_NONE:
        default:
            return input;
    }
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

static float R2D_AudioOscillate(R2D_AudioVoice *voice, float frequency, float duty)
{
    const R2D_Sfx *sfx = &voice->sfx;
    float sample = 0.0f;
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
    const float vibrato_phase = voice->elapsed * voice->sfx.vibrato_rate;
    const float vibrato = R2D_AudioFastSin(vibrato_phase) * voice->sfx.vibrato_depth;
    const float arpeggio = R2D_AudioArpeggioOffset(&voice->sfx, voice->elapsed);
    const float base_frequency = voice->sfx.frequency + voice->sfx.pitch_slide * voice->elapsed + vibrato;
    const float frequency = base_frequency * R2D_AudioSemitoneRatio(arpeggio);
    const float duty = R2D_AudioClamp(voice->sfx.duty + voice->sfx.duty_slide * voice->elapsed, 0.01f, 0.99f);
    const float raw_sample = R2D_AudioOscillate(voice, frequency, duty);
    const float sample = R2D_AudioApplyFilter(voice, raw_sample) * envelope * voice->sfx.volume;

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
                const float left_gain = sqrtf(1.0f - pan);
                const float right_gain = sqrtf(pan);
                const float sample = R2D_AudioProcessVoice(voice);

                left += sample * left_gain;
                right += sample * right_gain;
            }
        }

        left = R2D_AudioClamp(R2D_AudioSoftClip(left * r2d_audio.master_volume), -1.0f, 1.0f);
        right = R2D_AudioClamp(R2D_AudioSoftClip(right * r2d_audio.master_volume), -1.0f, 1.0f);

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
    r2d_audio.master_volume = 0.75f;
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

void R2D_AudioSetMasterVolume(float volume)
{
    r2d_audio.master_volume = R2D_AudioClamp(volume, 0.0f, 1.0f);
}

float R2D_AudioMasterVolume(void)
{
    return r2d_audio.master_volume;
}

R2D_Sfx R2D_DefaultSfx(void)
{
    return (R2D_Sfx) {
        .waveform = R2D_WAVE_SQUARE,
        .filter = R2D_FILTER_NONE,
        .frequency = 440.0f,
        .volume = 0.35f,
        .pan = 0.5f,
        .attack = 0.0f,
        .decay = 0.04f,
        .sustain = 0.8f,
        .duration = 0.05f,
        .release = 0.04f,
        .pitch_slide = 0.0f,
        .vibrato_depth = 0.0f,
        .vibrato_rate = 0.0f,
        .arpeggio_step_1 = 0.0f,
        .arpeggio_step_2 = 0.0f,
        .arpeggio_rate = 0.0f,
        .duty = 0.5f,
        .duty_slide = 0.0f,
        .filter_cutoff = 8000.0f,
        .filter_cutoff_slide = 0.0f,
        .filter_resonance = 0.0f
    };
}

R2D_Sfx R2D_SfxCoin(void)
{
    R2D_Sfx sfx = R2D_DefaultSfx();

    sfx.waveform = R2D_WAVE_SQUARE;
    sfx.frequency = 1320.0f;
    sfx.volume = 0.22f;
    sfx.attack = 0.0f;
    sfx.decay = 0.012f;
    sfx.sustain = 0.9f;
    sfx.duration = 0.075f;
    sfx.release = 0.035f;
    sfx.arpeggio_step_1 = 4.0f;
    sfx.arpeggio_step_2 = 7.0f;
    sfx.arpeggio_rate = 28.0f;
    sfx.duty = 0.5f;
    sfx.filter = R2D_FILTER_NONE;

    return sfx;
}

R2D_Sfx R2D_SfxJump(void)
{
    R2D_Sfx sfx = R2D_DefaultSfx();

    sfx.waveform = R2D_WAVE_TRIANGLE;
    sfx.frequency = 220.0f;
    sfx.volume = 0.32f;
    sfx.attack = 0.001f;
    sfx.decay = 0.03f;
    sfx.sustain = 0.85f;
    sfx.duration = 0.08f;
    sfx.release = 0.055f;
    sfx.pitch_slide = 760.0f;
    sfx.vibrato_depth = 8.0f;
    sfx.vibrato_rate = 18.0f;

    return sfx;
}

R2D_Sfx R2D_SfxLaser(void)
{
    R2D_Sfx sfx = R2D_DefaultSfx();

    sfx.waveform = R2D_WAVE_SQUARE;
    sfx.frequency = 1040.0f;
    sfx.volume = 0.24f;
    sfx.attack = 0.0f;
    sfx.decay = 0.025f;
    sfx.sustain = 0.9f;
    sfx.duration = 0.04f;
    sfx.release = 0.04f;
    sfx.pitch_slide = -5200.0f;
    sfx.vibrato_depth = 18.0f;
    sfx.vibrato_rate = 35.0f;
    sfx.duty = 0.18f;
    sfx.duty_slide = 2.2f;
    sfx.filter = R2D_FILTER_BANDPASS;
    sfx.filter_cutoff = 1800.0f;
    sfx.filter_cutoff_slide = -1200.0f;
    sfx.filter_resonance = 0.22f;

    return sfx;
}

R2D_Sfx R2D_SfxHit(void)
{
    R2D_Sfx sfx = R2D_DefaultSfx();

    sfx.waveform = R2D_WAVE_NOISE;
    sfx.frequency = 2600.0f;
    sfx.volume = 0.34f;
    sfx.attack = 0.0f;
    sfx.decay = 0.018f;
    sfx.sustain = 0.55f;
    sfx.duration = 0.025f;
    sfx.release = 0.05f;
    sfx.pitch_slide = -1800.0f;
    sfx.filter = R2D_FILTER_HIGHPASS;
    sfx.filter_cutoff = 900.0f;

    return sfx;
}

R2D_Sfx R2D_SfxExplosion(void)
{
    R2D_Sfx sfx = R2D_DefaultSfx();

    sfx.waveform = R2D_WAVE_NOISE;
    sfx.frequency = 950.0f;
    sfx.volume = 0.45f;
    sfx.attack = 0.0f;
    sfx.decay = 0.05f;
    sfx.sustain = 0.8f;
    sfx.duration = 0.12f;
    sfx.release = 0.18f;
    sfx.pitch_slide = -620.0f;
    sfx.vibrato_depth = 8.0f;
    sfx.vibrato_rate = 9.0f;
    sfx.filter = R2D_FILTER_LOWPASS;
    sfx.filter_cutoff = 2200.0f;
    sfx.filter_cutoff_slide = -3200.0f;
    sfx.filter_resonance = 0.08f;

    return sfx;
}

R2D_Sfx R2D_SfxPowerup(void)
{
    R2D_Sfx sfx = R2D_DefaultSfx();

    sfx.waveform = R2D_WAVE_SQUARE;
    sfx.frequency = 440.0f;
    sfx.volume = 0.26f;
    sfx.attack = 0.001f;
    sfx.decay = 0.025f;
    sfx.sustain = 0.75f;
    sfx.duration = 0.18f;
    sfx.release = 0.04f;
    sfx.pitch_slide = 540.0f;
    sfx.arpeggio_step_1 = 7.0f;
    sfx.arpeggio_step_2 = 12.0f;
    sfx.arpeggio_rate = 32.0f;
    sfx.duty = 0.25f;
    sfx.filter = R2D_FILTER_LOWPASS;
    sfx.filter_cutoff = 2600.0f;
    sfx.filter_resonance = 0.45f;

    return sfx;
}

bool R2D_LoadSfx(const char *path, R2D_Sfx *sfx)
{
    char *file_text;
    char *cursor;
    char line[256];
    R2D_Sfx loaded;

    if (path == 0 || sfx == 0) {
        return false;
    }

    file_text = R2D_LoadAssetText(path);
    if (file_text == 0) {
        return false;
    }

    loaded = R2D_DefaultSfx();
    cursor = file_text;

    while (*cursor != '\0') {
        int length = 0;
        char *text;
        char *separator;
        char *key;
        char *value;

        while (cursor[length] != '\0' && cursor[length] != '\n' && length < (int)sizeof(line) - 1) {
            line[length] = cursor[length];
            ++length;
        }

        line[length] = '\0';

        while (cursor[length] != '\0' && cursor[length] != '\n') {
            ++length;
        }

        cursor += length;
        if (*cursor == '\n') {
            ++cursor;
        }

        text = R2D_AudioTrim(line);

        if (*text == '\0' || *text == '#' || *text == ';') {
            continue;
        }

        separator = strchr(text, '=');

        if (separator == 0) {
            continue;
        }

        *separator = '\0';
        key = R2D_AudioTrim(text);
        value = R2D_AudioTrim(separator + 1);

        if (R2D_AudioStringEquals(key, "version")) {
            continue;
        }

        R2D_AudioApplySfxField(&loaded, key, value);
    }

    R2D_UnloadAssetText(file_text);
    *sfx = loaded;
    return true;
}

bool R2D_SaveSfx(const char *path, R2D_Sfx sfx)
{
    FILE *file = 0;

    if (path == 0 || !R2D_AudioOpenFile(&file, path, "wb")) {
        return false;
    }

    fprintf(file, "version=1\n");
    fprintf(file, "waveform=%s\n", R2D_AudioWaveformName(sfx.waveform));
    fprintf(file, "filter=%s\n", R2D_AudioFilterName(sfx.filter));
    fprintf(file, "frequency=%.6g\n", sfx.frequency);
    fprintf(file, "volume=%.6g\n", sfx.volume);
    fprintf(file, "pan=%.6g\n", sfx.pan);
    fprintf(file, "attack=%.6g\n", sfx.attack);
    fprintf(file, "decay=%.6g\n", sfx.decay);
    fprintf(file, "sustain=%.6g\n", sfx.sustain);
    fprintf(file, "duration=%.6g\n", sfx.duration);
    fprintf(file, "release=%.6g\n", sfx.release);
    fprintf(file, "pitch_slide=%.6g\n", sfx.pitch_slide);
    fprintf(file, "vibrato_depth=%.6g\n", sfx.vibrato_depth);
    fprintf(file, "vibrato_rate=%.6g\n", sfx.vibrato_rate);
    fprintf(file, "arpeggio_step_1=%.6g\n", sfx.arpeggio_step_1);
    fprintf(file, "arpeggio_step_2=%.6g\n", sfx.arpeggio_step_2);
    fprintf(file, "arpeggio_rate=%.6g\n", sfx.arpeggio_rate);
    fprintf(file, "duty=%.6g\n", sfx.duty);
    fprintf(file, "duty_slide=%.6g\n", sfx.duty_slide);
    fprintf(file, "filter_cutoff=%.6g\n", sfx.filter_cutoff);
    fprintf(file, "filter_cutoff_slide=%.6g\n", sfx.filter_cutoff_slide);
    fprintf(file, "filter_resonance=%.6g\n", sfx.filter_resonance);

    return fclose(file) == 0;
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
    sfx.frequency = R2D_AudioClamp(sfx.frequency, 20.0f, 22000.0f);
    sfx.filter_cutoff = R2D_AudioClamp(sfx.filter_cutoff, 20.0f, 20000.0f);
    sfx.filter_cutoff_slide = R2D_AudioClamp(sfx.filter_cutoff_slide, -20000.0f, 20000.0f);
    sfx.filter_resonance = R2D_AudioClamp(sfx.filter_resonance, 0.0f, 1.0f);
    sfx.attack = fmaxf(0.0f, sfx.attack);
    sfx.decay = fmaxf(0.0f, sfx.decay);
    sfx.sustain = R2D_AudioClamp(sfx.sustain, 0.0f, 1.0f);
    sfx.duration = fmaxf(0.0f, sfx.duration);
    sfx.release = fmaxf(0.001f, sfx.release);
    sfx.vibrato_depth = fmaxf(0.0f, sfx.vibrato_depth);
    sfx.vibrato_rate = fmaxf(0.0f, sfx.vibrato_rate);
    sfx.arpeggio_step_1 = R2D_AudioClamp(sfx.arpeggio_step_1, -48.0f, 48.0f);
    sfx.arpeggio_step_2 = R2D_AudioClamp(sfx.arpeggio_step_2, -48.0f, 48.0f);
    sfx.arpeggio_rate = fmaxf(0.0f, sfx.arpeggio_rate);
    sfx.duty = R2D_AudioClamp(sfx.duty, 0.01f, 0.99f);

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
        voice->filter_low = 0.0f;
        voice->filter_band = 0.0f;
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
