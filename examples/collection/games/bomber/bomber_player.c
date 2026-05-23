#include "bomber_player.h"

static bool BomberPlayerCanMoveTo(const BomberMap *map, Vector2 origin, Vector2 position)
{
    const int tile_x = (int)((position.x - origin.x) / (float)BOMBER_TILE_SIZE);
    const int tile_y = (int)((position.y - origin.y) / (float)BOMBER_TILE_SIZE);

    return BomberMapCanEnter(map, tile_x, tile_y);
}

void BomberPlayerInit(BomberPlayer *player, Vector2 origin)
{
    player->position = (Vector2) {
        origin.x + BOMBER_TILE_SIZE + BOMBER_TILE_SIZE * 0.5f,
        origin.y + BOMBER_TILE_SIZE + BOMBER_TILE_SIZE * 0.5f
    };
}

void BomberPlayerUpdate(BomberPlayer *player, const BomberMap *map, const CollectionHost *host, Vector2 origin, float dt)
{
    const float speed = 76.0f;
    const Vector2 axis = {
        R2D_InputAxis(host->input, "left", "right"),
        R2D_InputAxis(host->input, "up", "down")
    };
    Vector2 next = player->position;

    next.x += axis.x * speed * dt;
    if (BomberPlayerCanMoveTo(map, origin, next)) {
        player->position.x = next.x;
    }

    next = player->position;
    next.y += axis.y * speed * dt;
    if (BomberPlayerCanMoveTo(map, origin, next)) {
        player->position.y = next.y;
    }
}

void BomberPlayerDraw(const BomberPlayer *player)
{
    DrawCircleV(player->position, 6.0f, R2D_ColorFromHex(0xf8f8f2ff));
    DrawCircleV(player->position, 4.0f, R2D_ColorFromHex(0xfb5607ff));
}
