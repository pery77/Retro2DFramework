#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define COLLECT_MAX_COINS 32
#define COLLECT_PLAYER_BOUNDS_OFFSET_X 3.0f
#define COLLECT_PLAYER_BOUNDS_OFFSET_Y 2.0f
#define COLLECT_FLAG_FOUNTAIN_EVENT 0x00000001u

typedef enum CollectEntityType {
    COLLECT_ENTITY_COIN = 1
} CollectEntityType;

typedef enum PlayerDirection {
    PLAYER_SOUTH = 0,
    PLAYER_NORTH,
    PLAYER_EAST,
    PLAYER_WEST
} PlayerDirection;

typedef struct CollectDemo {
    Vector2 player;
    R2D_Camera camera;
    R2D_EntityWorld entities;
    R2D_Tilemap tilemap;
    R2D_SpriteSheet player_sheet;
    R2D_SpriteSheet coin_sheet;
    R2D_SpriteAtlas player_atlas;
    R2D_AnimSet player_anims;
    R2D_AnimSet coin_anims;
    R2D_AnimPlayer player_anim;
    R2D_AnimPlayer coin_anim;
    R2D_InputMap input;
    R2D_Sfx coin_sfx;
    R2D_Sfx event_sfx;
    R2D_Music music;
    R2D_Cinematic cinematic;
    R2D_Context *context;
    unsigned int event_flags;
    int collision_layer;
    int pickup_layer;
    int coin_count;
    int coins_collected;
    Color collision_debug_color;
    PlayerDirection player_direction;
    char trigger_text[64];
    char player_anim_name[R2D_ANIM_CLIP_NAME_SIZE];
    float trigger_timer;
    bool debug_draw;
    bool music_loaded;
} CollectDemo;

static R2D_Sfx Collect_LoadCoinSfx(void)
{
    R2D_Sfx sfx = R2D_SfxCoin();

    R2D_LoadSfx(R2D_AssetPath("audio/sfx/coin.r2sfx"), &sfx);
    return sfx;
}

static int Collect_PlayerDirectionRow(PlayerDirection direction)
{
    switch (direction) {
    case PLAYER_SOUTH:
        return 0;
    case PLAYER_WEST:
        return 1;
    case PLAYER_EAST:
        return 2;
    case PLAYER_NORTH:
        return 3;
    default:
        return 0;
    }
}

static const char *Collect_PlayerDirectionName(PlayerDirection direction)
{
    switch (direction) {
    case PLAYER_SOUTH:
        return "south";
    case PLAYER_WEST:
        return "west";
    case PLAYER_EAST:
        return "east";
    case PLAYER_NORTH:
        return "north";
    default:
        return "south";
    }
}

static bool Collect_ObjectIsCoin(const R2D_TilemapObject *object)
{
    return strcmp(object->type, "coin") == 0 ||
        strcmp(object->type, "pickup") == 0 ||
        strcmp(object->name, "Coin") == 0 ||
        strncmp(object->name, "Coin", 4) == 0;
}

static bool Collect_LayerDrawsAbovePlayer(const char *name)
{
    return strcmp(name, "Above") == 0 ||
        strcmp(name, "Foreground") == 0 ||
        strcmp(name, "Over") == 0;
}

static bool Collect_LayerSpawnsCoins(const R2D_Tilemap *tilemap, int layer_index)
{
    const R2D_TilemapProperty *spawn_property =
        R2D_TilemapLayerFindProperty(tilemap, layer_index, "spawn");
    const char *spawn = R2D_TilemapPropertyString(spawn_property, "");

    return strcmp(spawn, "coin") == 0 ||
        strcmp(spawn, "coins") == 0 ||
        strcmp(tilemap->layers[layer_index].name, "Coins") == 0 ||
        strcmp(tilemap->layers[layer_index].name, "Pickups") == 0;
}

static void Collect_DrawTileLayers(
    const CollectDemo *demo,
    Rectangle camera_view,
    Vector2 screen_position,
    bool above_player
)
{
    if (!R2D_TilemapIsReady(&demo->tilemap)) {
        return;
    }

    for (int i = 0; i < demo->tilemap.layer_count; ++i) {
        if (i == demo->collision_layer || Collect_LayerSpawnsCoins(&demo->tilemap, i)) {
            continue;
        }

        if (Collect_LayerDrawsAbovePlayer(demo->tilemap.layers[i].name) == above_player) {
            R2D_TilemapDrawLayerParallax(&demo->tilemap, i, camera_view, screen_position);
        }
    }
}

