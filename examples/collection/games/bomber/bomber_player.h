#ifndef COLLECTION_BOMBER_PLAYER_H
#define COLLECTION_BOMBER_PLAYER_H

#include "bomber_map.h"
#include "../../collection_game.h"

typedef struct BomberPlayer {
    Vector2 position;
} BomberPlayer;

void BomberPlayerInit(BomberPlayer *player, Vector2 origin);
void BomberPlayerUpdate(BomberPlayer *player, const BomberMap *map, const CollectionHost *host, Vector2 origin, float dt);
void BomberPlayerDraw(const BomberPlayer *player);

#endif
