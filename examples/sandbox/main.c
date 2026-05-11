#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Sandbox {
    Vector2 player;
    float blink_timer;
    float reload_message_timer;
    int collision_layer;
    R2D_Camera camera;
    bool debug_draw;
    bool facing_left;
    bool moving;
    R2D_SpriteSheet player_sheet;
    R2D_Anim idle_anim;
    R2D_Anim walk_anim;
    R2D_AnimPlayer player_anim;
    R2D_Tilemap tilemap;
    R2D_Sfx coin;
    R2D_Sfx hit;
    R2D_Sfx jump;
    R2D_Sfx laser;
    R2D_Sfx explosion;
    R2D_Sfx powerup;
    R2D_Music music;
    bool reload_ok;
    bool music_loaded;
    R2D_Context *context;
    R2D_Crt *crt;
} Sandbox;

static void Sandbox_AddFallbackCollision(R2D_Tilemap *tilemap)
{
    R2D_TilemapLayer *layers;
    R2D_TilemapLayer *collision;
    const int old_count = tilemap->layer_count;

    if (!R2D_TilemapIsReady(tilemap) || R2D_TilemapLayerIndex(tilemap, "Collision") >= 0) {
        return;
    }

    layers = (R2D_TilemapLayer *)realloc(
        tilemap->layers,
        (size_t)(old_count + 1) * sizeof(R2D_TilemapLayer)
    );

    if (layers == 0) {
        return;
    }

    tilemap->layers = layers;
    collision = &tilemap->layers[old_count];
    *collision = (R2D_TilemapLayer) { 0 };
    snprintf(collision->name, sizeof(collision->name), "%s", "Collision");
    collision->width = tilemap->width;
    collision->height = tilemap->height;
    collision->visible = false;
    collision->tiles = (unsigned int *)calloc((size_t)collision->width * (size_t)collision->height, sizeof(unsigned int));

    if (collision->tiles == 0) {
        return;
    }

    for (int y = 0; y < collision->height; ++y) {
        for (int x = 0; x < collision->width; ++x) {
            const bool border = x == 0 || y == 0 || x == collision->width - 1 || y == collision->height - 1;
            const bool block = (x >= 9 && x <= 12 && y >= 10 && y <= 12) || (x >= 18 && x <= 21 && y >= 18 && y <= 20);

            collision->tiles[y * collision->width + x] = border || block ? 1u : 0u;
        }
    }

    tilemap->layer_count = old_count + 1;
}

