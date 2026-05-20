#ifndef R2D_H
#define R2D_H

#include "raylib.h"

#define R2D_DEFAULT_VIRTUAL_WIDTH 320
#define R2D_DEFAULT_VIRTUAL_HEIGHT 200
#define R2D_DEFAULT_WINDOW_SCALE 4
#define R2D_AUDIO_SAMPLE_RATE 48000
#define R2D_AUDIO_MAX_VOICES 16
#define R2D_INPUT_MAX_ACTIONS 32
#define R2D_INPUT_MAX_BINDINGS 8
#define R2D_INPUT_ACTION_NAME_SIZE 32
#define R2D_STATE_STACK_MAX 8
#define R2D_COLLISION_MAX_HITS 8
#define R2D_ENTITY_MAX 256
#define R2D_PARTICLE_MAX 512
#define R2D_TIMER_MAX 64
#define R2D_TWEEN_MAX 64
#define R2D_PALETTE_MAX_COLORS 32
#define R2D_ASSET_CACHE_MAX_TEXTURES 128
#define R2D_ASSET_CACHE_MAX_SHADERS 32
#define R2D_ASSET_CACHE_PATH_SIZE 256
#define R2D_SAVE_PATH_SIZE 512

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

typedef enum R2D_InputSource {
    R2D_INPUT_KEY = 0,
    R2D_INPUT_MOUSE_BUTTON,
    R2D_INPUT_GAMEPAD_BUTTON,
    R2D_INPUT_GAMEPAD_AXIS_NEGATIVE,
    R2D_INPUT_GAMEPAD_AXIS_POSITIVE
} R2D_InputSource;

typedef struct R2D_InputBinding {
    R2D_InputSource source;
    int code;
    float deadzone;
} R2D_InputBinding;

typedef struct R2D_InputAction {
    char name[R2D_INPUT_ACTION_NAME_SIZE];
    R2D_InputBinding bindings[R2D_INPUT_MAX_BINDINGS];
    int binding_count;
    bool down;
    bool previous_down;
    float value;
    float previous_value;
} R2D_InputAction;

typedef struct R2D_InputMap {
    R2D_InputAction actions[R2D_INPUT_MAX_ACTIONS];
    int action_count;
    int gamepad;
    float default_deadzone;
} R2D_InputMap;

typedef void (*R2D_StateEnterCallback)(void *state_data, void *user_data);
typedef void (*R2D_StateUpdateCallback)(float dt, void *state_data, void *user_data);
typedef void (*R2D_StateDrawCallback)(void *state_data, void *user_data);
typedef void (*R2D_StateExitCallback)(void *state_data, void *user_data);

typedef struct R2D_State {
    const char *name;
    R2D_StateEnterCallback enter;
    R2D_StateUpdateCallback update;
    R2D_StateDrawCallback draw;
    R2D_StateExitCallback exit;
    void *data;
} R2D_State;

typedef struct R2D_StateMachine {
    R2D_State stack[R2D_STATE_STACK_MAX];
    int count;
    void *user_data;
} R2D_StateMachine;

typedef enum R2D_TextAlign {
    R2D_TEXT_ALIGN_LEFT = 0,
    R2D_TEXT_ALIGN_CENTER,
    R2D_TEXT_ALIGN_RIGHT
} R2D_TextAlign;

typedef struct R2D_TextStyle {
    Font font;
    int font_size;
    int spacing;
    int line_spacing;
    Color tint;
    Color shadow;
    Color outline;
    Vector2 shadow_offset;
    bool use_shadow;
    bool use_outline;
} R2D_TextStyle;

typedef struct R2D_UiStyle {
    Color panel;
    Color border;
    Color text;
    Color accent;
    Color hot;
    Color disabled;
    Color fill;
    int font_size;
    int border_size;
} R2D_UiStyle;

typedef struct R2D_UiNav {
    int focus;
    int count;
    bool submitted;
    bool changed;
} R2D_UiNav;

typedef struct R2D_NineSlice {
    Texture2D texture;
    Rectangle source;
    int left;
    int top;
    int right;
    int bottom;
} R2D_NineSlice;

