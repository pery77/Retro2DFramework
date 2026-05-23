#ifndef COLLECTION_BOMBER_MAP_H
#define COLLECTION_BOMBER_MAP_H

#include "r2d/r2d.h"

#define BOMBER_MAP_WIDTH 15
#define BOMBER_MAP_HEIGHT 9
#define BOMBER_TILE_SIZE 16

typedef enum BomberTile {
    BOMBER_TILE_FLOOR = 0,
    BOMBER_TILE_WALL,
    BOMBER_TILE_CRATE
} BomberTile;

typedef struct BomberMap {
    BomberTile tiles[BOMBER_MAP_HEIGHT][BOMBER_MAP_WIDTH];
} BomberMap;

void BomberMapInit(BomberMap *map);
void BomberMapDraw(const BomberMap *map, Vector2 origin);
bool BomberMapCanEnter(const BomberMap *map, int tile_x, int tile_y);
Rectangle BomberMapTileRect(Vector2 origin, int tile_x, int tile_y);

#endif