static Texture2D Sandbox_CreatePlayerTexture(void)
{
    Image image = GenImageColor(64, 16, BLANK);
    Texture2D texture;

    for (int frame = 0; frame < 4; ++frame) {
        const int x = frame * 16;
        const int step = frame == 1 ? 1 : frame == 3 ? -1 : 0;

        ImageDrawRectangle(&image, x + 5, 3, 6, 10, R2D_ColorFromHex(0xff5555ff));
        ImageDrawRectangle(&image, x + 4, 2, 8, 3, R2D_ColorFromHex(0xf1fa8cff));
        ImageDrawRectangle(&image, x + 6, 0, 4, 3, R2D_ColorFromHex(0xffb86cff));
        ImageDrawRectangle(&image, x + 3, 6, 2, 5, R2D_ColorFromHex(0x8be9fdff));
        ImageDrawRectangle(&image, x + 11, 6, 2, 5, R2D_ColorFromHex(0x8be9fdff));
        ImageDrawRectangle(&image, x + 5 + step, 13, 3, 3, R2D_ColorFromHex(0x50fa7bff));
        ImageDrawRectangle(&image, x + 9 - step, 13, 3, 3, R2D_ColorFromHex(0x50fa7bff));
        ImageDrawPixel(&image, x + 10, 3, BLACK);
    }

    texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

static R2D_Sfx Sandbox_LoadSfx(const char *path, R2D_Sfx fallback)
{
    R2D_Sfx sfx = fallback;

    R2D_LoadSfx(R2D_AssetPath(path), &sfx);
    return sfx;
}

static bool Sandbox_PlayerCollides(const Sandbox *sandbox, Vector2 position)
{
    const Rectangle bounds = R2D_Rect(position.x + 3.0f, position.y + 2.0f, 10.0f, 13.0f);

    return R2D_TilemapRectCollides(&sandbox->tilemap, sandbox->collision_layer, bounds);
}

static void Sandbox_Init(void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;
    const R2D_TilemapObject *player_start;
    char midi_path[1024];
    char soundfont_path[1024];
    char song_path[1024];

    sandbox->player = (Vector2) { 152.0f, 82.0f };
    sandbox->camera = R2D_CameraCreate(R2D_VirtualWidth(sandbox->context), R2D_VirtualHeight(sandbox->context));
    sandbox->blink_timer = 0.0f;
    sandbox->reload_message_timer = 0.0f;
    sandbox->facing_left = false;
    sandbox->moving = false;
    sandbox->debug_draw = false;
    sandbox->player_sheet = R2D_SpriteSheetFromTexture(Sandbox_CreatePlayerTexture(), 16, 16);
    sandbox->idle_anim = R2D_AnimFrames(0, 2, 3.0f, true);
    sandbox->walk_anim = R2D_AnimFrames(0, 4, 10.0f, true);
    R2D_AnimPlay(&sandbox->player_anim, sandbox->idle_anim);
    R2D_TilemapLoadTiledJson(&sandbox->tilemap, R2D_AssetPath("tilemaps/r2d_sandbox.json"));
    Sandbox_AddFallbackCollision(&sandbox->tilemap);
    sandbox->collision_layer = R2D_TilemapLayerIndex(&sandbox->tilemap, "Collision");
    player_start = R2D_TilemapFindObject(&sandbox->tilemap, "PlayerStart");
    if (player_start != 0) {
        sandbox->player = (Vector2) { player_start->rect.x, player_start->rect.y };
    }
    sandbox->coin = Sandbox_LoadSfx("audio/sfx/coin.r2sfx", R2D_SfxCoin());
    sandbox->hit = Sandbox_LoadSfx("audio/sfx/hit.r2sfx", R2D_SfxHit());
    sandbox->jump = Sandbox_LoadSfx("audio/sfx/jump.r2sfx", R2D_SfxJump());
    sandbox->laser = Sandbox_LoadSfx("audio/sfx/laser.r2sfx", R2D_SfxLaser());
    sandbox->explosion = Sandbox_LoadSfx("audio/sfx/explosion.r2sfx", R2D_SfxExplosion());
    sandbox->powerup = Sandbox_LoadSfx("audio/sfx/powerup.r2sfx", R2D_SfxPowerup());
    sandbox->reload_ok = false;

    snprintf(song_path, sizeof(song_path), "%s", R2D_AssetPath("audio/music/touhou-bad-apple.r2song"));

    sandbox->music_loaded = R2D_AssetExists(song_path) &&
        R2D_MusicLoadSong(&sandbox->music, song_path);

    if (!sandbox->music_loaded) {
        snprintf(midi_path, sizeof(midi_path), "%s", R2D_AssetPath("audio/music/touhou-bad-apple.mid"));
        snprintf(soundfont_path, sizeof(soundfont_path), "%s", R2D_AssetPath("audio/soundfonts/8-Bit_Sounds.sf2"));

        sandbox->music_loaded = R2D_AssetExists(midi_path) &&
            R2D_AssetExists(soundfont_path) &&
            R2D_MusicLoad(&sandbox->music, midi_path, soundfont_path);
    }

    if (sandbox->music_loaded) {
        R2D_MusicPlay(&sandbox->music, true);
    }
}

static void Sandbox_Update(float dt, void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;
    const float speed = 80.0f;
    const Vector2 previous = sandbox->player;
    Vector2 movement = { 0.0f, 0.0f };
    Vector2 next;

    R2D_MusicUpdate(&sandbox->music);

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        movement.x -= speed * dt;
        sandbox->facing_left = true;
    }

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        movement.x += speed * dt;
        sandbox->facing_left = false;
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        movement.y -= speed * dt;
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        movement.y += speed * dt;
    }

    next = (Vector2) { sandbox->player.x + movement.x, sandbox->player.y };
    if (!Sandbox_PlayerCollides(sandbox, next)) {
        sandbox->player.x = next.x;
    }

    next = (Vector2) { sandbox->player.x, sandbox->player.y + movement.y };
    if (!Sandbox_PlayerCollides(sandbox, next)) {
        sandbox->player.y = next.y;
    }

    if (IsKeyPressed(KEY_C) && sandbox->crt != 0) {
        R2D_CrtSetEnabled(sandbox->crt, !sandbox->crt->enabled);
    }

    if (IsKeyPressed(KEY_R) && sandbox->crt != 0) {
        sandbox->reload_ok = R2D_CrtReload(sandbox->crt);
        sandbox->reload_message_timer = 1.25f;
    }

    if (IsKeyPressed(KEY_F3)) {
        sandbox->debug_draw = !sandbox->debug_draw;
    }

    if (sandbox->reload_message_timer > 0.0f) {
        sandbox->reload_message_timer -= dt;
    }

    if (IsKeyPressed(KEY_Z)) {
        R2D_PlaySfx(sandbox->coin);
    }

    if (IsKeyPressed(KEY_X)) {
        R2D_PlaySfx(sandbox->hit);
    }

    if (IsKeyPressed(KEY_V)) {
        R2D_PlaySfx(sandbox->jump);
    }

    if (IsKeyPressed(KEY_B)) {
        R2D_PlaySfx(sandbox->laser);
    }

    if (IsKeyPressed(KEY_N)) {
        R2D_PlaySfx(sandbox->explosion);
    }

    if (IsKeyPressed(KEY_M)) {
        R2D_PlaySfx(sandbox->powerup);
    }

    if (IsKeyPressed(KEY_P) && sandbox->music_loaded) {
        if (R2D_MusicIsPlaying(&sandbox->music)) {
            R2D_MusicStop(&sandbox->music);
        } else {
            R2D_MusicPlay(&sandbox->music, true);
        }
    }

    sandbox->moving = previous.x != sandbox->player.x || previous.y != sandbox->player.y;
    if (sandbox->moving && sandbox->player_anim.anim.frame_count != sandbox->walk_anim.frame_count) {
        R2D_AnimPlay(&sandbox->player_anim, sandbox->walk_anim);
    } else if (!sandbox->moving && sandbox->player_anim.anim.frame_count != sandbox->idle_anim.frame_count) {
        R2D_AnimPlay(&sandbox->player_anim, sandbox->idle_anim);
    }

    R2D_AnimUpdate(&sandbox->player_anim, dt);
    R2D_CameraFollow(&sandbox->camera, (Vector2) { sandbox->player.x + 8.0f, sandbox->player.y + 8.0f });
    if (R2D_TilemapIsReady(&sandbox->tilemap)) {
        R2D_CameraClampToRect(
            &sandbox->camera,
            R2D_Rect(
                0.0f,
                0.0f,
                (float)(sandbox->tilemap.width * sandbox->tilemap.tile_width),
                (float)(sandbox->tilemap.height * sandbox->tilemap.tile_height)
            )
        );
    }
    sandbox->blink_timer += dt;
}

