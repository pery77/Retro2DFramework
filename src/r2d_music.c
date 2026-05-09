#include "r2d/r2d.h"

#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4244)
#pragma warning(disable:4701)
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#define R2D_MUSIC_CHANNELS 2
#define R2D_MUSIC_SAMPLE_SIZE 16
#define R2D_MUSIC_BUFFER_FRAMES 1024
#define R2D_MUSIC_RELEASE_MS 1000

static char r2d_music_last_error[256] = "";

typedef struct R2D_MusicState {
    AudioStream stream;
    tsf *soundfont;
    tml_message *first_message;
    tml_message *next_message;
    double position_ms;
    unsigned int length_ms;
    float volume;
    float channel_volume[16];
    float channel_activity[16];
    bool channel_used[16];
    bool channel_muted[16];
    bool channel_program_override[16];
    bool channel_bank_override[16];
    int channel_bank[16];
    int channel_program[16];
    bool loop;
    bool playing;
    bool paused;
} R2D_MusicState;

static R2D_MusicState *R2D_MusicGetState(R2D_Music *music)
{
    return music != 0 ? (R2D_MusicState *)music->state : 0;
}

static const R2D_MusicState *R2D_MusicGetConstState(const R2D_Music *music)
{
    return music != 0 ? (const R2D_MusicState *)music->state : 0;
}

static void R2D_MusicSetError(const char *message, const char *path)
{
    if (message == 0) {
        message = "";
    }

    if (path == 0 || path[0] == '\0') {
        snprintf(r2d_music_last_error, sizeof(r2d_music_last_error), "%s", message);
    } else {
        snprintf(r2d_music_last_error, sizeof(r2d_music_last_error), "%s: %s", message, path);
    }
}

