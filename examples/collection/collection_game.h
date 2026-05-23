#ifndef COLLECTION_GAME_H
#define COLLECTION_GAME_H

#include "r2d/r2d.h"

typedef struct CollectionHost {
    const R2D_InputMap *input;
    bool return_to_launcher;
} CollectionHost;

typedef struct CollectionGameDef {
    const char *id;
    const char *title;
    const char *description;
    size_t state_size;
    void (*init)(void *state, CollectionHost *host);
    void (*update)(void *state, CollectionHost *host, float dt);
    void (*draw)(void *state, const CollectionHost *host);
    void (*shutdown)(void *state, CollectionHost *host);
} CollectionGameDef;

#endif
