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

typedef struct R2D_Music {
    void *state;
} R2D_Music;

typedef struct R2D_Sprite {
    Texture2D texture;
    Rectangle source;
    Vector2 origin;
} R2D_Sprite;

typedef struct R2D_SpriteSheet {
    Texture2D texture;
    int frame_width;
    int frame_height;
    int columns;
    int rows;
} R2D_SpriteSheet;

typedef struct R2D_Anim {
    int first_frame;
    int frame_count;
    float fps;
    bool loop;
} R2D_Anim;

typedef struct R2D_AnimPlayer {
    R2D_Anim anim;
    float time;
    int frame;
    bool playing;
} R2D_AnimPlayer;

typedef struct R2D_Camera {
    Vector2 position;
    int viewport_width;
    int viewport_height;
} R2D_Camera;

typedef struct R2D_TilemapLayer {
    char name[64];
    unsigned int *tiles;
    int width;
    int height;
    bool visible;
} R2D_TilemapLayer;

typedef struct R2D_TilemapObject {
    char name[64];
    char type[64];
    Rectangle rect;
} R2D_TilemapObject;

typedef struct R2D_Tilemap {
    Texture2D texture;
    R2D_TilemapLayer *layers;
    R2D_TilemapObject *objects;
    int layer_count;
    int object_count;
    int width;
    int height;
    int tile_width;
    int tile_height;
    int first_gid;
    int columns;
    int tile_count;
    bool is_ready;
} R2D_Tilemap;

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

bool R2D_MusicLoad(R2D_Music *music, const char *midi_path, const char *soundfont_path);
bool R2D_MusicLoadSong(R2D_Music *music, const char *song_path);
void R2D_MusicUnload(R2D_Music *music);
void R2D_MusicPlay(R2D_Music *music, bool loop);
void R2D_MusicStop(R2D_Music *music);
void R2D_MusicPause(R2D_Music *music);
void R2D_MusicResume(R2D_Music *music);
void R2D_MusicUpdate(R2D_Music *music);
void R2D_MusicSetVolume(R2D_Music *music, float volume);
void R2D_MusicSetLoop(R2D_Music *music, bool loop);
bool R2D_MusicIsPlaying(const R2D_Music *music);
bool R2D_MusicIsPaused(const R2D_Music *music);
float R2D_MusicPosition(const R2D_Music *music);
float R2D_MusicLength(const R2D_Music *music);
bool R2D_MusicChannelUsed(const R2D_Music *music, int channel);
void R2D_MusicSetChannelMuted(R2D_Music *music, int channel, bool muted);
bool R2D_MusicChannelMuted(const R2D_Music *music, int channel);
void R2D_MusicSetChannelVolume(R2D_Music *music, int channel, float volume);
float R2D_MusicChannelVolume(const R2D_Music *music, int channel);
void R2D_MusicSetChannelProgram(R2D_Music *music, int channel, int program);
int R2D_MusicChannelProgram(const R2D_Music *music, int channel);
void R2D_MusicSetChannelBank(R2D_Music *music, int channel, int bank);
int R2D_MusicChannelBank(const R2D_Music *music, int channel);
float R2D_MusicChannelActivity(const R2D_Music *music, int channel);
const char *R2D_MusicLastError(void);

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

