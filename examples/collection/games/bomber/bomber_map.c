#include "bomber_map.h"

void BomberMapInit(BomberMap *map)
{
    for (int y = 0; y < BOMBER_MAP_HEIGHT; ++y) {
        for (int x = 0; x < BOMBER_MAP_WIDTH; ++x) {
            if (x == 0 || y == 0 || x == BOMBER_MAP_WIDTH - 1 || y == BOMBER_MAP_HEIGHT - 1 || (x % 2 == 0 && y % 2 == 0)) {
                map->tiles[y][x] = BOMBER_TILE_WALL;
            } else if ((x + y) % 4 == 0 && !(x < 3 && y < 3)) {
                map->tiles[y][x] = BOMBER_TILE_CRATE;
            } else {
                map->tiles[y][x] = BOMBER_TILE_FLOOR;
            }
        }
    }
}

Rectangle BomberMapTileRect(Vector2 origin, int tile_x, int tile_y)
{
    return R2D_Rect(
        origin.x + (float)(tile_x * BOMBER_TILE_SIZE),
        origin.y + (float)(tile_y * BOMBER_TILE_SIZE),
        (float)BOMBER_TILE_SIZE,
        (float)BOMBER_TILE_SIZE
    );
}

bool BomberMapCanEnter(const BomberMap *map, int tile_x, int tile_y)
{
    if (tile_x < 0 || tile_y < 0 || tile_x >= BOMBER_MAP_WIDTH || tile_y >= BOMBER_MAP_HEIGHT) {
        return false;
    }

    return map->tiles[tile_y][tile_x] == BOMBER_TILE_FLOOR;
}

void BomberMapDraw(const BomberMap *map, Vector2 origin)
{
    for (int y = 0; y < BOMBER_MAP_HEIGHT; ++y) {
        for (int x = 0; x < BOMBER_MAP_WIDTH; ++x) {
            const Rectangle tile = BomberMapTileRect(origin, x, y);
            Color color = R2D_ColorFromHex(0x263238ff);

            if (map->tiles[y][x] == BOMBER_TILE_WALL) {
                color = R2D_ColorFromHex(0x8ecae6ff);
            } else if (map->tiles[y][x] == BOMBER_TILE_CRATE) {
                color = R2D_ColorFromHex(0xffb703ff);
            }

            DrawRectangleRec(tile, color);
            DrawRectangleLinesEx(tile, 1.0f, R2D_ColorFromHex(0x101820ff));
        }
    }
}