static void Sandbox_DrawGrid(const R2D_Context *context, Vector2 camera_position)
{
    const Color grid = R2D_ColorFromHex(0x2a2a3aff);
    const int width = R2D_VirtualWidth(context);
    const int height = R2D_VirtualHeight(context);
    const int offset_x = -((int)camera_position.x % 16);
    const int offset_y = -((int)camera_position.y % 16);

    for (int x = offset_x; x <= width; x += 16) {
        DrawLine(x, 0, x, height, grid);
    }

    for (int y = offset_y; y <= height; y += 16) {
        DrawLine(0, y, width, y, grid);
    }
}

static void Sandbox_Draw(void *user_data)
{
    const Sandbox *sandbox = (const Sandbox *)user_data;
    const bool blink = ((int)(sandbox->blink_timer * 4.0f) % 2) == 0;
    const Vector2 camera_pixel = R2D_CameraPixelPosition(&sandbox->camera);
    const Vector2 camera_offset = {
        -camera_pixel.x,
        -camera_pixel.y
    };
    const Rectangle camera_view = R2D_Rect(
        camera_pixel.x,
        camera_pixel.y,
        (float)sandbox->camera.viewport_width,
        (float)sandbox->camera.viewport_height
    );
    const Vector2 player_screen = {
        floorf(sandbox->player.x - camera_pixel.x),
        floorf(sandbox->player.y - camera_pixel.y)
    };

    if (R2D_TilemapIsReady(&sandbox->tilemap)) {
        R2D_TilemapDrawVisible(&sandbox->tilemap, camera_view, camera_offset);
    }

    //Sandbox_DrawGrid(sandbox->context, camera_pixel);
    DrawText("Retro2DFramework", 8, 8, 10, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("WASD / Arrows", 8, 22, 8, R2D_ColorFromHex(0x8be9fdff));
    if (sandbox->crt != 0) {
        DrawText(sandbox->crt->enabled ? "C: CRT ON" : "C: CRT OFF", 8, 34, 8, R2D_ColorFromHex(0x50fa7bff));
        DrawText("R: RELOAD CRT", 8, 46, 8, R2D_ColorFromHex(0xffb86cff));
        DrawText(sandbox->debug_draw ? "F3: DEBUG ON" : "F3: DEBUG OFF", 8, 58, 8, R2D_ColorFromHex(0x50fa7bff));
        DrawText("Z/X/V/B/N/M: SFX", 8, 70, 8, R2D_ColorFromHex(0xffb86cff));
        DrawText(sandbox->music_loaded ? "P: MUSIC" : "MUSIC: ADD theme.mid + chiptune.sf2", 8, 82, 8, R2D_ColorFromHex(0xffb86cff));
    }

    if (sandbox->reload_message_timer > 0.0f) {
        DrawText(
            sandbox->reload_ok ? "SHADER RELOADED" : "SHADER ERROR",
            8,
            94,
            8,
            sandbox->reload_ok ? R2D_ColorFromHex(0x50fa7bff) : R2D_ColorFromHex(0xff5555ff)
        );
    }

    if (sandbox->debug_draw) {
        R2D_TilemapDrawCollisionDebugVisible(
            &sandbox->tilemap,
            sandbox->collision_layer,
            camera_view,
            camera_offset,
            R2D_ColorFromHex(0xff5555cc)
        );
        R2D_TilemapDrawObjectsDebug(
            &sandbox->tilemap,
            camera_offset,
            R2D_ColorFromHex(0x8be9fdcc)
        );
        DrawRectangleLinesEx(
            R2D_Rect(0.0f, 0.0f, (float)R2D_VirtualWidth(sandbox->context), (float)R2D_VirtualHeight(sandbox->context)),
            1.0f,
            R2D_ColorFromHex(0xf1fa8cff)
        );
    }

    R2D_DrawAnim(&sandbox->player_sheet, &sandbox->player_anim, player_screen, sandbox->facing_left);
    DrawRectangleLinesEx(R2D_Rect(player_screen.x, player_screen.y, 16.0f, 16.0f), 1.0f, R2D_ColorFromHex(0xf1fa8cff));

    if (blink) {
        DrawPixel((int)player_screen.x + 11, (int)player_screen.y + 5, BLACK);
    }
}

static void Sandbox_Shutdown(void *user_data)
{
    Sandbox *sandbox = (Sandbox *)user_data;

    R2D_MusicUnload(&sandbox->music);
    R2D_TilemapUnload(&sandbox->tilemap);
    R2D_UnloadSpriteSheet(&sandbox->player_sheet);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Crt crt = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    Sandbox sandbox = { 0 };

    config.title = "Retro2DFramework Sandbox";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_AudioInit();
    R2D_CrtInit(&crt);
    R2D_SetCrt(&context, &crt);
    sandbox.context = &context;
    sandbox.crt = &crt;

    R2D_Run(&context, (R2D_App) {
        Sandbox_Init,
        Sandbox_Update,
        Sandbox_Draw,
        Sandbox_Shutdown,
        &sandbox
    });

    R2D_CrtClose(&crt);
    R2D_AudioClose();
    R2D_Close(&context);
    return 0;
}
