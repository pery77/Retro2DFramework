#ifndef R2D_H
#define R2D_H

#include "raylib.h"

#define R2D_DEFAULT_VIRTUAL_WIDTH 320
#define R2D_DEFAULT_VIRTUAL_HEIGHT 200
#define R2D_DEFAULT_WINDOW_SCALE 4
#define R2D_AUDIO_SAMPLE_RATE 48000
#define R2D_AUDIO_MAX_VOICES 16

#ifdef __cplusplus
extern "C" {
#endif

typedef struct R2D_Config {
    int virtual_width;
    int virtual_height;
    int window_scale;
    const char *title;
    Color clear_color;
} R2D_Config;

typedef struct R2D_App {
    void (*init)(void *user_data);
    void (*update)(float dt, void *user_data);
    void (*draw)(void *user_data);
    void (*shutdown)(void *user_data);
    void *user_data;
} R2D_App;

typedef struct R2D_Crt {
    Shader shader;
    Texture2D noise;
    int resolution_loc;
    int virtual_resolution_loc;
    int noise_loc;
    int random_loc;
    bool enabled;
    bool is_ready;
} R2D_Crt;

typedef enum R2D_Waveform {
    R2D_WAVE_SQUARE = 0,
    R2D_WAVE_TRIANGLE,
    R2D_WAVE_SAW,
    R2D_WAVE_RAMP,
    R2D_WAVE_NOISE,
    R2D_WAVE_SINE
} R2D_Waveform;

typedef enum R2D_Filter {
    R2D_FILTER_NONE = 0,
    R2D_FILTER_LOWPASS,
    R2D_FILTER_HIGHPASS,
    R2D_FILTER_BANDPASS
} R2D_Filter;

typedef struct R2D_Sfx {
    R2D_Waveform waveform;
    R2D_Filter filter;
    float frequency;
    float volume;
    float pan;
    float attack;
    float decay;
    float sustain;
    float duration;
    float release;
    float pitch_slide;
    float vibrato_depth;
    float vibrato_rate;
    float arpeggio_step_1;
    float arpeggio_step_2;
    float arpeggio_rate;
    float duty;
    float duty_slide;
    float filter_cutoff;
    float filter_cutoff_slide;
    float filter_resonance;
} R2D_Sfx;

typedef struct R2D_Context {
    R2D_Config config;
    RenderTexture2D target;
    Rectangle source;
    Rectangle destination;
    Vector2 origin;
    int windowed_width;
    int windowed_height;
    Vector2 windowed_position;
    R2D_Crt *crt;
    bool screenshot_requested;
    bool is_ready;
} R2D_Context;

R2D_Config R2D_DefaultConfig(void);

bool R2D_Init(R2D_Context *ctx, R2D_Config config);
void R2D_Run(R2D_Context *ctx, R2D_App app);
void R2D_Close(R2D_Context *ctx);

void R2D_BeginFrame(R2D_Context *ctx);
void R2D_EndFrame(R2D_Context *ctx);

void R2D_ToggleFullscreen(R2D_Context *ctx);
void R2D_TakeScreenshot(void);

bool R2D_AudioInit(void);
void R2D_AudioClose(void);
bool R2D_AudioIsReady(void);
void R2D_AudioSetMasterVolume(float volume);
float R2D_AudioMasterVolume(void);
R2D_Sfx R2D_DefaultSfx(void);
R2D_Sfx R2D_SfxCoin(void);
R2D_Sfx R2D_SfxJump(void);
R2D_Sfx R2D_SfxLaser(void);
R2D_Sfx R2D_SfxHit(void);
R2D_Sfx R2D_SfxExplosion(void);
R2D_Sfx R2D_SfxPowerup(void);
bool R2D_LoadSfx(const char *path, R2D_Sfx *sfx);
bool R2D_SaveSfx(const char *path, R2D_Sfx sfx);
void R2D_PlaySfx(R2D_Sfx sfx);
void R2D_PlayTone(R2D_Waveform waveform, float frequency, float duration);

bool R2D_CrtInit(R2D_Crt *crt);
bool R2D_CrtReload(R2D_Crt *crt);
void R2D_CrtClose(R2D_Crt *crt);
void R2D_CrtSetEnabled(R2D_Crt *crt, bool enabled);
void R2D_SetCrt(R2D_Context *ctx, R2D_Crt *crt);

const char *R2D_AssetPath(const char *relative_path);

int R2D_VirtualWidth(const R2D_Context *ctx);
int R2D_VirtualHeight(const R2D_Context *ctx);
Vector2 R2D_VirtualSize(const R2D_Context *ctx);
Vector2 R2D_MouseVirtualPosition(const R2D_Context *ctx);
Rectangle R2D_Rect(float x, float y, float width, float height);

void R2D_DrawSprite(Texture2D texture, Rectangle source, Vector2 position, bool flip_x);
Color R2D_ColorFromHex(unsigned int rgba);

#ifdef __cplusplus
}
#endif

#endif