R2D_Camera R2D_CameraCreate(int viewport_width, int viewport_height);
void R2D_CameraFollow(R2D_Camera *camera, Vector2 target);
void R2D_CameraClampToRect(R2D_Camera *camera, Rectangle bounds);
Vector2 R2D_CameraPixelPosition(const R2D_Camera *camera);
Vector2 R2D_CameraWorldToScreen(const R2D_Camera *camera, Vector2 world);
Vector2 R2D_CameraScreenToWorld(const R2D_Camera *camera, Vector2 screen);
Rectangle R2D_CameraView(const R2D_Camera *camera);
R2D_SpriteSheet R2D_LoadSpriteSheet(const char *path, int frame_width, int frame_height);
R2D_SpriteSheet R2D_SpriteSheetFromTexture(Texture2D texture, int frame_width, int frame_height);
void R2D_UnloadSpriteSheet(R2D_SpriteSheet *sheet);
bool R2D_SpriteSheetIsReady(const R2D_SpriteSheet *sheet);
int R2D_SpriteSheetFrameCount(const R2D_SpriteSheet *sheet);
Rectangle R2D_SpriteSheetFrame(const R2D_SpriteSheet *sheet, int frame);
R2D_Anim R2D_AnimFrames(int first_frame, int frame_count, float fps, bool loop);
void R2D_AnimPlay(R2D_AnimPlayer *player, R2D_Anim anim);
void R2D_AnimStop(R2D_AnimPlayer *player);
void R2D_AnimUpdate(R2D_AnimPlayer *player, float dt);
int R2D_AnimFrame(const R2D_AnimPlayer *player);
void R2D_DrawSprite(Texture2D texture, Rectangle source, Vector2 position, bool flip_x);
void R2D_DrawSpriteEx(Texture2D texture, Rectangle source, Vector2 position, Vector2 origin, float rotation, float scale, bool flip_x, Color tint);
void R2D_DrawSheetFrame(const R2D_SpriteSheet *sheet, int frame, Vector2 position, bool flip_x);
void R2D_DrawAnim(const R2D_SpriteSheet *sheet, const R2D_AnimPlayer *player, Vector2 position, bool flip_x);
bool R2D_TilemapLoadTiledJson(R2D_Tilemap *tilemap, const char *path);
void R2D_TilemapUnload(R2D_Tilemap *tilemap);
bool R2D_TilemapIsReady(const R2D_Tilemap *tilemap);
int R2D_TilemapLayerIndex(const R2D_Tilemap *tilemap, const char *name);
unsigned int R2D_TilemapTileAt(const R2D_Tilemap *tilemap, int layer_index, int x, int y);
Vector2 R2D_TilemapWorldToTile(const R2D_Tilemap *tilemap, Vector2 position);
Rectangle R2D_TilemapTileBounds(const R2D_Tilemap *tilemap, int x, int y);
bool R2D_TilemapSolidAt(const R2D_Tilemap *tilemap, int layer_index, Vector2 position);
bool R2D_TilemapRectCollides(const R2D_Tilemap *tilemap, int layer_index, Rectangle rect);
int R2D_TilemapObjectCount(const R2D_Tilemap *tilemap);
const R2D_TilemapObject *R2D_TilemapObjectAt(const R2D_Tilemap *tilemap, int index);
const R2D_TilemapObject *R2D_TilemapFindObject(const R2D_Tilemap *tilemap, const char *name);
const R2D_TilemapObject *R2D_TilemapFindObjectByType(const R2D_Tilemap *tilemap, const char *type);
void R2D_TilemapDraw(const R2D_Tilemap *tilemap, Vector2 position);
void R2D_TilemapDrawLayer(const R2D_Tilemap *tilemap, int layer_index, Vector2 position);
void R2D_TilemapDrawVisible(const R2D_Tilemap *tilemap, Rectangle view, Vector2 position);
void R2D_TilemapDrawLayerVisible(const R2D_Tilemap *tilemap, int layer_index, Rectangle view, Vector2 position);
void R2D_TilemapDrawCollisionDebug(const R2D_Tilemap *tilemap, int layer_index, Vector2 position, Color color);
void R2D_TilemapDrawCollisionDebugVisible(const R2D_Tilemap *tilemap, int layer_index, Rectangle view, Vector2 position, Color color);
void R2D_TilemapDrawObjectsDebug(const R2D_Tilemap *tilemap, Vector2 position, Color color);
Color R2D_ColorFromHex(unsigned int rgba);

#ifdef __cplusplus
}
#endif

#endif
