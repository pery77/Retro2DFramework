#ifndef R2D_H
#define R2D_H

#include "raylib.h"
#include <stddef.h>

#define R2D_DEFAULT_VIRTUAL_WIDTH 320
#define R2D_DEFAULT_VIRTUAL_HEIGHT 200
#define R2D_DEFAULT_WINDOW_SCALE 4
#define R2D_AUDIO_SAMPLE_RATE 48000
#define R2D_AUDIO_MAX_VOICES 16
#define R2D_INPUT_MAX_ACTIONS 32
#define R2D_INPUT_MAX_BINDINGS 8
#define R2D_INPUT_ACTION_NAME_SIZE 32
#define R2D_ANIM_CLIP_NAME_SIZE 32
#define R2D_ANIM_SET_MAX_CLIPS 32
#define R2D_SPRITE_ATLAS_MAX_FRAMES 128
#define R2D_SPRITE_FRAME_NAME_SIZE 32
#define R2D_STATE_STACK_MAX 8
#define R2D_COLLISION_MAX_HITS 8
#define R2D_ENTITY_MAX 256
#define R2D_PARTICLE_MAX 512
#define R2D_TIMER_MAX 64
#define R2D_TWEEN_MAX 64
#define R2D_CINEMATIC_MAX_STEPS 32
#define R2D_CINEMATIC_TEXT_SIZE 160
#define R2D_PALETTE_MAX_COLORS 32
#define R2D_ASSET_CACHE_MAX_TEXTURES 128
#define R2D_ASSET_CACHE_MAX_SHADERS 32
#define R2D_ASSET_CACHE_PATH_SIZE 256
#define R2D_HOT_RELOAD_PATH_SIZE 512
#define R2D_RUNTIME_PATH_SIZE 512
#define R2D_LOG_MESSAGE_SIZE 512
#define R2D_LOG_LAST_ERROR_SIZE 256
#define R2D_LOCALE_MAX_ENTRIES 128
#define R2D_LOCALE_KEY_SIZE 64
#define R2D_LOCALE_TEXT_SIZE 256
#define R2D_LOCALE_LANGUAGE_SIZE 16
#define R2D_SAVE_PATH_SIZE 512
#define R2D_TILEMAP_MAX_PROPERTIES 16
#define R2D_TILEMAP_PROPERTY_NAME_SIZE 64
#define R2D_TILEMAP_PROPERTY_STRING_SIZE 128
#define R2D_GRID_MAX_SEARCH_NODES 2048