static Rectangle Collect_PlayerBounds(Vector2 position)
{
    return R2D_Rect(
        position.x + COLLECT_PLAYER_BOUNDS_OFFSET_X,
        position.y + COLLECT_PLAYER_BOUNDS_OFFSET_Y,
        10.0f,
        13.0f
    );
}

static void Collect_InitInput(CollectDemo *demo)
{
    R2D_InputInit(&demo->input);

    R2D_InputBindKey(&demo->input, "move_left", KEY_LEFT);
    R2D_InputBindKey(&demo->input, "move_left", KEY_A);
    R2D_InputBindGamepadButton(&demo->input, "move_left", GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    R2D_InputBindGamepadAxis(&demo->input, "move_left", GAMEPAD_AXIS_LEFT_X, false);

    R2D_InputBindKey(&demo->input, "move_right", KEY_RIGHT);
    R2D_InputBindKey(&demo->input, "move_right", KEY_D);
    R2D_InputBindGamepadButton(&demo->input, "move_right", GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    R2D_InputBindGamepadAxis(&demo->input, "move_right", GAMEPAD_AXIS_LEFT_X, true);

    R2D_InputBindKey(&demo->input, "move_up", KEY_UP);
    R2D_InputBindKey(&demo->input, "move_up", KEY_W);
    R2D_InputBindGamepadButton(&demo->input, "move_up", GAMEPAD_BUTTON_LEFT_FACE_UP);
    R2D_InputBindGamepadAxis(&demo->input, "move_up", GAMEPAD_AXIS_LEFT_Y, false);

    R2D_InputBindKey(&demo->input, "move_down", KEY_DOWN);
    R2D_InputBindKey(&demo->input, "move_down", KEY_S);
    R2D_InputBindGamepadButton(&demo->input, "move_down", GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    R2D_InputBindGamepadAxis(&demo->input, "move_down", GAMEPAD_AXIS_LEFT_Y, true);

    R2D_InputBindKey(&demo->input, "debug", KEY_F3);
    R2D_InputBindGamepadButton(&demo->input, "debug", GAMEPAD_BUTTON_MIDDLE_RIGHT);
}

static void Collect_SpawnCoin(CollectDemo *demo, Rectangle rect)
{
    R2D_Entity *coin;

    if (demo->coin_count >= COLLECT_MAX_COINS) {
        return;
    }

    coin = R2D_EntitySpawn(&demo->entities, COLLECT_ENTITY_COIN, (Vector2) { rect.x, rect.y });
    if (coin == 0) {
        return;
    }

    coin->bounds = rect;
    coin->layer = 1u;
    demo->coin_count++;
}

static void Collect_PlayPlayerAnim(CollectDemo *demo, const char *name)
{
    R2D_Anim fallback;

    if (name == 0 || strcmp(demo->player_anim_name, name) == 0) {
        return;
    }

    fallback = strcmp(name, "walk") == 0 ?
        R2D_AnimFrames(0, 4, 8.0f, true) :
        R2D_AnimFrames(0, 1, 1.0f, true);

    R2D_AnimPlayNamed(&demo->player_anim, &demo->player_anims, name, fallback);
    snprintf(demo->player_anim_name, sizeof(demo->player_anim_name), "%s", name);
}

static void Collect_LoadCoinLayer(CollectDemo *demo)
{
    if (!R2D_TilemapIsReady(&demo->tilemap)) {
        return;
    }

    for (int layer_index = 0; layer_index < demo->tilemap.layer_count; ++layer_index) {
        const R2D_TilemapLayer *layer = &demo->tilemap.layers[layer_index];

        if (!Collect_LayerSpawnsCoins(&demo->tilemap, layer_index) || layer->tiles == 0) {
            continue;
        }

        demo->pickup_layer = layer_index;
        for (int y = 0; y < layer->height && demo->coin_count < COLLECT_MAX_COINS; ++y) {
            for (int x = 0; x < layer->width && demo->coin_count < COLLECT_MAX_COINS; ++x) {
                if (layer->tiles[y * layer->width + x] == 0) {
                    continue;
                }

                Collect_SpawnCoin(
                    demo,
                    R2D_Rect(
                        (float)(x * demo->tilemap.tile_width),
                        (float)(y * demo->tilemap.tile_height),
                        (float)demo->tilemap.tile_width,
                        (float)demo->tilemap.tile_height
                    )
                );
            }
        }
    }
}

static void Collect_LoadObjects(CollectDemo *demo)
{
    const R2D_TilemapObject *player_start = R2D_TilemapFindObject(&demo->tilemap, "PlayerStart");
    const int object_count = R2D_TilemapObjectCount(&demo->tilemap);
    static const Vector2 fallback_coins[] = {
        { 96.0f, 48.0f },
        { 192.0f, 64.0f },
        { 320.0f, 96.0f },
        { 112.0f, 176.0f },
        { 256.0f, 240.0f },
        { 384.0f, 288.0f }
    };

    if (player_start != 0) {
        demo->player = (Vector2) { player_start->rect.x, player_start->rect.y };
    }

    for (int i = 0; i < object_count && demo->coin_count < COLLECT_MAX_COINS; ++i) {
        const R2D_TilemapObject *object = R2D_TilemapObjectAt(&demo->tilemap, i);

        if (object != 0 && Collect_ObjectIsCoin(object)) {
            Collect_SpawnCoin(demo, object->rect);
        }
    }

    /* Keep the sample playable even when a map only defines the player spawn. */
    if (demo->coin_count == 0) {
        const int fallback_count = (int)(sizeof(fallback_coins) / sizeof(fallback_coins[0]));

        for (int i = 0; i < fallback_count && demo->coin_count < COLLECT_MAX_COINS; ++i) {
            Collect_SpawnCoin(demo, R2D_Rect(fallback_coins[i].x, fallback_coins[i].y, 12.0f, 12.0f));
        }
    }
}

static void Collect_StartFountainCinematic(CollectDemo *demo, const R2D_TilemapObject *trigger)
{
    Vector2 target;

    if (trigger == 0 ||
        R2D_CinematicActive(&demo->cinematic) ||
        (demo->event_flags & COLLECT_FLAG_FOUNTAIN_EVENT) != 0) {
        return;
    }

    target = (Vector2) {
        trigger->rect.x + trigger->rect.width * 0.5f,
        trigger->rect.y + trigger->rect.height * 0.5f
    };

    R2D_CinematicInit(&demo->cinematic);
    R2D_CinematicAddSfx(&demo->cinematic, demo->event_sfx, R2D_AUDIO_GROUP_SFX);
    R2D_CinematicAddMoveCamera(&demo->cinematic, target, 0.65f);
    R2D_CinematicAddDialog(&demo->cinematic, "The fountain hums. Events can lock input, move camera and show dialog.", 2.1f);
    R2D_CinematicAddSetFlag(&demo->cinematic, &demo->event_flags, COLLECT_FLAG_FOUNTAIN_EVENT, true);
    R2D_CinematicAddMoveCamera(&demo->cinematic, (Vector2) { demo->player.x + 8.0f, demo->player.y + 8.0f }, 0.55f);
    R2D_CinematicStart(&demo->cinematic, (Vector2) { demo->player.x + 8.0f, demo->player.y + 8.0f });
}

static void Collect_Init(void *user_data)
{
    CollectDemo *demo = (CollectDemo *)user_data;

    demo->player = (Vector2) { 32.0f, 32.0f };
    demo->camera = R2D_CameraCreate(R2D_VirtualWidth(demo->context), R2D_VirtualHeight(demo->context));
    demo->debug_draw = false;
    demo->player_direction = PLAYER_SOUTH;
    demo->collision_layer = -1;
    demo->pickup_layer = -1;
    demo->collision_debug_color = R2D_ColorFromHex(0xff5555cc);
    demo->coin_count = 0;
    demo->coins_collected = 0;
    demo->music_loaded = false;
    demo->event_flags = 0u;
    demo->trigger_timer = 0.0f;
    demo->trigger_text[0] = '\0';
    demo->player_anim_name[0] = '\0';
    R2D_EntityWorldInit(&demo->entities, demo);
    Collect_InitInput(demo);
    demo->player_sheet = R2D_LoadSpriteSheet(R2D_AssetPath("textures/DawnLike/Commissions/Mage.png"), 16, 16);
    demo->coin_sheet = R2D_LoadSpriteSheet(R2D_AssetPath("textures/Coin.png"), 16, 16);
    R2D_LoadSpriteAtlas(&demo->player_atlas, R2D_AssetPath("atlases/collect_player.r2atlas"));
    if (!R2D_AnimSetLoad(&demo->player_anims, R2D_AssetPath("animations/collect_player.r2anim"))) {
        R2D_AnimSetInit(&demo->player_anims);
        R2D_AnimSetAdd(&demo->player_anims, "idle", R2D_AnimFrames(0, 1, 1.0f, true));
        R2D_AnimSetAdd(&demo->player_anims, "walk", R2D_AnimFrames(0, 4, 8.0f, true));
    }
    if (!R2D_AnimSetLoad(&demo->coin_anims, R2D_AssetPath("animations/coin.r2anim"))) {
        R2D_AnimSetInit(&demo->coin_anims);
        R2D_AnimSetAdd(&demo->coin_anims, "spin", R2D_AnimFrames(0, 7, 10.0f, true));
    }
    Collect_PlayPlayerAnim(demo, "idle");
    R2D_AnimPlayNamed(&demo->coin_anim, &demo->coin_anims, "spin", R2D_AnimFrames(0, 7, 10.0f, true));
    R2D_TilemapLoadTiledJson(&demo->tilemap, R2D_AssetPath("tilemaps/collect.json"));
    demo->collision_layer = R2D_TilemapLayerIndex(&demo->tilemap, "Collision");
    demo->collision_debug_color = R2D_TilemapPropertyColor(
        R2D_TilemapLayerFindProperty(&demo->tilemap, demo->collision_layer, "debug_color"),
        demo->collision_debug_color
    );
    Collect_LoadCoinLayer(demo);
    Collect_LoadObjects(demo);
    demo->coin_sfx = Collect_LoadCoinSfx();
    demo->event_sfx = R2D_SfxPowerup();
    demo->music_loaded = R2D_MusicLoadSong(&demo->music, R2D_AssetPath("audio/music/Mario Bros..r2song"));
    if (demo->music_loaded) {
        R2D_MusicSetVolume(&demo->music, 0.045f);
        R2D_MusicPlay(&demo->music, true);
    }
}

static void Collect_Update(float dt, void *user_data)
{
    CollectDemo *demo = (CollectDemo *)user_data;
    const float speed = 82.0f;
    const Vector2 previous = demo->player;
    Vector2 movement = { 0.0f, 0.0f };
    Rectangle player_bounds;
    const bool input_locked = R2D_CinematicInputLocked(&demo->cinematic);

    R2D_InputUpdate(&demo->input);
    R2D_AudioMixerUpdate(dt);

    if (demo->music_loaded) {
        R2D_MusicUpdate(&demo->music);
    }

    if (!input_locked) {
        movement.x = R2D_InputAxis(&demo->input, "move_left", "move_right");
        movement.y = R2D_InputAxis(&demo->input, "move_up", "move_down");
    }

    if (movement.x < 0.0f) {
        demo->player_direction = PLAYER_WEST;
    }

    if (movement.x > 0.0f) {
        demo->player_direction = PLAYER_EAST;
    }

    if (movement.y < 0.0f) {
        if (movement.x == 0.0f) {
            demo->player_direction = PLAYER_NORTH;
        }
    }

    if (movement.y > 0.0f) {
        if (movement.x == 0.0f) {
            demo->player_direction = PLAYER_SOUTH;
        }
    }

    if (movement.x != 0.0f || movement.y != 0.0f) {
        const float length = sqrtf(movement.x * movement.x + movement.y * movement.y);

        if (length > 1.0f) {
            movement.x /= length;
            movement.y /= length;
        }

        movement.x *= speed * dt;
        movement.y *= speed * dt;
    }

    if (demo->collision_layer >= 0) {
        const Vector2 moved_bounds = R2D_TilemapMoveAndSlide(
            &demo->tilemap,
            demo->collision_layer,
            Collect_PlayerBounds(demo->player),
            movement,
            0
        );

        demo->player = (Vector2) {
            moved_bounds.x - COLLECT_PLAYER_BOUNDS_OFFSET_X,
            moved_bounds.y - COLLECT_PLAYER_BOUNDS_OFFSET_Y
        };
    } else {
        demo->player.x += movement.x;
        demo->player.y += movement.y;
    }

    if (R2D_InputPressed(&demo->input, "debug")) {
        demo->debug_draw = !demo->debug_draw;
    }

    player_bounds = Collect_PlayerBounds(demo->player);
    {
        R2D_Collider triggers[16];
        R2D_CollisionHit hits[4];
        const int trigger_count = R2D_TilemapTriggerColliders(&demo->tilemap, triggers, 16, 2u, 1u);
        const int hit_count = R2D_CollisionQueryRect(player_bounds, 1u, 2u, triggers, trigger_count, hits, 4);

        if (hit_count > 0) {
            const R2D_TilemapObject *trigger = (const R2D_TilemapObject *)hits[0].user_data;
            const char *event = R2D_TilemapPropertyString(
                R2D_TilemapObjectFindProperty(trigger, "event"),
                trigger != 0 && trigger->name[0] != '\0' ? trigger->name : "trigger"
            );

            snprintf(demo->trigger_text, sizeof(demo->trigger_text), "TRIGGER: %s", event);
            demo->trigger_timer = 1.2f;

            if (strcmp(event, "fountain") == 0) {
                Collect_StartFountainCinematic(demo, trigger);
            }
        }
    }

    if (demo->trigger_timer > 0.0f) {
        demo->trigger_timer -= dt;
        if (demo->trigger_timer < 0.0f) {
            demo->trigger_timer = 0.0f;
        }
    }

    {
        int cursor = 0;
        R2D_Entity *coin;

        while ((coin = R2D_EntityFindByType(&demo->entities, COLLECT_ENTITY_COIN, &cursor)) != 0) {
            if (!CheckCollisionRecs(player_bounds, coin->bounds)) {
                continue;
            }

            R2D_EntityDestroy(&demo->entities, coin->id);
            demo->coins_collected++;
            R2D_PlaySfxRandomPitch(demo->coin_sfx, R2D_AUDIO_GROUP_SFX, 0.6f);
        }
    }

    if (previous.x != demo->player.x || previous.y != demo->player.y) {
        Collect_PlayPlayerAnim(demo, "walk");
    } else {
        Collect_PlayPlayerAnim(demo, "idle");
    }

    R2D_AnimUpdate(&demo->player_anim, dt);
    R2D_AnimUpdate(&demo->coin_anim, dt);
    if (R2D_CinematicActive(&demo->cinematic)) {
        R2D_CinematicUpdate(&demo->cinematic, dt, (Vector2) { demo->player.x + 8.0f, demo->player.y + 8.0f });
        R2D_CameraFollow(
            &demo->camera,
            R2D_CinematicCameraPosition(&demo->cinematic, (Vector2) { demo->player.x + 8.0f, demo->player.y + 8.0f })
        );
    } else {
        R2D_CameraFollow(&demo->camera, (Vector2) { demo->player.x + 8.0f, demo->player.y + 8.0f });
    }

    if (R2D_TilemapIsReady(&demo->tilemap)) {
        R2D_CameraClampToRect(
            &demo->camera,
            R2D_Rect(
                0.0f,
                0.0f,
                (float)(demo->tilemap.width * demo->tilemap.tile_width),
                (float)(demo->tilemap.height * demo->tilemap.tile_height)
            )
        );
    }
}

static void Collect_DrawHud(const CollectDemo *demo)
{
    char text[64];

    snprintf(text, sizeof(text), "COINS %02d/%02d", demo->coins_collected, demo->coin_count);
    DrawRectangle(5, 5, 94, 26, R2D_ColorFromHex(0x101820dd));
    DrawRectangleLines(5, 5, 94, 26, R2D_ColorFromHex(0xffd166ff));
    DrawText(text, 10, 11, 10, R2D_ColorFromHex(0xf8f8f2ff));

    DrawText(demo->debug_draw ? "F3 DEBUG ON" : "F3 DEBUG", 242, 11, 8, R2D_ColorFromHex(0x8ecae6ff));
}

static void Collect_DrawClearMessage(const CollectDemo *demo)
{
    const int panel_width = 132;
    const int panel_height = 38;
    const int x = R2D_VirtualWidth(demo->context) / 2 - panel_width / 2;
    const int y = R2D_VirtualHeight(demo->context) / 2 - panel_height / 2;

    if (demo->coins_collected != demo->coin_count || demo->coin_count <= 0) {
        return;
    }

    DrawRectangle(x, y, panel_width, panel_height, R2D_ColorFromHex(0x101820dd));
    DrawRectangleLines(x, y, panel_width, panel_height, R2D_ColorFromHex(0xffd166ff));
    DrawText("ALL CLEAR", x + 31, y + 8, 12, R2D_ColorFromHex(0x06d6a0ff));
    DrawText("coins collected", x + 23, y + 23, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

static void Collect_DrawTriggerMessage(const CollectDemo *demo)
{
    if (demo->trigger_timer <= 0.0f || demo->trigger_text[0] == '\0') {
        return;
    }

    DrawRectangle(90, 114, 140, 16, R2D_ColorFromHex(0x101820dd));
    DrawRectangleLines(90, 114, 140, 16, R2D_ColorFromHex(0x8ecae6cc));
    DrawText(demo->trigger_text, 96, 118, 8, R2D_ColorFromHex(0x8ecae6ff));
}

static size_t Collect_DebugMemoryBytes(const CollectDemo *demo)
{
    size_t bytes = sizeof(*demo);

    bytes += (size_t)R2D_EntityCount(&demo->entities) * sizeof(R2D_Entity);

    for (int i = 0; i < demo->tilemap.layer_count; ++i) {
        bytes += (size_t)demo->tilemap.layers[i].width *
            (size_t)demo->tilemap.layers[i].height *
            sizeof(unsigned int);
    }

    for (int i = 0; i < demo->tilemap.tileset_count; ++i) {
        for (int animation = 0; animation < demo->tilemap.tilesets[i].animation_count; ++animation) {
            bytes += (size_t)demo->tilemap.tilesets[i].animations[animation].frame_count *
                sizeof(R2D_TilemapAnimationFrame);
        }
    }

    return bytes;
}

static void Collect_DrawDebugTileCursor(const CollectDemo *demo, Vector2 camera_pixel)
{
    const Vector2 mouse = R2D_MouseVirtualPosition(demo->context);
    const Vector2 world = { mouse.x + camera_pixel.x, mouse.y + camera_pixel.y };
    const Vector2 tile = R2D_TilemapWorldToTile(&demo->tilemap, world);
    const int tile_x = (int)tile.x;
    const int tile_y = (int)tile.y;

    if (!R2D_TilemapIsReady(&demo->tilemap) ||
        tile_x < 0 || tile_y < 0 ||
        tile_x >= demo->tilemap.width || tile_y >= demo->tilemap.height) {
        return;
    }

    R2D_DrawRectangleLinesCamera(
        &demo->camera,
        R2D_Rect(
            (float)(tile_x * demo->tilemap.tile_width),
            (float)(tile_y * demo->tilemap.tile_height),
            (float)demo->tilemap.tile_width,
            (float)demo->tilemap.tile_height
        ),
        1.0f,
        R2D_ColorFromHex(0xffd166ff)
    );
}

static void Collect_DrawDebugOverlay(const CollectDemo *demo, Vector2 camera_pixel)
{
    R2D_DebugInfo info = R2D_DebugInfoDefault();
    const Vector2 mouse = R2D_MouseVirtualPosition(demo->context);
    const Vector2 world = { mouse.x + camera_pixel.x, mouse.y + camera_pixel.y };
    const Vector2 tile = R2D_TilemapWorldToTile(&demo->tilemap, world);

    info.title = "COLLECT DEBUG";
    info.entity_count = R2D_EntityCount(&demo->entities);
    info.memory_bytes = Collect_DebugMemoryBytes(demo);
    info.tile_x = (int)tile.x;
    info.tile_y = (int)tile.y;
    info.tile_gid = demo->collision_layer >= 0 ?
        R2D_TilemapTileAt(&demo->tilemap, demo->collision_layer, info.tile_x, info.tile_y) :
        0u;
    info.line = demo->trigger_timer > 0.0f ? demo->trigger_text : "camera, collision, triggers";

    R2D_DebugDrawOverlay(&info, 6, 36);
}

static void Collect_DrawDebugPath(const CollectDemo *demo)
{
    const R2D_TilemapObject *trigger = R2D_TilemapFindObject(&demo->tilemap, "FountainTrigger");
    R2D_GridPoint path[128];
    R2D_GridPoint start;
    R2D_GridPoint goal;
    int path_count;
    bool line_of_sight;
    Color path_color;

    if (trigger == 0 || demo->collision_layer < 0 || !R2D_TilemapIsReady(&demo->tilemap)) {
        return;
    }

    start = R2D_GridPointMake(
        (int)((demo->player.x + 8.0f) / (float)demo->tilemap.tile_width),
        (int)((demo->player.y + 8.0f) / (float)demo->tilemap.tile_height)
    );
    goal = R2D_GridPointMake(
        (int)((trigger->rect.x + trigger->rect.width * 0.5f) / (float)demo->tilemap.tile_width),
        (int)((trigger->rect.y + trigger->rect.height * 0.5f) / (float)demo->tilemap.tile_height)
    );

    path_count = R2D_TilemapFindPath(&demo->tilemap, demo->collision_layer, start, goal, path, 128);
    line_of_sight = R2D_TilemapLineOfSight(&demo->tilemap, demo->collision_layer, start, goal);
    path_color = line_of_sight ? R2D_ColorFromHex(0x06d6a0cc) : R2D_ColorFromHex(0xffd166cc);

    for (int i = 0; i < path_count; ++i) {
        R2D_DrawRectangleCamera(
            &demo->camera,
            R2D_Rect(
                (float)(path[i].x * demo->tilemap.tile_width) + 5.0f,
                (float)(path[i].y * demo->tilemap.tile_height) + 5.0f,
                6.0f,
                6.0f
            ),
            path_color
        );
    }

    {
        const Vector2 start_screen = R2D_CameraWorldToPixelScreen(
            &demo->camera,
            (Vector2) {
                (float)(start.x * demo->tilemap.tile_width) + 8.0f,
                (float)(start.y * demo->tilemap.tile_height) + 8.0f
            }
        );
        const Vector2 goal_screen = R2D_CameraWorldToPixelScreen(
            &demo->camera,
            (Vector2) {
                (float)(goal.x * demo->tilemap.tile_width) + 8.0f,
                (float)(goal.y * demo->tilemap.tile_height) + 8.0f
            }
        );

        DrawLine((int)start_screen.x, (int)start_screen.y, (int)goal_screen.x, (int)goal_screen.y, path_color);
    }
}

static void Collect_Draw(void *user_data)
{
    const CollectDemo *demo = (const CollectDemo *)user_data;
    const Vector2 camera_pixel = R2D_CameraPixelPosition(&demo->camera);
    const Vector2 camera_offset = { -camera_pixel.x, -camera_pixel.y };
    const Rectangle camera_view = R2D_Rect(
        camera_pixel.x,
        camera_pixel.y,
        (float)demo->camera.viewport_width,
        (float)demo->camera.viewport_height
    );
    const Vector2 player_screen = R2D_CameraWorldToPixelScreen(&demo->camera, demo->player);
    const int player_frame =
        Collect_PlayerDirectionRow(demo->player_direction) * 4 +
        (R2D_AnimFrame(&demo->player_anim) % 4);
    char player_frame_name[32];
    const R2D_SpriteAtlasFrame *player_atlas_frame;

    Collect_DrawTileLayers(demo, camera_view, (Vector2) { 0.0f, 0.0f }, false);

    for (int i = 0; i < R2D_EntityCount(&demo->entities); ++i) {
        const R2D_Entity *coin = R2D_EntityAtConst(&demo->entities, i);
        Vector2 position;

        if (coin == 0 || coin->type != COLLECT_ENTITY_COIN) {
            continue;
        }

        position = (Vector2) { coin->bounds.x, coin->bounds.y };
        R2D_DrawAnimCamera(&demo->camera, &demo->coin_sheet, &demo->coin_anim, position, false);
    }

    snprintf(
        player_frame_name,
        sizeof(player_frame_name),
        "%s_%d",
        Collect_PlayerDirectionName(demo->player_direction),
        R2D_AnimFrame(&demo->player_anim) % 4
    );
    player_atlas_frame = R2D_SpriteAtlasFind(&demo->player_atlas, player_frame_name);
    if (player_atlas_frame != 0) {
        R2D_DrawAtlasFrameExCamera(
            &demo->camera,
            &demo->player_atlas,
            player_atlas_frame,
            (Vector2) { demo->player.x + 8.0f, demo->player.y + 8.0f },
            0.0f,
            1.0f,
            false,
            WHITE
        );
    } else {
        R2D_DrawSheetFrameCamera(&demo->camera, &demo->player_sheet, player_frame, demo->player, false);
    }
    Collect_DrawTileLayers(demo, camera_view, (Vector2) { 0.0f, 0.0f }, true);

    if (demo->debug_draw) {
        R2D_TilemapDrawCollisionDebugVisible(
            &demo->tilemap,
            demo->collision_layer,
            camera_view,
            camera_offset,
            demo->collision_debug_color
        );
        R2D_TilemapDrawObjectsDebug(&demo->tilemap, camera_offset, R2D_ColorFromHex(0x8ecae6cc));
        Collect_DrawDebugPath(demo);
        Collect_DrawDebugTileCursor(demo, camera_pixel);
        DrawRectangleLines(0, 0, demo->camera.viewport_width, demo->camera.viewport_height, R2D_ColorFromHex(0xf8f8f255));
        DrawRectangleLinesEx(
            R2D_Rect(player_screen.x + 3.0f, player_screen.y + 2.0f, 10.0f, 13.0f),
            1.0f,
            R2D_ColorFromHex(0xf72585ff)
        );
        if (player_atlas_frame != 0) {
            DrawRectangleLinesEx(
                R2D_SpriteAtlasHitbox(player_atlas_frame, (Vector2) { player_screen.x + 8.0f, player_screen.y + 8.0f }),
                1.0f,
                R2D_ColorFromHex(0xffd166ff)
            );
            DrawRectangleLinesEx(
                R2D_SpriteAtlasHurtbox(player_atlas_frame, (Vector2) { player_screen.x + 8.0f, player_screen.y + 8.0f }),
                1.0f,
                R2D_ColorFromHex(0x06d6a0ff)
            );
        }
        Collect_DrawDebugOverlay(demo, camera_pixel);
    }

    Collect_DrawHud(demo);
    Collect_DrawClearMessage(demo);
    Collect_DrawTriggerMessage(demo);
    R2D_CinematicDrawDialog(
        &demo->cinematic,
        R2D_Rect(24.0f, 148.0f, 272.0f, 42.0f),
        R2D_DefaultUiStyle()
    );
}

static void Collect_Shutdown(void *user_data)
{
    CollectDemo *demo = (CollectDemo *)user_data;

    R2D_TilemapUnload(&demo->tilemap);
    R2D_UnloadSpriteAtlas(&demo->player_atlas);
    R2D_UnloadSpriteSheet(&demo->player_sheet);
    R2D_UnloadSpriteSheet(&demo->coin_sheet);
    R2D_MusicUnload(&demo->music);
}

int main(int argc, char **argv)
{
    R2D_Context context = { 0 };
    R2D_Crt crt = { 0 };
    R2D_RuntimeConfig runtime = R2D_RuntimeConfigDefault();
    CollectDemo demo = { 0 };

    runtime.config.title = "Retro2DFramework Collect";
    runtime.config.clear_color = R2D_ColorFromHex(0x101820ff);
    {
        char runtime_config_path[1024];

        snprintf(runtime_config_path, sizeof(runtime_config_path), "%sr2d.ini", GetApplicationDirectory());
        R2D_RuntimeConfigLoad(&runtime, runtime_config_path);
    }
    R2D_RuntimeConfigApplyArgs(&runtime, argc, argv);

    if (!R2D_Init(&context, runtime.config)) {
        return 1;
    }

    R2D_AudioInit();
    R2D_RuntimeConfigApplyAudio(&runtime);
    R2D_CrtInit(&crt);
    R2D_RuntimeConfigApplyCrt(&runtime, &crt);
    R2D_SetCrt(&context, &crt);
    demo.context = &context;

    R2D_Run(&context, (R2D_App) {
        Collect_Init,
        Collect_Update,
        Collect_Draw,
        Collect_Shutdown,
        &demo
    });

    R2D_CrtClose(&crt);
    R2D_AudioClose();
    R2D_Close(&context);
    return 0;
}
