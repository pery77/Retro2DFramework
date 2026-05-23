#include "bomber_game.h"
#include "bomber_map.h"
#include "bomber_player.h"

typedef struct BomberGame {
    BomberMap map;
    BomberPlayer player;
    Vector2 origin;
    float bomb_flash;
} BomberGame;

static void Bomber_Init(void *state, CollectionHost *host)
{
    (void)host;

    BomberGame *game = (BomberGame *)state;
    game->origin = (Vector2) { 40.0f, 42.0f };
    game->bomb_flash = 0.0f;
    BomberMapInit(&game->map);
    BomberPlayerInit(&game->player, game->origin);
}

static void Bomber_Update(void *state, CollectionHost *host, float dt)
{
    BomberGame *game = (BomberGame *)state;

    if (R2D_InputPressed(host->input, "back")) {
        host->return_to_launcher = true;
        return;
    }

    if (R2D_InputPressed(host->input, "action")) {
        game->bomb_flash = 0.25f;
    }

    if (game->bomb_flash > 0.0f) {
        game->bomb_flash -= dt;
    }

    BomberPlayerUpdate(&game->player, &game->map, host, game->origin, dt);
}

static void Bomber_Draw(void *state, const CollectionHost *host)
{
    (void)host;

    const BomberGame *game = (const BomberGame *)state;
    const int bomb_x = ((int)((game->player.position.x - game->origin.x) / BOMBER_TILE_SIZE) * BOMBER_TILE_SIZE) + (int)game->origin.x + 8;
    const int bomb_y = ((int)((game->player.position.y - game->origin.y) / BOMBER_TILE_SIZE) * BOMBER_TILE_SIZE) + (int)game->origin.y + 8;

    ClearBackground(R2D_ColorFromHex(0x15151fff));
    DrawText("Bomber Lite", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Split across game/map/player files. Esc returns.", 12, 28, 8, R2D_ColorFromHex(0xf8f8f2ff));

    BomberMapDraw(&game->map, game->origin);

    if (game->bomb_flash > 0.0f) {
        DrawCircle(bomb_x, bomb_y, 7.0f, R2D_ColorFromHex(0xff006eff));
        DrawCircle(bomb_x, bomb_y, 4.0f, R2D_ColorFromHex(0x101820ff));
    }

    BomberPlayerDraw(&game->player);
    DrawText("WASD move   Z bomb", 92, 190, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

static void Bomber_Shutdown(void *state, CollectionHost *host)
{
    (void)state;
    (void)host;
}

CollectionGameDef Bomber_GetGameDef(void)
{
    return (CollectionGameDef) {
        "bomber",
        "Bomber Lite",
        "A larger game split into files.",
        sizeof(BomberGame),
        Bomber_Init,
        Bomber_Update,
        Bomber_Draw,
        Bomber_Shutdown
    };
}