typedef struct R2D_Typewriter {
    const char *text;
    float chars_per_second;
    float timer;
    int visible_count;
    int text_length;
    bool done;
} R2D_Typewriter;

typedef struct R2D_Collider {
    Rectangle rect;
    unsigned int layer;
    unsigned int mask;
    bool trigger;
    void *user_data;
} R2D_Collider;

typedef struct R2D_CollisionHit {
    int collider_index;
    Rectangle rect;
    Vector2 normal;
    float penetration;
    bool trigger;
    void *user_data;
} R2D_CollisionHit;

typedef struct R2D_CollisionResult {
    Vector2 position;
    Vector2 movement;
    bool collided_x;
    bool collided_y;
    int hit_count;
    R2D_CollisionHit hits[R2D_COLLISION_MAX_HITS];
} R2D_CollisionResult;

typedef unsigned int R2D_EntityId;

typedef struct R2D_Entity R2D_Entity;

typedef void (*R2D_EntityUpdateCallback)(R2D_Entity *entity, float dt, void *world_data);
typedef void (*R2D_EntityDrawCallback)(const R2D_Entity *entity, void *world_data);

struct R2D_Entity {
    R2D_EntityId id;
    Vector2 position;
    Vector2 velocity;
    Rectangle bounds;
    int type;
    unsigned int layer;
    unsigned int flags;
    bool active;
    void *user_data;
    R2D_EntityUpdateCallback update;
    R2D_EntityDrawCallback draw;
};

typedef struct R2D_EntityWorld {
    R2D_Entity entities[R2D_ENTITY_MAX];
    unsigned int generations[R2D_ENTITY_MAX];
    int count;
    void *user_data;
} R2D_EntityWorld;

typedef enum R2D_ParticleShape {
    R2D_PARTICLE_SHAPE_PIXEL = 0,
    R2D_PARTICLE_SHAPE_CIRCLE
} R2D_ParticleShape;

typedef enum R2D_ParticlePreset {
    R2D_PARTICLE_PRESET_DUST = 0,
    R2D_PARTICLE_PRESET_HIT,
    R2D_PARTICLE_PRESET_SPARK,
    R2D_PARTICLE_PRESET_SMOKE,
    R2D_PARTICLE_PRESET_COIN,
    R2D_PARTICLE_PRESET_STAR
} R2D_ParticlePreset;

typedef struct R2D_Particle {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    float life;
    float age;
    float start_size;
    float end_size;
    Color start_color;
    Color end_color;
    R2D_ParticleShape shape;
    bool active;
} R2D_Particle;

typedef struct R2D_ParticleEmitter {
    Vector2 position;
    Vector2 velocity_min;
    Vector2 velocity_max;
    Vector2 acceleration;
    float life_min;
    float life_max;
    float size_min;
    float size_max;
    float end_size_min;
    float end_size_max;
    float emit_rate;
    float emit_timer;
    Color start_color;
    Color end_color;
    R2D_ParticleShape shape;
    bool active;
} R2D_ParticleEmitter;

typedef struct R2D_ParticleSystem {
    R2D_Particle particles[R2D_PARTICLE_MAX];
    int count;
} R2D_ParticleSystem;

typedef void (*R2D_TimerCallback)(void *user_data);

typedef struct R2D_Timer {
    float duration;
    float elapsed;
    int repeat_count;
    int fired_count;
    bool active;
    R2D_TimerCallback callback;
    void *user_data;
} R2D_Timer;

typedef struct R2D_TimerSystem {
    R2D_Timer timers[R2D_TIMER_MAX];
    int count;
} R2D_TimerSystem;

typedef enum R2D_Ease {
    R2D_EASE_LINEAR = 0,
    R2D_EASE_IN,
    R2D_EASE_OUT,
    R2D_EASE_IN_OUT
} R2D_Ease;

typedef struct R2D_Tween {
    float *target;
    float start;
    float end;
    float duration;
    float elapsed;
    R2D_Ease ease;
    bool active;
} R2D_Tween;

