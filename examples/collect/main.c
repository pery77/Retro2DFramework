#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define COLLECT_MAX_COINS 32
#define COLLECT_PLAYER_BOUNDS_OFFSET_X 3.0f
#define COLLECT_PLAYER_BOUNDS_OFFSET_Y 2.0f

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
    R2D_Anim idle_anim;
    R2D_Anim walk_anim;
    R2D_AnimPlayer player_anim;
    R2D_AnimPlayer coin_anim;
    R2D_InputMap input;
    R2D_Sfx coin_sfx;
    R2D_Music music;
    R2D_Context *context;
    int collision_layer;
    int coin_count;
    int coins_collected;
    Color collision_debug_color;
    PlayerDirection player_direction;
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

static void Collect_DrawTileLayers(
    const CollectDemo *demo,
    Rectangle camera_view,
    Vector2 camera_offset,
    bool above_player
)
{
    if (!R2D_TilemapIsReady(&demo->tilemap)) {
        return;
    }

    for (int i = 0; i < demo->tilemap.layer_count; ++i) {
        if (i == demo->collision_layer) {
            continue;
        }

        if (Collect_LayerDrawsAbovePlayer(demo->tilemap.layers[i].name) == above_player) {
            R2D_TilemapDrawLayerVisible(&demo->tilemap, i, camera_view, camera_offset);
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

static void Collect_Init(void *user_data)
{
    CollectDemo *demo = (CollectDemo *)user_data;

    demo->player = (Vector2) { 32.0f, 32.0f };
    demo->camera = R2D_CameraCreate(R2D_VirtualWidth(demo->context), R2D_VirtualHeight(demo->context));
    demo->debug_draw = false;
    demo->player_direction = PLAYER_SOUTH;
    demo->collision_layer = -1;
    demo->collision_debug_color = R2D_ColorFromHex(0xff5555cc);
    demo->coin_count = 0;
    demo->coins_collected = 0;
    demo->music_loaded = false;
    R2D_EntityWorldInit(&demo->entities, demo);
    Collect_InitInput(demo);
    demo->player_sheet = R2D_LoadSpriteSheet(R2D_AssetPath("textures/DawnLike/Commissions/Mage.png"), 16, 16);
    demo->coin_sheet = R2D_LoadSpriteSheet(R2D_AssetPath("textures/Coin.png"), 16, 16);
    demo->idle_anim = R2D_AnimFrames(0, 1, 1.0f, true);
    demo->walk_anim = R2D_AnimFrames(0, 4, 8.0f, true);
    R2D_AnimPlay(&demo->player_anim, demo->idle_anim);
    R2D_AnimPlay(&demo->coin_anim, R2D_AnimFrames(0, 7, 10.0f, true));
    R2D_TilemapLoadTiledJson(&demo->tilemap, R2D_AssetPath("tilemaps/collect.json"));
    demo->collision_layer = R2D_TilemapLayerIndex(&demo->tilemap, "Collision");
    demo->collision_debug_color = R2D_TilemapPropertyColor(
        R2D_TilemapLayerFindProperty(&demo->tilemap, demo->collision_layer, "debug_color"),
        demo->collision_debug_color
    );
    Collect_LoadObjects(demo);
    demo->coin_sfx = Collect_LoadCoinSfx();
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

    R2D_InputUpdate(&demo->input);

    if (demo->music_loaded) {
        R2D_MusicUpdate(&demo->music);
    }

    movement.x = R2D_InputAxis(&demo->input, "move_left", "move_right");
    movement.y = R2D_InputAxis(&demo->input, "move_up", "move_down");

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
        int cursor = 0;
        R2D_Entity *coin;

        while ((coin = R2D_EntityFindByType(&demo->entities, COLLECT_ENTITY_COIN, &cursor)) != 0) {
            if (!CheckCollisionRecs(player_bounds, coin->bounds)) {
                continue;
            }

            R2D_EntityDestroy(&demo->entities, coin->id);
            demo->coins_collected++;
            R2D_PlaySfx(demo->coin_sfx);
        }
    }

    if (previous.x != demo->player.x || previous.y != demo->player.y) {
        if (demo->player_anim.anim.frame_count != demo->walk_anim.frame_count) {
            R2D_AnimPlay(&demo->player_anim, demo->walk_anim);
        }
    } else if (demo->player_anim.anim.frame_count != demo->idle_anim.frame_count) {
        R2D_AnimPlay(&demo->player_anim, demo->idle_anim);
    }

    R2D_AnimUpdate(&demo->player_anim, dt);
    R2D_AnimUpdate(&demo->coin_anim, dt);
    R2D_CameraFollow(&demo->camera, (Vector2) { demo->player.x + 8.0f, demo->player.y + 8.0f });

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
    const Vector2 player_screen = {
        floorf(demo->player.x - camera_pixel.x),
        floorf(demo->player.y - camera_pixel.y)
    };
    const int player_frame =
        Collect_PlayerDirectionRow(demo->player_direction) * 4 +
        (R2D_AnimFrame(&demo->player_anim) % 4);

    Collect_DrawTileLayers(demo, camera_view, camera_offset, false);

    for (int i = 0; i < R2D_EntityCount(&demo->entities); ++i) {
        const R2D_Entity *coin = R2D_EntityAtConst(&demo->entities, i);
        Vector2 position;

        if (coin == 0 || coin->type != COLLECT_ENTITY_COIN) {
            continue;
        }

        position = (Vector2) {
            floorf(coin->bounds.x - camera_pixel.x),
            floorf(coin->bounds.y - camera_pixel.y)
        };
        R2D_DrawAnim(&demo->coin_sheet, &demo->coin_anim, position, false);
    }

    R2D_DrawSheetFrame(&demo->player_sheet, player_frame, player_screen, false);
    Collect_DrawTileLayers(demo, camera_view, camera_offset, true);

    if (demo->debug_draw) {
        R2D_TilemapDrawCollisionDebugVisible(
            &demo->tilemap,
            demo->collision_layer,
            camera_view,
            camera_offset,
            demo->collision_debug_color
        );
        R2D_TilemapDrawObjectsDebug(&demo->tilemap, camera_offset, R2D_ColorFromHex(0x8ecae6cc));
        DrawRectangleLinesEx(
            R2D_Rect(player_screen.x + 3.0f, player_screen.y + 2.0f, 10.0f, 13.0f),
            1.0f,
            R2D_ColorFromHex(0xf72585ff)
        );
    }

    Collect_DrawHud(demo);
    Collect_DrawClearMessage(demo);
}

static void Collect_Shutdown(void *user_data)
{
    CollectDemo *demo = (CollectDemo *)user_data;

    R2D_TilemapUnload(&demo->tilemap);
    R2D_UnloadSpriteSheet(&demo->player_sheet);
    R2D_UnloadSpriteSheet(&demo->coin_sheet);
    R2D_MusicUnload(&demo->music);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Crt crt = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    CollectDemo demo = { 0 };

    config.title = "Retro2DFramework Collect";
    config.clear_color = R2D_ColorFromHex(0x101820ff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_AudioInit();
    R2D_CrtInit(&crt);
    R2D_CrtSetEnabled(&crt, true);
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