#ifdef __cplusplus
extern "C" {
#endif

typedef struct R2D_Config {
    int virtual_width;
    int virtual_height;
    int window_scale;
    const char *title;
    Color clear_color;
    bool fullscreen;
    const char *asset_pack_path;
} R2D_Config;

typedef struct R2D_RuntimeConfig {
    R2D_Config config;
    char asset_pack_path[R2D_RUNTIME_PATH_SIZE];
    bool crt_enabled;
    float master_volume;
    float music_volume;
    float sfx_volume;
    float ui_volume;
    float ambient_volume;
} R2D_RuntimeConfig;

typedef enum R2D_LogLevel {
    R2D_LOG_LEVEL_DEBUG = 0,
    R2D_LOG_LEVEL_INFO,
    R2D_LOG_LEVEL_WARN,
    R2D_LOG_LEVEL_ERROR,
    R2D_LOG_LEVEL_NONE
} R2D_LogLevel;

typedef enum R2D_LogSubsystem {
    R2D_LOG_SUBSYSTEM_CORE = 0,
    R2D_LOG_SUBSYSTEM_ASSETS,
    R2D_LOG_SUBSYSTEM_AUDIO,
    R2D_LOG_SUBSYSTEM_INPUT,
    R2D_LOG_SUBSYSTEM_RENDER,
    R2D_LOG_SUBSYSTEM_SAVE,
    R2D_LOG_SUBSYSTEM_TILEMAP,
    R2D_LOG_SUBSYSTEM_UI,
    R2D_LOG_SUBSYSTEM_GAME,
    R2D_LOG_SUBSYSTEM_SCRIPT,
    R2D_LOG_SUBSYSTEM_COUNT
} R2D_LogSubsystem;

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

typedef enum R2D_RadianceDebugView {
    R2D_RADIANCE_DEBUG_NONE = 0,
    R2D_RADIANCE_DEBUG_MASK,
    R2D_RADIANCE_DEBUG_CASCADE
} R2D_RadianceDebugView;

typedef struct R2D_Radiance {
    RenderTexture2D mask;
    RenderTexture2D cascade_a;
    RenderTexture2D cascade_b;
    RenderTexture2D light;
    RenderTexture2D body_glow;
    RenderTexture2D color;
    Shader cascade_shader;
    Shader resolve_shader;
    Shader body_glow_shader;
    Shader compose_shader;
    int cascade_scene_loc;
    int cascade_prev_loc;
    int cascade_resolution_loc;
    int cascade_base_spacing_loc;
    int cascade_base_rays_loc;
    int cascade_probe_count_loc;
    int cascade_index_loc;
    int cascade_count_loc;
    int cascade_sky_enabled_loc;
    int cascade_sky_color_loc;
    int cascade_falloff_loc;
    int cascade_light_range_loc;
    int resolve_cascade_loc;
    int resolve_resolution_loc;
    int resolve_viewport_resolution_loc;
    int resolve_mask_offset_loc;
    int resolve_base_spacing_loc;
    int resolve_base_rays_loc;
    int resolve_probe_count_loc;
    int resolve_intensity_loc;
    int body_glow_texture_loc;
    int body_glow_resolution_loc;
    int body_glow_radius_loc;
    int compose_scene_loc;
    int compose_light_loc;
    int compose_body_glow_loc;
    int compose_mask_loc;
    int compose_resolution_loc;
    int compose_ambient_loc;
    int compose_edge_force_loc;
    int compose_body_force_loc;
    int compose_body_glow_strength_loc;
    int compose_viewport_resolution_loc;
    int compose_mask_offset_loc;
    int width;
    int height;
    int mask_width;
    int mask_height;
    int viewport_padding;
    int cascade_width;
    int cascade_height;
    int base_spacing;
    int base_rays;
    int cascade_count;
    float intensity;
    float ambient;
    float falloff;
    float light_range;
    float edge_force;
    float body_force;
    float body_glow_strength;
    float body_glow_radius;
    Color sky_color;
    bool sky_enabled;
    bool enabled;
    bool is_ready;
    bool mask_ready;
    R2D_RadianceDebugView debug_view;
} R2D_Radiance;

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

typedef enum R2D_AudioGroup {
    R2D_AUDIO_GROUP_MUSIC = 0,
    R2D_AUDIO_GROUP_SFX,
    R2D_AUDIO_GROUP_UI,
    R2D_AUDIO_GROUP_AMBIENT,
    R2D_AUDIO_GROUP_COUNT
} R2D_AudioGroup;

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

typedef struct R2D_Script {
    void *state;
    bool loaded;
} R2D_Script;

typedef int (*R2D_ScriptFunction)(void *state);

typedef struct R2D_MusicCrossfade {
    R2D_Music *from;
    R2D_Music *to;
    float from_start_volume;
    float to_target_volume;
    float duration;
    float elapsed;
    bool active;
} R2D_MusicCrossfade;

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

typedef struct R2D_SpriteAtlasFrame {
    char name[R2D_SPRITE_FRAME_NAME_SIZE];
    Rectangle source;
    Vector2 pivot;
    Rectangle hitbox;
    Rectangle hurtbox;
    bool has_hitbox;
    bool has_hurtbox;
} R2D_SpriteAtlasFrame;

typedef struct R2D_SpriteAtlas {
    Texture2D texture;
    R2D_SpriteAtlasFrame frames[R2D_SPRITE_ATLAS_MAX_FRAMES];
    int frame_count;
} R2D_SpriteAtlas;

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

typedef struct R2D_AnimClip {
    char name[R2D_ANIM_CLIP_NAME_SIZE];
    R2D_Anim anim;
} R2D_AnimClip;

typedef struct R2D_AnimSet {
    R2D_AnimClip clips[R2D_ANIM_SET_MAX_CLIPS];
    int count;
} R2D_AnimSet;

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

typedef struct R2D_LocalizedText {
    char key[R2D_LOCALE_KEY_SIZE];
    char text[R2D_LOCALE_TEXT_SIZE];
} R2D_LocalizedText;

typedef struct R2D_Localization {
    char language[R2D_LOCALE_LANGUAGE_SIZE];
    R2D_LocalizedText entries[R2D_LOCALE_MAX_ENTRIES];
    int count;
} R2D_Localization;

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

typedef enum R2D_CinematicStepType {
    R2D_CINEMATIC_STEP_WAIT = 0,
    R2D_CINEMATIC_STEP_DIALOG,
    R2D_CINEMATIC_STEP_MOVE_CAMERA,
    R2D_CINEMATIC_STEP_SET_FLAG,
    R2D_CINEMATIC_STEP_PLAY_SFX,
    R2D_CINEMATIC_STEP_PLAY_MUSIC
} R2D_CinematicStepType;

typedef struct R2D_CinematicStep {
    R2D_CinematicStepType type;
    float duration;
    Vector2 target;
    char text[R2D_CINEMATIC_TEXT_SIZE];
    unsigned int *flags;
    unsigned int flag_mask;
    bool flag_value;
    R2D_Sfx sfx;
    R2D_AudioGroup audio_group;
    R2D_Music *music;
    float music_volume;
    bool music_loop;
} R2D_CinematicStep;

typedef struct R2D_Cinematic {
    R2D_CinematicStep steps[R2D_CINEMATIC_MAX_STEPS];
    int step_count;
    int current_step;
    float elapsed;
    Vector2 camera_start;
    Vector2 camera_position;
    bool step_started;
    bool active;
    bool input_locked;
    bool dialog_visible;
    char dialog_text[R2D_CINEMATIC_TEXT_SIZE];
} R2D_Cinematic;

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

typedef struct R2D_FileWatch {
    char path[R2D_HOT_RELOAD_PATH_SIZE];
    long long last_write_time;
    bool exists;
    bool initialized;
} R2D_FileWatch;

typedef enum R2D_TilemapPropertyType {
    R2D_TILEMAP_PROPERTY_STRING = 0,
    R2D_TILEMAP_PROPERTY_INT,
    R2D_TILEMAP_PROPERTY_FLOAT,
    R2D_TILEMAP_PROPERTY_BOOL,
    R2D_TILEMAP_PROPERTY_COLOR
} R2D_TilemapPropertyType;

typedef struct R2D_TilemapProperty {
    char name[R2D_TILEMAP_PROPERTY_NAME_SIZE];
    R2D_TilemapPropertyType type;
    char string_value[R2D_TILEMAP_PROPERTY_STRING_SIZE];
    int int_value;
    float float_value;
    bool bool_value;
    Color color_value;
} R2D_TilemapProperty;

typedef struct R2D_TilemapLayer {
    char name[64];
    unsigned int *tiles;
    int width;
    int height;
    float opacity;
    float offset_x;
    float offset_y;
    float parallax_x;
    float parallax_y;
    bool visible;
    R2D_TilemapProperty properties[R2D_TILEMAP_MAX_PROPERTIES];
    int property_count;
} R2D_TilemapLayer;

typedef struct R2D_TilemapObject {
    char name[64];
    char type[64];
    Rectangle rect;
    R2D_TilemapProperty properties[R2D_TILEMAP_MAX_PROPERTIES];
    int property_count;
} R2D_TilemapObject;

typedef struct R2D_TilemapAnimationFrame {
    int tile_id;
    int duration_ms;
} R2D_TilemapAnimationFrame;

typedef struct R2D_TilemapTileAnimation {
    int tile_id;
    R2D_TilemapAnimationFrame *frames;
    int frame_count;
    int duration_ms;
} R2D_TilemapTileAnimation;

typedef struct R2D_TilemapTileset {
    Texture2D texture;
    R2D_TilemapTileAnimation *animations;
    int first_gid;
    int tile_width;
    int tile_height;
    int margin;
    int spacing;
    int columns;
    int tile_count;
    int animation_count;
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

typedef struct R2D_GridPoint {
    int x;
    int y;
} R2D_GridPoint;

typedef bool (*R2D_GridBlockedCallback)(int x, int y, void *user_data);

typedef struct R2D_DebugInfo {
    const char *title;
    const char *line;
    int fps;
    float frame_ms;
    int entity_count;
    int asset_count;
    int tile_x;
    int tile_y;
    unsigned int tile_gid;
    size_t memory_bytes;
} R2D_DebugInfo;

typedef struct R2D_Context {
    R2D_Config config;
    RenderTexture2D target;
    RenderTexture2D overlay;
    RenderTexture2D composite;
    Rectangle source;
    Rectangle destination;
    Vector2 origin;
    int windowed_width;
    int windowed_height;
    Vector2 windowed_position;
    R2D_Crt *crt;
    R2D_Radiance *radiance;
    bool screenshot_requested;
    bool close_requested;
    bool is_ready;
} R2D_Context;

R2D_Config R2D_DefaultConfig(void);
void R2D_LogSetLevel(R2D_LogLevel level);
R2D_LogLevel R2D_LogLevelCurrent(void);
bool R2D_LogOpenFile(const char *path);
void R2D_LogCloseFile(void);
void R2D_LogMessage(R2D_LogLevel level, R2D_LogSubsystem subsystem, const char *format, ...);
void R2D_LogInfo(R2D_LogSubsystem subsystem, const char *format, ...);
void R2D_LogWarn(R2D_LogSubsystem subsystem, const char *format, ...);
void R2D_LogError(R2D_LogSubsystem subsystem, const char *format, ...);
const char *R2D_LogLastError(R2D_LogSubsystem subsystem);
const char *R2D_LogLevelName(R2D_LogLevel level);
const char *R2D_LogSubsystemName(R2D_LogSubsystem subsystem);
R2D_RuntimeConfig R2D_RuntimeConfigDefault(void);
bool R2D_RuntimeConfigLoad(R2D_RuntimeConfig *runtime, const char *path);
void R2D_RuntimeConfigApplyArgs(R2D_RuntimeConfig *runtime, int argc, char **argv);
void R2D_RuntimeConfigApplyAudio(const R2D_RuntimeConfig *runtime);
void R2D_RuntimeConfigApplyCrt(const R2D_RuntimeConfig *runtime, R2D_Crt *crt);
bool R2D_ScriptAvailable(void);
bool R2D_ScriptInit(R2D_Script *script);
void R2D_ScriptClose(R2D_Script *script);
bool R2D_ScriptIsReady(const R2D_Script *script);
void *R2D_ScriptState(R2D_Script *script);
bool R2D_ScriptLoadFile(R2D_Script *script, const char *path);
bool R2D_ScriptDoString(R2D_Script *script, const char *source);
bool R2D_ScriptCall(R2D_Script *script, const char *function_name, int argument_count, int result_count);
bool R2D_ScriptRegister(R2D_Script *script, const char *name, R2D_ScriptFunction function);
void R2D_ScriptPop(R2D_Script *script, int count);
int R2D_ScriptStackTop(R2D_Script *script);
void R2D_ScriptPushNumber(R2D_Script *script, double value);
void R2D_ScriptPushBoolean(R2D_Script *script, bool value);
void R2D_ScriptPushString(R2D_Script *script, const char *value);
double R2D_ScriptToNumber(R2D_Script *script, int index, double fallback);
bool R2D_ScriptToBoolean(R2D_Script *script, int index, bool fallback);
const char *R2D_ScriptToString(R2D_Script *script, int index, const char *fallback);

bool R2D_Init(R2D_Context *ctx, R2D_Config config);
void R2D_Run(R2D_Context *ctx, R2D_App app);
void R2D_RequestClose(R2D_Context *ctx);
void R2D_Close(R2D_Context *ctx);

void R2D_BeginFrame(R2D_Context *ctx);
void R2D_EndFrame(R2D_Context *ctx);
void R2D_BeginOverlay(R2D_Context *ctx);
void R2D_EndOverlay(R2D_Context *ctx);
void R2D_ClearOverlay(R2D_Context *ctx);

void R2D_ToggleFullscreen(R2D_Context *ctx);
void R2D_TakeScreenshot(void);

bool R2D_AudioInit(void);
void R2D_AudioClose(void);
bool R2D_AudioIsReady(void);
void R2D_AudioSetMasterVolume(float volume);
float R2D_AudioMasterVolume(void);
void R2D_AudioSetGroupVolume(R2D_AudioGroup group, float volume);
float R2D_AudioGroupVolume(R2D_AudioGroup group);
void R2D_AudioFadeGroup(R2D_AudioGroup group, float target_volume, float duration);
void R2D_AudioMixerUpdate(float dt);
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
void R2D_PlaySfxGroup(R2D_Sfx sfx, R2D_AudioGroup group);
void R2D_PlaySfxRandomPitch(R2D_Sfx sfx, R2D_AudioGroup group, float semitone_range);
void R2D_PlayTone(R2D_Waveform waveform, float frequency, float duration);
void R2D_PlayToneGroup(R2D_Waveform waveform, float frequency, float duration, R2D_AudioGroup group);

bool R2D_MusicLoad(R2D_Music *music, const char *midi_path, const char *soundfont_path);
bool R2D_MusicLoadSong(R2D_Music *music, const char *song_path);
void R2D_MusicUnload(R2D_Music *music);
void R2D_MusicPlay(R2D_Music *music, bool loop);
void R2D_MusicStop(R2D_Music *music);
void R2D_MusicPause(R2D_Music *music);
void R2D_MusicResume(R2D_Music *music);
void R2D_MusicUpdate(R2D_Music *music);
void R2D_MusicSetVolume(R2D_Music *music, float volume);
float R2D_MusicVolume(const R2D_Music *music);
void R2D_MusicSetGroup(R2D_Music *music, R2D_AudioGroup group);
void R2D_MusicCrossfadeStart(R2D_MusicCrossfade *crossfade, R2D_Music *from, R2D_Music *to, float target_volume, float duration);
bool R2D_MusicCrossfadeUpdate(R2D_MusicCrossfade *crossfade, float dt);
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

bool R2D_RadianceInit(R2D_Radiance *radiance, int width, int height);
void R2D_RadianceClose(R2D_Radiance *radiance);
bool R2D_RadianceReload(R2D_Radiance *radiance);
void R2D_RadianceSetEnabled(R2D_Radiance *radiance, bool enabled);
void R2D_RadianceSetDebugView(R2D_Radiance *radiance, R2D_RadianceDebugView debug_view);
void R2D_RadianceSetLight(R2D_Radiance *radiance, float intensity, float ambient);
void R2D_RadianceSetOccluderLight(R2D_Radiance *radiance, float edge_force, float body_force);
void R2D_RadianceSetOccluderBodyGlow(R2D_Radiance *radiance, float strength, float radius);
void R2D_RadianceSetFalloff(R2D_Radiance *radiance, float falloff);
void R2D_RadianceSetLightRange(R2D_Radiance *radiance, float light_range);
void R2D_RadianceSetSky(R2D_Radiance *radiance, bool enabled, Color color);
bool R2D_RadianceSetViewportPadding(R2D_Radiance *radiance, int padding);
bool R2D_RadianceSetQuality(R2D_Radiance *radiance, int base_spacing, int base_rays, int cascade_count);
void R2D_RadianceBeginMask(R2D_Context *ctx, R2D_Radiance *radiance);
void R2D_RadianceEndMask(R2D_Context *ctx, R2D_Radiance *radiance);
void R2D_RadianceDrawOccluderRect(Rectangle rect);
void R2D_RadianceDrawOccluderCircle(Vector2 center, float radius);
void R2D_RadianceDrawEmitterRect(Rectangle rect, Color color);
void R2D_RadianceDrawEmitterCircle(Vector2 center, float radius, Color color);
Texture2D R2D_RadianceRender(R2D_Radiance *radiance, Texture2D color_texture);
void R2D_SetRadiance(R2D_Context *ctx, R2D_Radiance *radiance);

const char *R2D_AssetPath(const char *relative_path);
void R2D_SetDevelopmentAssetDir(const char *path);
const char *R2D_DevelopmentAssetDir(void);
bool R2D_MountAssetPack(const char *path);
void R2D_UnmountAssetPack(void);
bool R2D_AssetPackMounted(void);
int R2D_AssetPackEntryCount(void);
bool R2D_AssetExists(const char *path);
bool R2D_LoadAssetData(const char *path, unsigned char **data, int *size);
void R2D_UnloadAssetData(unsigned char *data);
char *R2D_LoadAssetText(const char *path);
void R2D_UnloadAssetText(char *text);
Image R2D_LoadImage(const char *path);
Texture2D R2D_LoadTexture(const char *path);
Shader R2D_LoadFragmentShader(const char *path);
void R2D_AssetCacheInit(R2D_AssetCache *cache);
void R2D_AssetCacheClear(R2D_AssetCache *cache);
void R2D_AssetCacheReleaseGroup(R2D_AssetCache *cache, int group);
Texture2D R2D_AssetCacheLoadTexture(R2D_AssetCache *cache, const char *path, int group);
Shader R2D_AssetCacheLoadFragmentShader(R2D_AssetCache *cache, const char *path, int group);
int R2D_AssetCacheTextureCount(const R2D_AssetCache *cache);
int R2D_AssetCacheShaderCount(const R2D_AssetCache *cache);
void R2D_FileWatchInit(R2D_FileWatch *watch);
bool R2D_FileWatchSet(R2D_FileWatch *watch, const char *path);
bool R2D_FileWatchCheck(R2D_FileWatch *watch);
bool R2D_FileWatchExists(const R2D_FileWatch *watch);
const char *R2D_FileWatchPath(const R2D_FileWatch *watch);

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
Vector2 R2D_CameraWorldToPixelScreen(const R2D_Camera *camera, Vector2 world);
Vector2 R2D_CameraScreenToWorld(const R2D_Camera *camera, Vector2 screen);
Rectangle R2D_CameraRectToPixelScreen(const R2D_Camera *camera, Rectangle rect);
Rectangle R2D_CameraView(const R2D_Camera *camera);
void R2D_DrawRectangleCamera(const R2D_Camera *camera, Rectangle rect, Color color);
void R2D_DrawRectangleLinesCamera(const R2D_Camera *camera, Rectangle rect, float line_thick, Color color);
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
R2D_DebugInfo R2D_DebugInfoDefault(void);
const char *R2D_DebugFormatBytes(size_t bytes, char *buffer, int buffer_size);
void R2D_DebugDrawOverlay(const R2D_DebugInfo *info, int x, int y);
void R2D_TypewriterStart(R2D_Typewriter *typewriter, const char *text, float chars_per_second);
void R2D_TypewriterReset(R2D_Typewriter *typewriter);
void R2D_TypewriterUpdate(R2D_Typewriter *typewriter, float dt);
void R2D_TypewriterComplete(R2D_Typewriter *typewriter);
bool R2D_TypewriterDone(const R2D_Typewriter *typewriter);
void R2D_DrawTypewriter(R2D_Typewriter typewriter, Rectangle bounds, R2D_TextStyle style);
void R2D_LocalizationInit(R2D_Localization *localization);
bool R2D_LocalizationLoad(R2D_Localization *localization, const char *language);
bool R2D_LocalizationLoadFile(R2D_Localization *localization, const char *path, const char *language);
bool R2D_LocalizationSet(R2D_Localization *localization, const char *key, const char *text);
bool R2D_LocalizationHas(const R2D_Localization *localization, const char *key);
const char *R2D_LocalizationGet(const R2D_Localization *localization, const char *key, const char *fallback);
void R2D_LocalizationClear(R2D_Localization *localization);
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
float R2D_Clamp(float value, float min, float max);
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
void R2D_CinematicInit(R2D_Cinematic *cinematic);
bool R2D_CinematicAddWait(R2D_Cinematic *cinematic, float duration);
bool R2D_CinematicAddDialog(R2D_Cinematic *cinematic, const char *text, float duration);
bool R2D_CinematicAddMoveCamera(R2D_Cinematic *cinematic, Vector2 target, float duration);
bool R2D_CinematicAddSetFlag(R2D_Cinematic *cinematic, unsigned int *flags, unsigned int flag_mask, bool value);
bool R2D_CinematicAddSfx(R2D_Cinematic *cinematic, R2D_Sfx sfx, R2D_AudioGroup group);
bool R2D_CinematicAddMusic(R2D_Cinematic *cinematic, R2D_Music *music, float volume, bool loop);
void R2D_CinematicStart(R2D_Cinematic *cinematic, Vector2 camera_position);
void R2D_CinematicStop(R2D_Cinematic *cinematic);
bool R2D_CinematicUpdate(R2D_Cinematic *cinematic, float dt, Vector2 camera_position);
bool R2D_CinematicActive(const R2D_Cinematic *cinematic);
bool R2D_CinematicInputLocked(const R2D_Cinematic *cinematic);
Vector2 R2D_CinematicCameraPosition(const R2D_Cinematic *cinematic, Vector2 fallback);
void R2D_CinematicDrawDialog(const R2D_Cinematic *cinematic, Rectangle rect, R2D_UiStyle style);
R2D_Palette R2D_PaletteCreate(const Color *colors, int count);
R2D_Palette R2D_PaletteFromHex(const unsigned int *rgba, int count);
Color R2D_PaletteColor(const R2D_Palette *palette, int index, Color fallback);
int R2D_PaletteNearestIndex(const R2D_Palette *palette, Color color);
Color R2D_PaletteNearestColor(const R2D_Palette *palette, Color color, Color fallback);
Color R2D_PaletteMixColor(Color color, Color target, float amount);
Color R2D_PaletteFadeColor(Color color, Color target, float amount);
Image R2D_ImageRecolorPalette(Image image, const R2D_Palette *from, const R2D_Palette *to, float amount);
Texture2D R2D_LoadTextureFromPalette(Image image, const R2D_Palette *from, const R2D_Palette *to, float amount);
R2D_GridPoint R2D_GridPointMake(int x, int y);
int R2D_GridManhattanDistance(R2D_GridPoint a, R2D_GridPoint b);
float R2D_GridEuclideanDistance(R2D_GridPoint a, R2D_GridPoint b);
bool R2D_GridLineOfSight(R2D_GridPoint start, R2D_GridPoint end, R2D_GridBlockedCallback blocked, void *user_data);
int R2D_GridFloodFill(R2D_GridPoint start, int width, int height, R2D_GridBlockedCallback blocked, void *user_data, R2D_GridPoint *out_points, int max_points);
int R2D_GridAStar(R2D_GridPoint start, R2D_GridPoint goal, int width, int height, R2D_GridBlockedCallback blocked, void *user_data, R2D_GridPoint *out_path, int max_path);
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
bool R2D_LoadSpriteAtlas(R2D_SpriteAtlas *atlas, const char *path);
void R2D_UnloadSpriteAtlas(R2D_SpriteAtlas *atlas);
const R2D_SpriteAtlasFrame *R2D_SpriteAtlasFind(const R2D_SpriteAtlas *atlas, const char *name);
Rectangle R2D_SpriteAtlasHitbox(const R2D_SpriteAtlasFrame *frame, Vector2 position);
Rectangle R2D_SpriteAtlasHurtbox(const R2D_SpriteAtlasFrame *frame, Vector2 position);
R2D_Anim R2D_AnimFrames(int first_frame, int frame_count, float fps, bool loop);
void R2D_AnimSetInit(R2D_AnimSet *set);
bool R2D_AnimSetAdd(R2D_AnimSet *set, const char *name, R2D_Anim anim);
const R2D_Anim *R2D_AnimSetFind(const R2D_AnimSet *set, const char *name);
R2D_Anim R2D_AnimSetGet(const R2D_AnimSet *set, const char *name, R2D_Anim fallback);
bool R2D_AnimSetLoad(R2D_AnimSet *set, const char *path);
void R2D_AnimPlay(R2D_AnimPlayer *player, R2D_Anim anim);
bool R2D_AnimPlayNamed(R2D_AnimPlayer *player, const R2D_AnimSet *set, const char *name, R2D_Anim fallback);
void R2D_AnimStop(R2D_AnimPlayer *player);
void R2D_AnimUpdate(R2D_AnimPlayer *player, float dt);
int R2D_AnimFrame(const R2D_AnimPlayer *player);
void R2D_DrawSprite(Texture2D texture, Rectangle source, Vector2 position, bool flip_x);
void R2D_DrawSpriteEx(Texture2D texture, Rectangle source, Vector2 position, Vector2 origin, float rotation, float scale, bool flip_x, Color tint);
void R2D_DrawSpriteCamera(const R2D_Camera *camera, Texture2D texture, Rectangle source, Vector2 position, bool flip_x);
void R2D_DrawSpriteExCamera(const R2D_Camera *camera, Texture2D texture, Rectangle source, Vector2 position, Vector2 origin, float rotation, float scale, bool flip_x, Color tint);
void R2D_DrawAtlasFrame(const R2D_SpriteAtlas *atlas, const char *name, Vector2 position, bool flip_x);
void R2D_DrawAtlasFrameEx(const R2D_SpriteAtlas *atlas, const R2D_SpriteAtlasFrame *frame, Vector2 position, float rotation, float scale, bool flip_x, Color tint);
void R2D_DrawAtlasFrameExCamera(const R2D_Camera *camera, const R2D_SpriteAtlas *atlas, const R2D_SpriteAtlasFrame *frame, Vector2 position, float rotation, float scale, bool flip_x, Color tint);
void R2D_DrawSheetFrame(const R2D_SpriteSheet *sheet, int frame, Vector2 position, bool flip_x);
void R2D_DrawSheetFrameCamera(const R2D_Camera *camera, const R2D_SpriteSheet *sheet, int frame, Vector2 position, bool flip_x);
void R2D_DrawAnim(const R2D_SpriteSheet *sheet, const R2D_AnimPlayer *player, Vector2 position, bool flip_x);
void R2D_DrawAnimCamera(const R2D_Camera *camera, const R2D_SpriteSheet *sheet, const R2D_AnimPlayer *player, Vector2 position, bool flip_x);
bool R2D_TilemapLoadTiledJson(R2D_Tilemap *tilemap, const char *path);
void R2D_TilemapUnload(R2D_Tilemap *tilemap);
bool R2D_TilemapIsReady(const R2D_Tilemap *tilemap);
int R2D_TilemapLayerIndex(const R2D_Tilemap *tilemap, const char *name);
unsigned int R2D_TilemapTileAt(const R2D_Tilemap *tilemap, int layer_index, int x, int y);
int R2D_TilemapLayerPropertyCount(const R2D_Tilemap *tilemap, int layer_index);
const R2D_TilemapProperty *R2D_TilemapLayerPropertyAt(const R2D_Tilemap *tilemap, int layer_index, int property_index);
const R2D_TilemapProperty *R2D_TilemapLayerFindProperty(const R2D_Tilemap *tilemap, int layer_index, const char *name);
const R2D_TilemapProperty *R2D_TilemapObjectFindProperty(const R2D_TilemapObject *object, const char *name);
const char *R2D_TilemapPropertyString(const R2D_TilemapProperty *property, const char *fallback);
int R2D_TilemapPropertyInt(const R2D_TilemapProperty *property, int fallback);
float R2D_TilemapPropertyFloat(const R2D_TilemapProperty *property, float fallback);
bool R2D_TilemapPropertyBool(const R2D_TilemapProperty *property, bool fallback);
Color R2D_TilemapPropertyColor(const R2D_TilemapProperty *property, Color fallback);
Vector2 R2D_TilemapWorldToTile(const R2D_Tilemap *tilemap, Vector2 position);
Rectangle R2D_TilemapTileBounds(const R2D_Tilemap *tilemap, int x, int y);
bool R2D_TilemapSolidAt(const R2D_Tilemap *tilemap, int layer_index, Vector2 position);
bool R2D_TilemapRectCollides(const R2D_Tilemap *tilemap, int layer_index, Rectangle rect);
bool R2D_TilemapGridBlocked(const R2D_Tilemap *tilemap, int layer_index, int x, int y);
int R2D_TilemapFindPath(const R2D_Tilemap *tilemap, int layer_index, R2D_GridPoint start, R2D_GridPoint goal, R2D_GridPoint *out_path, int max_path);
int R2D_TilemapFloodFill(const R2D_Tilemap *tilemap, int layer_index, R2D_GridPoint start, R2D_GridPoint *out_points, int max_points);
bool R2D_TilemapLineOfSight(const R2D_Tilemap *tilemap, int layer_index, R2D_GridPoint start, R2D_GridPoint end);
int R2D_TilemapCollisionRects(const R2D_Tilemap *tilemap, int layer_index, Rectangle area, R2D_Collider *colliders, int max_colliders, unsigned int layer, unsigned int mask);
Vector2 R2D_TilemapMoveAndSlide(const R2D_Tilemap *tilemap, int layer_index, Rectangle bounds, Vector2 movement, R2D_CollisionResult *result);
int R2D_TilemapObjectCount(const R2D_Tilemap *tilemap);
const R2D_TilemapObject *R2D_TilemapObjectAt(const R2D_Tilemap *tilemap, int index);
const R2D_TilemapObject *R2D_TilemapFindObject(const R2D_Tilemap *tilemap, const char *name);
const R2D_TilemapObject *R2D_TilemapFindObjectByType(const R2D_Tilemap *tilemap, const char *type);
bool R2D_TilemapObjectIsTrigger(const R2D_TilemapObject *object);
int R2D_TilemapTriggerColliders(const R2D_Tilemap *tilemap, R2D_Collider *colliders, int max_colliders, unsigned int layer, unsigned int mask);
void R2D_TilemapDraw(const R2D_Tilemap *tilemap, Vector2 position);
void R2D_TilemapDrawLayer(const R2D_Tilemap *tilemap, int layer_index, Vector2 position);
void R2D_TilemapDrawVisible(const R2D_Tilemap *tilemap, Rectangle view, Vector2 position);
void R2D_TilemapDrawLayerVisible(const R2D_Tilemap *tilemap, int layer_index, Rectangle view, Vector2 position);
void R2D_TilemapDrawLayerParallax(const R2D_Tilemap *tilemap, int layer_index, Rectangle camera_view, Vector2 screen_position);
void R2D_TilemapDrawCollisionDebug(const R2D_Tilemap *tilemap, int layer_index, Vector2 position, Color color);
void R2D_TilemapDrawCollisionDebugVisible(const R2D_Tilemap *tilemap, int layer_index, Rectangle view, Vector2 position, Color color);
void R2D_TilemapDrawObjectsDebug(const R2D_Tilemap *tilemap, Vector2 position, Color color);
Color R2D_ColorFromHex(unsigned int rgba);

#ifdef __cplusplus
}
#endif

#endif