typedef struct R2D_TweenSystem {
    R2D_Tween tweens[R2D_TWEEN_MAX];
    int count;
} R2D_TweenSystem;

typedef struct R2D_Shake {
    float duration;
    float elapsed;
    float strength;
    Vector2 offset;
    bool active;
} R2D_Shake;

typedef struct R2D_TimeEffects {
    float hitstop_timer;
    float slow_timer;
    float slow_scale;
    float flash_timer;
    float flash_duration;
    float fade_alpha;
    Color flash_color;
} R2D_TimeEffects;

typedef struct R2D_Palette {
    Color colors[R2D_PALETTE_MAX_COLORS];
    int count;
} R2D_Palette;

typedef struct R2D_SaveData {
    int version;
    int window_scale;
    bool fullscreen;
    float master_volume;
    float music_volume;
    float sfx_volume;
    int progress;
    int high_score;
} R2D_SaveData;

typedef struct R2D_CachedTexture {
    char path[R2D_ASSET_CACHE_PATH_SIZE];
    Texture2D texture;
    int group;
    bool loaded;
} R2D_CachedTexture;

typedef struct R2D_CachedShader {
    char path[R2D_ASSET_CACHE_PATH_SIZE];
    Shader shader;
    int group;
    bool loaded;
} R2D_CachedShader;

typedef struct R2D_AssetCache {
    R2D_CachedTexture textures[R2D_ASSET_CACHE_MAX_TEXTURES];
    R2D_CachedShader shaders[R2D_ASSET_CACHE_MAX_SHADERS];
    int texture_count;
    int shader_count;
} R2D_AssetCache;

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

typedef struct R2D_TilemapTileset {
    Texture2D texture;
    int first_gid;
    int tile_width;
    int tile_height;
    int columns;
    int tile_count;
} R2D_TilemapTileset;

typedef struct R2D_Tilemap {
    Texture2D texture;
    R2D_TilemapTileset *tilesets;
    R2D_TilemapLayer *layers;
    R2D_TilemapObject *objects;
    int tileset_count;
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
    bool close_requested;
    bool is_ready;
} R2D_Context;

R2D_Config R2D_DefaultConfig(void);

