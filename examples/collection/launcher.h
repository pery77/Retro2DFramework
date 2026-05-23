#ifndef COLLECTION_LAUNCHER_H
#define COLLECTION_LAUNCHER_H

#include "collection_game.h"

typedef struct CollectionLauncher {
    int selected;
    float blink;
} CollectionLauncher;

void CollectionLauncherInit(CollectionLauncher *launcher);
void CollectionLauncherUpdate(CollectionLauncher *launcher, const R2D_InputMap *input, int game_count, float dt);
void CollectionLauncherDraw(const CollectionLauncher *launcher, const CollectionGameDef *games, int game_count);

#endif