static float R2D_MusicClamp(float value, float min, float max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

static void R2D_MusicApplyMessage(R2D_MusicState *state, const tml_message *message)
{
    const int channel = (int)message->channel;
    const int key = (int)(unsigned char)message->key;
    const int velocity = (int)(unsigned char)message->velocity;

    if (channel < 0 || channel >= 16) {
        return;
    }

    switch (message->type) {
        case TML_NOTE_ON:
            if (velocity == 0) {
                tsf_channel_note_off(state->soundfont, channel, key);
            } else if (!state->channel_muted[channel]) {
                tsf_channel_note_on(
                    state->soundfont,
                    channel,
                    key,
                    ((float)velocity / 127.0f) * state->channel_volume[channel]
                );
                state->channel_activity[channel] = 1.0f;
            }
            break;
        case TML_NOTE_OFF:
            tsf_channel_note_off(state->soundfont, channel, key);
            break;
        case TML_PROGRAM_CHANGE:
            if (!state->channel_program_override[channel]) {
                state->channel_program[channel] = (int)(unsigned char)message->program;
            }
            if (state->channel_bank_override[channel]) {
                tsf_channel_set_bank_preset(
                    state->soundfont,
                    channel,
                    state->channel_bank[channel],
                    state->channel_program[channel]
                );
            } else {
                tsf_channel_set_presetnumber(
                    state->soundfont,
                    channel,
                    state->channel_program[channel],
                    channel == 9
                );
            }
            break;
        case TML_CONTROL_CHANGE:
            tsf_channel_midi_control(
                state->soundfont,
                channel,
                (int)(unsigned char)message->control,
                (int)(unsigned char)message->control_value
            );
            break;
        case TML_PITCH_BEND:
            tsf_channel_set_pitchwheel(state->soundfont, channel, (int)message->pitch_bend);
            break;
        default:
            break;
    }
}

static void R2D_MusicScanChannels(R2D_MusicState *state)
{
    const tml_message *message = state->first_message;

    for (int channel = 0; channel < 16; ++channel) {
        state->channel_program[channel] = 0;
        state->channel_bank[channel] = channel == 9 ? 128 : 0;
        state->channel_volume[channel] = 1.0f;
    }

    while (message != 0) {
        const int channel = (int)message->channel;

        if (channel >= 0 && channel < 16) {
            switch (message->type) {
                case TML_NOTE_ON:
                case TML_NOTE_OFF:
                case TML_PROGRAM_CHANGE:
                case TML_CONTROL_CHANGE:
                case TML_PITCH_BEND:
                    state->channel_used[channel] = true;
                    break;
                default:
                    break;
            }

            if (message->type == TML_PROGRAM_CHANGE) {
                state->channel_program[channel] = (int)(unsigned char)message->program;
            }
        }

        message = message->next;
    }
}

static void R2D_MusicSetupChannels(tsf *soundfont)
{
    for (int channel = 0; channel < 16; ++channel) {
        tsf_channel_set_presetnumber(soundfont, channel, 0, channel == 9);
    }
}

static void R2D_MusicRewind(R2D_MusicState *state)
{
    if (state->soundfont != 0) {
        tsf_reset(state->soundfont);
        tsf_set_volume(state->soundfont, state->volume);
        R2D_MusicSetupChannels(state->soundfont);

        for (int channel = 0; channel < 16; ++channel) {
            if (state->channel_program_override[channel] || state->channel_bank_override[channel]) {
                tsf_channel_set_bank_preset(
                    state->soundfont,
                    channel,
                    state->channel_bank[channel],
                    state->channel_program[channel]
                );
            }
        }
    }

    state->next_message = state->first_message;
    state->position_ms = 0.0;
}

static void R2D_MusicRender(R2D_MusicState *state, short *samples, int frames)
{
    int rendered = 0;

    memset(samples, 0, (size_t)frames * R2D_MUSIC_CHANNELS * sizeof(short));

    while (rendered < frames && state->playing && !state->paused) {
        tml_message *message = state->next_message;
        int chunk_frames = frames - rendered;

        while (message != 0 && (double)message->time <= state->position_ms) {
            R2D_MusicApplyMessage(state, message);
            message = message->next;
            state->next_message = message;
        }

        if (message != 0) {
            const double event_ms = (double)message->time;
            const int event_frames = (int)((event_ms - state->position_ms) * R2D_AUDIO_SAMPLE_RATE / 1000.0);

            if (event_frames > 0 && event_frames < chunk_frames) {
                chunk_frames = event_frames;
            }
        } else if (state->position_ms >= (double)(state->length_ms + R2D_MUSIC_RELEASE_MS)) {
            if (state->loop) {
                R2D_MusicRewind(state);
                continue;
            }

            state->playing = false;
            break;
        }

        if (chunk_frames <= 0) {
            chunk_frames = 1;
        }

        tsf_render_short(
            state->soundfont,
            samples + rendered * R2D_MUSIC_CHANNELS,
            chunk_frames,
            0
        );

        rendered += chunk_frames;
        state->position_ms += (double)chunk_frames * 1000.0 / (double)R2D_AUDIO_SAMPLE_RATE;

        for (int channel = 0; channel < 16; ++channel) {
            state->channel_activity[channel] -= (float)chunk_frames / (float)R2D_AUDIO_SAMPLE_RATE * 8.0f;

            if (state->channel_activity[channel] < 0.0f) {
                state->channel_activity[channel] = 0.0f;
            }
        }
    }
}

bool R2D_MusicLoad(R2D_Music *music, const char *midi_path, const char *soundfont_path)
{
    tsf *soundfont;
    tml_message *messages;
    R2D_MusicState *state;
    AudioStream stream;
    unsigned int length_ms = 0;

    if (music == 0 || midi_path == 0 || soundfont_path == 0) {
        R2D_MusicSetError("invalid music load args", 0);
        return false;
    }

    if (music->state != 0) {
        R2D_MusicUnload(music);
    }

    memset(music, 0, sizeof(*music));

    if (!R2D_AudioIsReady() && !R2D_AudioInit()) {
        R2D_MusicSetError("audio init failed", 0);
        return false;
    }

    state = (R2D_MusicState *)calloc(1, sizeof(*state));
    if (state == 0) {
        R2D_MusicSetError("music state allocation failed", 0);
        return false;
    }

    soundfont = tsf_load_filename(soundfont_path);
    if (soundfont == 0) {
        R2D_MusicSetError("soundfont load failed", soundfont_path);
        TraceLog(LOG_WARNING, "R2D: Failed to load SoundFont: %s", soundfont_path);
        free(state);
        return false;
    }

    messages = tml_load_filename(midi_path);
    if (messages == 0) {
        R2D_MusicSetError("midi load failed", midi_path);
        TraceLog(LOG_WARNING, "R2D: Failed to load MIDI: %s", midi_path);
        tsf_close(soundfont);
        free(state);
        return false;
    }

    SetAudioStreamBufferSizeDefault(R2D_MUSIC_BUFFER_FRAMES);
    stream = LoadAudioStream(R2D_AUDIO_SAMPLE_RATE, R2D_MUSIC_SAMPLE_SIZE, R2D_MUSIC_CHANNELS);
    if (!IsAudioStreamValid(stream)) {
        R2D_MusicSetError("music stream creation failed", 0);
        TraceLog(LOG_WARNING, "R2D: Failed to create music stream");
        tml_free(messages);
        tsf_close(soundfont);
        free(state);
        return false;
    }

    tml_get_info(messages, 0, 0, 0, 0, &length_ms);
    tsf_set_output(soundfont, TSF_STEREO_INTERLEAVED, R2D_AUDIO_SAMPLE_RATE, 0.0f);
    tsf_set_max_voices(soundfont, 64);

    state->stream = stream;
    state->soundfont = soundfont;
    state->first_message = messages;
    state->next_message = messages;
    state->length_ms = length_ms;
    state->volume = 0.65f;
    R2D_MusicScanChannels(state);
    music->state = state;

    tsf_set_volume(soundfont, state->volume);
    PlayAudioStream(state->stream);

    TraceLog(LOG_INFO, "R2D: MIDI music loaded: %s", midi_path);
    R2D_MusicSetError("", 0);
    return true;
}

void R2D_MusicUnload(R2D_Music *music)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0) {
        return;
    }

    StopAudioStream(state->stream);
    UnloadAudioStream(state->stream);
    tml_free(state->first_message);
    tsf_close(state->soundfont);
    free(state);
    memset(music, 0, sizeof(*music));
}

