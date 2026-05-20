#include "r2d/r2d.h"

#include <string.h>

static R2D_EntityId R2D_EntityMakeId(int index, unsigned int generation)
{
    return ((generation & 0xffffu) << 16) | (unsigned int)(index + 1);
}

static int R2D_EntityIndex(R2D_EntityId id)
{
    const int index = (int)(id & 0xffffu) - 1;

    return index >= 0 && index < R2D_ENTITY_MAX ? index : -1;
}

static unsigned int R2D_EntityGeneration(R2D_EntityId id)
{
    return (id >> 16) & 0xffffu;
}

void R2D_EntityWorldInit(R2D_EntityWorld *world, void *user_data)
{
    if (world == 0) {
        return;
    }

    memset(world, 0, sizeof(*world));
    world->user_data = user_data;

    for (int i = 0; i < R2D_ENTITY_MAX; ++i) {
        world->generations[i] = 1u;
    }
}

void R2D_EntityWorldClear(R2D_EntityWorld *world)
{
    void *user_data;

    if (world == 0) {
        return;
    }

    user_data = world->user_data;
    R2D_EntityWorldInit(world, user_data);
}

R2D_Entity *R2D_EntitySpawn(R2D_EntityWorld *world, int type, Vector2 position)
{
    R2D_Entity *entity;

    if (world == 0 || world->count >= R2D_ENTITY_MAX) {
        return 0;
    }

    for (int i = 0; i < R2D_ENTITY_MAX; ++i) {
        if (world->entities[i].active) {
            continue;
        }

        entity = &world->entities[i];
        memset(entity, 0, sizeof(*entity));
        entity->id = R2D_EntityMakeId(i, world->generations[i]);
        entity->position = position;
        entity->bounds = (Rectangle) { position.x, position.y, 0.0f, 0.0f };
        entity->type = type;
        entity->layer = 1u;
        entity->active = true;
        world->count++;
        return entity;
    }

    return 0;
}

bool R2D_EntityDestroy(R2D_EntityWorld *world, R2D_EntityId id)
{
    const int index = R2D_EntityIndex(id);
    R2D_Entity *entity;

    if (world == 0 || index < 0 || !R2D_EntityAlive(world, id)) {
        return false;
    }

    entity = &world->entities[index];
    memset(entity, 0, sizeof(*entity));
    world->generations[index] = (world->generations[index] + 1u) & 0xffffu;
    if (world->generations[index] == 0u) {
        world->generations[index] = 1u;
    }
    world->count--;
    return true;
}

bool R2D_EntityAlive(const R2D_EntityWorld *world, R2D_EntityId id)
{
    const int index = R2D_EntityIndex(id);

    if (world == 0 || index < 0 || id == 0) {
        return false;
    }

    return world->entities[index].active &&
        world->entities[index].id == id &&
        world->generations[index] == R2D_EntityGeneration(id);
}

R2D_Entity *R2D_EntityGet(R2D_EntityWorld *world, R2D_EntityId id)
{
    const int index = R2D_EntityIndex(id);

    if (world == 0 || index < 0 || !R2D_EntityAlive(world, id)) {
        return 0;
    }

    return &world->entities[index];
}

const R2D_Entity *R2D_EntityGetConst(const R2D_EntityWorld *world, R2D_EntityId id)
{
    const int index = R2D_EntityIndex(id);

    if (world == 0 || index < 0 || !R2D_EntityAlive(world, id)) {
        return 0;
    }

    return &world->entities[index];
}

int R2D_EntityCount(const R2D_EntityWorld *world)
{
    return world != 0 ? world->count : 0;
}

R2D_Entity *R2D_EntityAt(R2D_EntityWorld *world, int active_index)
{
    int seen = 0;

    if (world == 0 || active_index < 0) {
        return 0;
    }

    for (int i = 0; i < R2D_ENTITY_MAX; ++i) {
        if (!world->entities[i].active) {
            continue;
        }

        if (seen == active_index) {
            return &world->entities[i];
        }

        seen++;
    }

    return 0;
}

const R2D_Entity *R2D_EntityAtConst(const R2D_EntityWorld *world, int active_index)
{
    int seen = 0;

    if (world == 0 || active_index < 0) {
        return 0;
    }

    for (int i = 0; i < R2D_ENTITY_MAX; ++i) {
        if (!world->entities[i].active) {
            continue;
        }

        if (seen == active_index) {
            return &world->entities[i];
        }

        seen++;
    }

    return 0;
}

R2D_Entity *R2D_EntityFindByType(R2D_EntityWorld *world, int type, int *cursor)
{
    int start;

    if (world == 0 || cursor == 0) {
        return 0;
    }

    start = *cursor;
    if (start < 0) {
        start = 0;
    }

    for (int i = start; i < R2D_ENTITY_MAX; ++i) {
        if (world->entities[i].active && world->entities[i].type == type) {
            *cursor = i + 1;
            return &world->entities[i];
        }
    }

    return 0;
}

R2D_Entity *R2D_EntityFindByLayer(R2D_EntityWorld *world, unsigned int layer_mask, int *cursor)
{
    int start;

    if (world == 0 || cursor == 0) {
        return 0;
    }

    start = *cursor;
    if (start < 0) {
        start = 0;
    }

    for (int i = start; i < R2D_ENTITY_MAX; ++i) {
        if (world->entities[i].active && (world->entities[i].layer & layer_mask) != 0) {
            *cursor = i + 1;
            return &world->entities[i];
        }
    }

    return 0;
}

void R2D_EntityWorldUpdate(R2D_EntityWorld *world, float dt)
{
    if (world == 0) {
        return;
    }

    for (int i = 0; i < R2D_ENTITY_MAX; ++i) {
        R2D_Entity *entity = &world->entities[i];

        if (!entity->active) {
            continue;
        }

        if (entity->update != 0) {
            entity->update(entity, dt, world->user_data);
        } else {
            entity->position.x += entity->velocity.x * dt;
            entity->position.y += entity->velocity.y * dt;
        }

        entity->bounds.x = entity->position.x;
        entity->bounds.y = entity->position.y;
    }
}

void R2D_EntityWorldDraw(const R2D_EntityWorld *world)
{
    if (world == 0) {
        return;
    }

    for (int i = 0; i < R2D_ENTITY_MAX; ++i) {
        const R2D_Entity *entity = &world->entities[i];

        if (entity->active && entity->draw != 0) {
            entity->draw(entity, world->user_data);
        }
    }
}
