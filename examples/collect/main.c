#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define COLLECT_MAX_COINS 32

typedef struct Coin {
    Rectangle rect;
    bool collected;
} Coin;

typedef struct CollectDemo {
    Vector2 player;
    R2D_Camera camera;
    R2D_Tilemap tilemap;
    R2D_SpriteSheet player_sheet;
    R2D_SpriteSheet coin_sheet;
    R2D_Anim idle_anim;
    R2D_Anim walk_anim;
    R2D_AnimPlayer player_anim;
    R2D_AnimPlayer coin_anim;
    R2D_Sfx coin_sfx;
    R2D_Music music;
    R2D_Context *context;
    R2D_Crt *crt;
    int collision_layer;
    int coin_count;
    int coins_collected;
    Coin coins[COLLECT_MAX_COINS];
    bool facing_left;
    bool debug_draw;
    bool music_loaded;
} CollectDemo;

static Texture2D Collect_CreatePlayerTexture(void)
{
    Image image = GenImageColor(64, 16, BLANK);
    Texture2D texture;

    for (int frame = 0; frame < 4; ++frame) {
        const int x = frame * 16;
        const int step = frame == 1 ? 1 : frame == 3 ? -1 : 0;

        ImageDrawRectangle(&image, x + 5, 3, 6, 10, R2D_ColorFromHex(0x4cc9f0ff));
        ImageDrawRectangle(&image, x + 4, 2, 8, 3, R2D_ColorFromHex(0xf72585ff));
        ImageDrawRectangle(&image, x + 6, 0, 4, 3, R2D_ColorFromHex(0xffd166ff));
        ImageDrawRectangle(&image, x + 3, 6, 2, 5, R2D_ColorFromHex(0xf8f8f2ff));
        ImageDrawRectangle(&image, x + 11, 6, 2, 5, R2D_ColorFromHex(0xf8f8f2ff));
        ImageDrawRectangle(&image, x + 5 + step, 13, 3, 3, R2D_ColorFromHex(0x06d6a0ff));
        ImageDrawRectangle(&image, x + 9 - step, 13, 3, 3, R2D_ColorFromHex(0x06d6a0ff));
        ImageDrawPixel(&image, x + 10, 3, BLACK);
    }

    texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

static R2D_Sfx Collect_LoadCoinSfx(void)
{
    R2D_Sfx sfx = R2D_SfxCoin();

    R2D_LoadSfx(R2D_AssetPath("audio/sfx/coin.r2sfx"), &sfx);
    return sfx;
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
    return R2D_Rect(position.x + 3.0f, position.y + 2.0f, 10.0f, 13.0f);
}

static bool Collect_PlayerCollides(const CollectDemo *demo, Vector2 position)
{
    if (demo->collision_layer < 0) {
        return false;
    }

    return R2D_TilemapRectCollides(&demo->tilemap, demo->collision_layer, Collect_PlayerBounds(position));
}

static void Collect_LoadObjects(CollectDemo *demo)
{
    const R2D_TilemapObject *player_start = R2D_TilemapFindObject(&demo->tilemap, "PlayerStart");
    const int object_count = R2D_TilemapObjectCount(&demo->tilemap);

    if (player_start != 0) {
        demo->player = (Vector2) { player_start->rect.x, player_start->rect.y };
    }

    for (int i = 0; i < object_count && demo->coin_count < COLLECT_MAX_COINS; ++i) {
        const R2D_TilemapObject *object = R2D_TilemapObjectAt(&demo->tilemap, i);

        if (object != 0 && Collect_ObjectIsCoin(object)) {
            demo->coins[demo->coin_count].rect = object->rect;
            demo->coins[demo->coin_count].collected = false;
            demo->coin_count++;
        }
    }
}

static void Collect_Init(void *user_data)
{
    CollectDemo *demo = (CollectDemo *)user_data;

    demo->player = (Vector2) { 32.0f, 32.0f };
    demo->camera = R2D_CameraCreate(R2D_VirtualWidth(demo->context), R2D_VirtualHeight(demo->context));
    demo->debug_draw = false;
    demo->facing_left = false;
    demo->collision_layer = -1;
    demo->coin_count = 0;
    demo->coins_collected = 0;
    demo->music_loaded = false;
    demo->player_sheet = R2D_SpriteSheetFromTexture(Collect_CreatePlayerTexture(), 16, 16);
    demo->coin_sheet = R2D_LoadSpriteSheet(R2D_AssetPath("textures/Dungeon/Coin Sheet.png"), 16, 16);
    demo->idle_anim = R2D_AnimFrames(0, 2, 3.0f, true);
    demo->walk_anim = R2D_AnimFrames(0, 4, 10.0f, true);
    R2D_AnimPlay(&demo->player_anim, demo->idle_anim);
    R2D_AnimPlay(&demo->coin_anim, R2D_AnimFrames(0, 8, 10.0f, true));
    R2D_TilemapLoadTiledJson(&demo->tilemap, R2D_AssetPath("tilemaps/collect.json"));
    demo->collision_layer = R2D_TilemapLayerIndex(&demo->tilemap, "Collision");
    Collect_LoadObjects(demo);
    demo->coin_sfx = Collect_LoadCoinSfx();
    demo->music_loaded = R2D_MusicLoadSong(&demo->music, R2D_AssetPath("audio/music/Mario Bros..r2song"));
    if (demo->music_loaded) {
        R2D_MusicSetVolume(&demo->music, 0.45f);
        R2D_MusicPlay(&demo->music, true);
    }
}

static void Collect_Update(float dt, void *user_data)
{
    CollectDemo *demo = (CollectDemo *)user_data;
    const float speed = 82.0f;
    const Vector2 previous = demo->player;
    Vector2 movement = { 0.0f, 0.0f };
    Vector2 next;
    Rectangle player_bounds;

    if (demo->music_loaded) {
        R2D_MusicUpdate(&demo->music);
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        movement.x -= 1.0f;
        demo->facing_left = true;
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        movement.x += 1.0f;
        demo->facing_left = false;
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        movement.y -= 1.0f;
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        movement.y += 1.0f;
    }

    if (movement.x != 0.0f || movement.y != 0.0f) {
        const float length = sqrtf(movement.x * movement.x + movement.y * movement.y);
        movement.x = movement.x / length * speed * dt;
        movement.y = movement.y / length * speed * dt;
    }

    next = (Vector2) { demo->player.x + movement.x, demo->player.y };
    if (!Collect_PlayerCollides(demo, next)) {
        demo->player.x = next.x;
    }

    next = (Vector2) { demo->player.x, demo->player.y + movement.y };
    if (!Collect_PlayerCollides(demo, next)) {
        demo->player.y = next.y;
    }

    if (IsKeyPressed(KEY_F3)) {
        demo->debug_draw = !demo->debug_draw;
    }

    player_bounds = Collect_PlayerBounds(demo->player);
    for (int i = 0; i < demo->coin_count; ++i) {
        if (!demo->coins[i].collected && CheckCollisionRecs(player_bounds, demo->coins[i].rect)) {
            demo->coins[i].collected = true;
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

    if (demo->coins_collected == demo->coin_count && demo->coin_count > 0) {
        DrawText("ALL CLEAR", 122, 11, 10, R2D_ColorFromHex(0x06d6a0ff));
    }

    DrawText(demo->debug_draw ? "F3 DEBUG ON" : "F3 DEBUG", 242, 11, 8, R2D_ColorFromHex(0x8ecae6ff));
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

    Collect_DrawTileLayers(demo, camera_view, camera_offset, false);

    for (int i = 0; i < demo->coin_count; ++i) {
        const Coin *coin = &demo->coins[i];
        Vector2 position;

        if (coin->collected) {
            continue;
        }

        position = (Vector2) {
            floorf(coin->rect.x - camera_pixel.x),
            floorf(coin->rect.y - camera_pixel.y)
        };
        R2D_DrawAnim(&demo->coin_sheet, &demo->coin_anim, position, false);
    }

    R2D_DrawAnim(&demo->player_sheet, &demo->player_anim, player_screen, demo->facing_left);
    Collect_DrawTileLayers(demo, camera_view, camera_offset, true);

    if (demo->debug_draw) {
        R2D_TilemapDrawCollisionDebugVisible(
            &demo->tilemap,
            demo->collision_layer,
            camera_view,
            camera_offset,
            R2D_ColorFromHex(0xff5555cc)
        );
        R2D_TilemapDrawObjectsDebug(&demo->tilemap, camera_offset, R2D_ColorFromHex(0x8ecae6cc));
        DrawRectangleLinesEx(
            R2D_Rect(player_screen.x + 3.0f, player_screen.y + 2.0f, 10.0f, 13.0f),
            1.0f,
            R2D_ColorFromHex(0xf72585ff)
        );
    }

    Collect_DrawHud(demo);
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
    demo.crt = &crt;

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