bool R2D_Init(R2D_Context *ctx, R2D_Config config);
void R2D_Run(R2D_Context *ctx, R2D_App app);
void R2D_RequestClose(R2D_Context *ctx);
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
bool R2D_MountAssetPack(const char *path);
void R2D_UnmountAssetPack(void);
bool R2D_AssetPackMounted(void);
bool R2D_AssetExists(const char *path);
bool R2D_LoadAssetData(const char *path, unsigned char **data, int *size);
void R2D_UnloadAssetData(unsigned char *data);
char *R2D_LoadAssetText(const char *path);
void R2D_UnloadAssetText(char *text);
Texture2D R2D_LoadTexture(const char *path);
Shader R2D_LoadFragmentShader(const char *path);
void R2D_AssetCacheInit(R2D_AssetCache *cache);
void R2D_AssetCacheClear(R2D_AssetCache *cache);
void R2D_AssetCacheReleaseGroup(R2D_AssetCache *cache, int group);
Texture2D R2D_AssetCacheLoadTexture(R2D_AssetCache *cache, const char *path, int group);
Shader R2D_AssetCacheLoadFragmentShader(R2D_AssetCache *cache, const char *path, int group);
int R2D_AssetCacheTextureCount(const R2D_AssetCache *cache);
int R2D_AssetCacheShaderCount(const R2D_AssetCache *cache);

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
void R2D_InputInit(R2D_InputMap *input);
void R2D_InputClear(R2D_InputMap *input);
void R2D_InputSetGamepad(R2D_InputMap *input, int gamepad);
void R2D_InputSetDefaultDeadzone(R2D_InputMap *input, float deadzone);
int R2D_InputAddAction(R2D_InputMap *input, const char *name);
int R2D_InputFindAction(const R2D_InputMap *input, const char *name);
bool R2D_InputBindKey(R2D_InputMap *input, const char *action, int key);
bool R2D_InputBindMouseButton(R2D_InputMap *input, const char *action, int button);
bool R2D_InputBindGamepadButton(R2D_InputMap *input, const char *action, int button);
bool R2D_InputBindGamepadAxis(R2D_InputMap *input, const char *action, int axis, bool positive);
void R2D_InputUpdate(R2D_InputMap *input);
bool R2D_InputDown(const R2D_InputMap *input, const char *action);
bool R2D_InputPressed(const R2D_InputMap *input, const char *action);
bool R2D_InputReleased(const R2D_InputMap *input, const char *action);
float R2D_InputValue(const R2D_InputMap *input, const char *action);
float R2D_InputAxis(const R2D_InputMap *input, const char *negative_action, const char *positive_action);
void R2D_StateMachineInit(R2D_StateMachine *machine, void *user_data);
void R2D_StateMachineClear(R2D_StateMachine *machine);
bool R2D_StateMachineSet(R2D_StateMachine *machine, R2D_State state);
bool R2D_StateMachinePush(R2D_StateMachine *machine, R2D_State state);
bool R2D_StateMachinePop(R2D_StateMachine *machine);
void R2D_StateMachineUpdate(R2D_StateMachine *machine, float dt);
void R2D_StateMachineDraw(R2D_StateMachine *machine);
void R2D_StateMachineDrawStack(R2D_StateMachine *machine);
const R2D_State *R2D_StateMachineCurrent(const R2D_StateMachine *machine);
int R2D_StateMachineCount(const R2D_StateMachine *machine);
bool R2D_StateMachineIsEmpty(const R2D_StateMachine *machine);
const char *R2D_StateMachineCurrentName(const R2D_StateMachine *machine);
R2D_TextStyle R2D_DefaultTextStyle(int font_size, Color tint);
Font R2D_LoadBitmapFont(const char *path);
void R2D_UnloadBitmapFont(Font *font);
Font R2D_LoadFont(const char *path);
void R2D_UnloadFont(Font *font);
Vector2 R2D_MeasureTextStyled(const char *text, R2D_TextStyle style);
void R2D_DrawTextStyled(const char *text, Vector2 position, R2D_TextStyle style);
void R2D_DrawTextAligned(const char *text, Rectangle bounds, R2D_TextStyle style, R2D_TextAlign align);
void R2D_DrawTextWrapped(const char *text, Rectangle bounds, R2D_TextStyle style);
R2D_UiStyle R2D_DefaultUiStyle(void);
void R2D_UiNavInit(R2D_UiNav *nav, int item_count);
void R2D_UiNavSetCount(R2D_UiNav *nav, int item_count);
void R2D_UiNavMove(R2D_UiNav *nav, int delta);
void R2D_UiNavUpdate(R2D_UiNav *nav, const R2D_InputMap *input, const char *previous_action, const char *next_action, const char *submit_action);
bool R2D_UiNavSubmitted(const R2D_UiNav *nav, int item);
bool R2D_UiNavFocused(const R2D_UiNav *nav, int item);
R2D_NineSlice R2D_NineSliceCreate(Texture2D texture, Rectangle source, int left, int top, int right, int bottom);
void R2D_DrawUiPanel(Rectangle rect, R2D_UiStyle style);
void R2D_DrawUiNineSlice(R2D_NineSlice slice, Rectangle rect, Color tint);
void R2D_DrawUiButton(Rectangle rect, const char *text, bool focused, bool pressed, R2D_UiStyle style);
void R2D_DrawUiMenuItem(Rectangle rect, const char *text, bool focused, bool selected, R2D_UiStyle style);
void R2D_DrawUiToggle(Rectangle rect, const char *text, bool value, bool focused, R2D_UiStyle style);
void R2D_DrawUiSelector(Rectangle rect, const char *text, const char *value, bool focused, R2D_UiStyle style);
void R2D_DrawUiSlider(Rectangle rect, const char *text, float value, bool focused, R2D_UiStyle style);
void R2D_DrawUiBar(Rectangle rect, float value, Color fill, R2D_UiStyle style);
void R2D_DrawUiDialog(Rectangle rect, const char *title, const char *text, R2D_UiStyle style);
void R2D_TypewriterStart(R2D_Typewriter *typewriter, const char *text, float chars_per_second);
void R2D_TypewriterReset(R2D_Typewriter *typewriter);
void R2D_TypewriterUpdate(R2D_Typewriter *typewriter, float dt);
void R2D_TypewriterComplete(R2D_Typewriter *typewriter);
bool R2D_TypewriterDone(const R2D_Typewriter *typewriter);
void R2D_DrawTypewriter(R2D_Typewriter typewriter, Rectangle bounds, R2D_TextStyle style);
R2D_Collider R2D_ColliderRect(Rectangle rect, unsigned int layer, unsigned int mask, bool trigger, void *user_data);
bool R2D_AabbIntersects(Rectangle a, Rectangle b);
bool R2D_CollisionLayersMatch(unsigned int layer, unsigned int mask, unsigned int other_layer, unsigned int other_mask);
int R2D_CollisionQueryRect(Rectangle rect, unsigned int layer, unsigned int mask, const R2D_Collider *colliders, int collider_count, R2D_CollisionHit *hits, int max_hits);
int R2D_CollisionQueryPoint(Vector2 point, unsigned int layer, unsigned int mask, const R2D_Collider *colliders, int collider_count, R2D_CollisionHit *hits, int max_hits);
int R2D_CollisionQueryCircle(Vector2 center, float radius, unsigned int layer, unsigned int mask, const R2D_Collider *colliders, int collider_count, R2D_CollisionHit *hits, int max_hits);
Vector2 R2D_MoveAndSlide(Rectangle bounds, Vector2 movement, unsigned int layer, unsigned int mask, const R2D_Collider *colliders, int collider_count, R2D_CollisionResult *result);
void R2D_EntityWorldInit(R2D_EntityWorld *world, void *user_data);
void R2D_EntityWorldClear(R2D_EntityWorld *world);
R2D_Entity *R2D_EntitySpawn(R2D_EntityWorld *world, int type, Vector2 position);
bool R2D_EntityDestroy(R2D_EntityWorld *world, R2D_EntityId id);
bool R2D_EntityAlive(const R2D_EntityWorld *world, R2D_EntityId id);
R2D_Entity *R2D_EntityGet(R2D_EntityWorld *world, R2D_EntityId id);
const R2D_Entity *R2D_EntityGetConst(const R2D_EntityWorld *world, R2D_EntityId id);
int R2D_EntityCount(const R2D_EntityWorld *world);
R2D_Entity *R2D_EntityAt(R2D_EntityWorld *world, int active_index);
const R2D_Entity *R2D_EntityAtConst(const R2D_EntityWorld *world, int active_index);
R2D_Entity *R2D_EntityFindByType(R2D_EntityWorld *world, int type, int *cursor);
R2D_Entity *R2D_EntityFindByLayer(R2D_EntityWorld *world, unsigned int layer_mask, int *cursor);
void R2D_EntityWorldUpdate(R2D_EntityWorld *world, float dt);
void R2D_EntityWorldDraw(const R2D_EntityWorld *world);
void R2D_ParticleSystemInit(R2D_ParticleSystem *system);
void R2D_ParticleSystemClear(R2D_ParticleSystem *system);
R2D_ParticleEmitter R2D_ParticleEmitterPreset(R2D_ParticlePreset preset, Vector2 position);
bool R2D_ParticleEmit(R2D_ParticleSystem *system, const R2D_ParticleEmitter *emitter);
int R2D_ParticleBurst(R2D_ParticleSystem *system, const R2D_ParticleEmitter *emitter, int count);
void R2D_ParticleEmitterUpdate(R2D_ParticleSystem *system, R2D_ParticleEmitter *emitter, float dt);
void R2D_ParticleSystemUpdate(R2D_ParticleSystem *system, float dt);
void R2D_ParticleSystemDraw(const R2D_ParticleSystem *system);
int R2D_ParticleSystemAliveCount(const R2D_ParticleSystem *system);
float R2D_Clamp01(float value);
float R2D_Lerp(float a, float b, float t);
Color R2D_LerpColor(Color a, Color b, float t);
float R2D_EaseValue(R2D_Ease ease, float t);
void R2D_TimerSystemInit(R2D_TimerSystem *system);
void R2D_TimerSystemClear(R2D_TimerSystem *system);
int R2D_TimerAfter(R2D_TimerSystem *system, float delay, R2D_TimerCallback callback, void *user_data);
int R2D_TimerEvery(R2D_TimerSystem *system, float interval, int repeat_count, R2D_TimerCallback callback, void *user_data);
bool R2D_TimerCancel(R2D_TimerSystem *system, int timer_index);
void R2D_TimerSystemUpdate(R2D_TimerSystem *system, float dt);
int R2D_TimerSystemActiveCount(const R2D_TimerSystem *system);
void R2D_TweenSystemInit(R2D_TweenSystem *system);
void R2D_TweenSystemClear(R2D_TweenSystem *system);
int R2D_TweenFloat(R2D_TweenSystem *system, float *target, float end, float duration, R2D_Ease ease);
bool R2D_TweenCancel(R2D_TweenSystem *system, int tween_index);
void R2D_TweenSystemUpdate(R2D_TweenSystem *system, float dt);
int R2D_TweenSystemActiveCount(const R2D_TweenSystem *system);
void R2D_ShakeStart(R2D_Shake *shake, float duration, float strength);
void R2D_ShakeUpdate(R2D_Shake *shake, float dt);
Vector2 R2D_ShakeOffset(const R2D_Shake *shake);
void R2D_TimeEffectsInit(R2D_TimeEffects *effects);
float R2D_TimeEffectsUpdate(R2D_TimeEffects *effects, float dt);
void R2D_TimeEffectsHitstop(R2D_TimeEffects *effects, float duration);
void R2D_TimeEffectsSlowMotion(R2D_TimeEffects *effects, float duration, float scale);
void R2D_TimeEffectsFlash(R2D_TimeEffects *effects, float duration, Color color);
void R2D_TimeEffectsFade(R2D_TimeEffects *effects, float alpha);
Color R2D_TimeEffectsFlashColor(const R2D_TimeEffects *effects);
float R2D_TimeEffectsFadeAlpha(const R2D_TimeEffects *effects);
R2D_Palette R2D_PaletteCreate(const Color *colors, int count);
R2D_Palette R2D_PaletteFromHex(const unsigned int *rgba, int count);
Color R2D_PaletteColor(const R2D_Palette *palette, int index, Color fallback);
int R2D_PaletteNearestIndex(const R2D_Palette *palette, Color color);
Color R2D_PaletteNearestColor(const R2D_Palette *palette, Color color, Color fallback);
Color R2D_PaletteMixColor(Color color, Color target, float amount);
Color R2D_PaletteFadeColor(Color color, Color target, float amount);
Image R2D_ImageRecolorPalette(Image image, const R2D_Palette *from, const R2D_Palette *to, float amount);
Texture2D R2D_LoadTextureFromPalette(Image image, const R2D_Palette *from, const R2D_Palette *to, float amount);
const char *R2D_UserDataPath(const char *app_name, const char *file_name);
R2D_SaveData R2D_SaveDataDefault(void);
bool R2D_SaveDataLoad(const char *path, R2D_SaveData *save);
bool R2D_SaveDataSave(const char *path, R2D_SaveData save);
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
int R2D_TilemapCollisionRects(const R2D_Tilemap *tilemap, int layer_index, Rectangle area, R2D_Collider *colliders, int max_colliders, unsigned int layer, unsigned int mask);
Vector2 R2D_TilemapMoveAndSlide(const R2D_Tilemap *tilemap, int layer_index, Rectangle bounds, Vector2 movement, R2D_CollisionResult *result);
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