void R2D_MusicPlay(R2D_Music *music, bool loop)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0) {
        return;
    }

    state->loop = loop;
    state->playing = true;
    state->paused = false;
    R2D_MusicRewind(state);
    PlayAudioStream(state->stream);
}

void R2D_MusicStop(R2D_Music *music)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0) {
        return;
    }

    state->playing = false;
    state->paused = false;
    R2D_MusicRewind(state);
}

void R2D_MusicPause(R2D_Music *music)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0 || !state->playing) {
        return;
    }

    state->paused = true;
}

void R2D_MusicResume(R2D_Music *music)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0 || !state->playing) {
        return;
    }

    state->paused = false;
}

void R2D_MusicUpdate(R2D_Music *music)
{
    R2D_MusicState *state = R2D_MusicGetState(music);
    short samples[R2D_MUSIC_BUFFER_FRAMES * R2D_MUSIC_CHANNELS];

    if (state == 0) {
        return;
    }

    while (IsAudioStreamProcessed(state->stream)) {
        R2D_MusicRender(state, samples, R2D_MUSIC_BUFFER_FRAMES);
        UpdateAudioStream(state->stream, samples, R2D_MUSIC_BUFFER_FRAMES);
    }
}

void R2D_MusicSetVolume(R2D_Music *music, float volume)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0) {
        return;
    }

    state->volume = R2D_MusicClamp(volume, 0.0f, 1.0f);
    tsf_set_volume(state->soundfont, state->volume);
}

void R2D_MusicSetLoop(R2D_Music *music, bool loop)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state != 0) {
        state->loop = loop;
    }
}

bool R2D_MusicIsPlaying(const R2D_Music *music)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    return state != 0 && state->playing && !state->paused;
}

bool R2D_MusicIsPaused(const R2D_Music *music)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    return state != 0 && state->playing && state->paused;
}

float R2D_MusicPosition(const R2D_Music *music)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    if (state == 0) {
        return 0.0f;
    }

    return (float)(state->position_ms / 1000.0);
}

float R2D_MusicLength(const R2D_Music *music)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    if (state == 0) {
        return 0.0f;
    }

    return (float)state->length_ms / 1000.0f;
}

const char *R2D_MusicLastError(void)
{
    return r2d_music_last_error;
}

bool R2D_MusicChannelUsed(const R2D_Music *music, int channel)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return false;
    }

    return state->channel_used[channel];
}

void R2D_MusicSetChannelMuted(R2D_Music *music, int channel, bool muted)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return;
    }

    state->channel_muted[channel] = muted;

    if (muted) {
        tsf_channel_sounds_off_all(state->soundfont, channel);
    }
}

bool R2D_MusicChannelMuted(const R2D_Music *music, int channel)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return false;
    }

    return state->channel_muted[channel];
}

int R2D_MusicChannelProgram(const R2D_Music *music, int channel)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return 0;
    }

    return state->channel_program[channel];
}

void R2D_MusicSetChannelVolume(R2D_Music *music, int channel, float volume)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return;
    }

    state->channel_volume[channel] = R2D_MusicClamp(volume, 0.0f, 1.0f);
}

float R2D_MusicChannelVolume(const R2D_Music *music, int channel)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return 0.0f;
    }

    return state->channel_volume[channel];
}

void R2D_MusicSetChannelProgram(R2D_Music *music, int channel, int program)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return;
    }

    if (program < 0) {
        program = 0;
    }

    if (program > 127) {
        program = 127;
    }

    state->channel_program[channel] = program;
    state->channel_program_override[channel] = true;
    tsf_channel_set_bank_preset(state->soundfont, channel, state->channel_bank[channel], program);
    tsf_channel_sounds_off_all(state->soundfont, channel);
}

void R2D_MusicSetChannelBank(R2D_Music *music, int channel, int bank)
{
    R2D_MusicState *state = R2D_MusicGetState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return;
    }

    if (bank < 0) {
        bank = 0;
    }

    if (bank > 255) {
        bank = 255;
    }

    state->channel_bank[channel] = bank;
    state->channel_bank_override[channel] = true;
    tsf_channel_set_bank_preset(state->soundfont, channel, bank, state->channel_program[channel]);
    tsf_channel_sounds_off_all(state->soundfont, channel);
}

int R2D_MusicChannelBank(const R2D_Music *music, int channel)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return 0;
    }

    return state->channel_bank[channel];
}

float R2D_MusicChannelActivity(const R2D_Music *music, int channel)
{
    const R2D_MusicState *state = R2D_MusicGetConstState(music);

    if (state == 0 || channel < 0 || channel >= 16) {
        return 0.0f;
    }

    return state->channel_activity[channel];
}
