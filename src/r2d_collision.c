#include "r2d/r2d.h"

#include <string.h>

R2D_Collider R2D_ColliderRect(Rectangle rect, unsigned int layer, unsigned int mask, bool trigger, void *user_data)
{
    return (R2D_Collider) {
        rect,
        layer,
        mask,
        trigger,
        user_data
    };
}

bool R2D_AabbIntersects(Rectangle a, Rectangle b)
{
    return a.width > 0.0f &&
        a.height > 0.0f &&
        b.width > 0.0f &&
        b.height > 0.0f &&
        CheckCollisionRecs(a, b);
}

bool R2D_CollisionLayersMatch(unsigned int layer, unsigned int mask, unsigned int other_layer, unsigned int other_mask)
{
    return (layer & other_mask) != 0 && (other_layer & mask) != 0;
}

static bool R2D_CollisionAppendHit(
    R2D_CollisionHit *hits,
    int max_hits,
    int *hit_count,
    int collider_index,
    const R2D_Collider *collider,
    Vector2 normal,
    float penetration
)
{
    if (hit_count == 0 || *hit_count >= max_hits || hits == 0 || collider == 0) {
        return false;
    }

    hits[*hit_count] = (R2D_CollisionHit) {
        collider_index,
        collider->rect,
        normal,
        penetration,
        collider->trigger,
        collider->user_data
    };
    ++(*hit_count);
    return true;
}

int R2D_CollisionQueryRect(
    Rectangle rect,
    unsigned int layer,
    unsigned int mask,
    const R2D_Collider *colliders,
    int collider_count,
    R2D_CollisionHit *hits,
    int max_hits
)
{
    int hit_count = 0;

    if (colliders == 0 || collider_count <= 0 || max_hits <= 0) {
        return 0;
    }

    for (int i = 0; i < collider_count; ++i) {
        const R2D_Collider *collider = &colliders[i];

        if (!R2D_CollisionLayersMatch(layer, mask, collider->layer, collider->mask) ||
            !R2D_AabbIntersects(rect, collider->rect)) {
            continue;
        }

        R2D_CollisionAppendHit(
            hits,
            max_hits,
            &hit_count,
            i,
            collider,
            (Vector2) { 0.0f, 0.0f },
            0.0f
        );
    }

    return hit_count;
}

int R2D_CollisionQueryPoint(
    Vector2 point,
    unsigned int layer,
    unsigned int mask,
    const R2D_Collider *colliders,
    int collider_count,
    R2D_CollisionHit *hits,
    int max_hits
)
{
    int hit_count = 0;

    if (colliders == 0 || collider_count <= 0 || max_hits <= 0) {
        return 0;
    }

    for (int i = 0; i < collider_count; ++i) {
        const R2D_Collider *collider = &colliders[i];

        if (!R2D_CollisionLayersMatch(layer, mask, collider->layer, collider->mask) ||
            !CheckCollisionPointRec(point, collider->rect)) {
            continue;
        }

        R2D_CollisionAppendHit(
            hits,
            max_hits,
            &hit_count,
            i,
            collider,
            (Vector2) { 0.0f, 0.0f },
            0.0f
        );
    }

    return hit_count;
}

int R2D_CollisionQueryCircle(
    Vector2 center,
    float radius,
    unsigned int layer,
    unsigned int mask,
    const R2D_Collider *colliders,
    int collider_count,
    R2D_CollisionHit *hits,
    int max_hits
)
{
    int hit_count = 0;

    if (colliders == 0 || collider_count <= 0 || max_hits <= 0 || radius <= 0.0f) {
        return 0;
    }

    for (int i = 0; i < collider_count; ++i) {
        const R2D_Collider *collider = &colliders[i];

        if (!R2D_CollisionLayersMatch(layer, mask, collider->layer, collider->mask) ||
            !CheckCollisionCircleRec(center, radius, collider->rect)) {
            continue;
        }

        R2D_CollisionAppendHit(
            hits,
            max_hits,
            &hit_count,
            i,
            collider,
            (Vector2) { 0.0f, 0.0f },
            0.0f
        );
    }

    return hit_count;
}

static void R2D_MoveAndSlideRecordTriggers(
    Rectangle bounds,
    unsigned int layer,
    unsigned int mask,
    const R2D_Collider *colliders,
    int collider_count,
    R2D_CollisionResult *result
)
{
    if (result == 0 || colliders == 0) {
        return;
    }

    for (int i = 0; i < collider_count; ++i) {
        const R2D_Collider *collider = &colliders[i];

        if (!collider->trigger ||
            !R2D_CollisionLayersMatch(layer, mask, collider->layer, collider->mask) ||
            !R2D_AabbIntersects(bounds, collider->rect)) {
            continue;
        }

        R2D_CollisionAppendHit(
            result->hits,
            R2D_COLLISION_MAX_HITS,
            &result->hit_count,
            i,
            collider,
            (Vector2) { 0.0f, 0.0f },
            0.0f
        );
    }
}

static bool R2D_RangesOverlap(float min_a, float max_a, float min_b, float max_b)
{
    return min_a < max_b && max_a > min_b;
}

Vector2 R2D_MoveAndSlide(
    Rectangle bounds,
    Vector2 movement,
    unsigned int layer,
    unsigned int mask,
    const R2D_Collider *colliders,
    int collider_count,
    R2D_CollisionResult *result
)
{
    Vector2 position = { bounds.x, bounds.y };
    Vector2 applied = { 0.0f, 0.0f };
    const Rectangle start_bounds = bounds;

    if (result != 0) {
        memset(result, 0, sizeof(*result));
        result->position = position;
    }

    if (colliders == 0 || collider_count <= 0 || bounds.width <= 0.0f || bounds.height <= 0.0f) {
        position.x += movement.x;
        position.y += movement.y;
        if (result != 0) {
            result->position = position;
            result->movement = movement;
        }
        return position;
    }

    applied.x = movement.x;

    for (int i = 0; i < collider_count; ++i) {
        const R2D_Collider *collider = &colliders[i];
        const bool overlaps_y = R2D_RangesOverlap(
            bounds.y,
            bounds.y + bounds.height,
            collider->rect.y,
            collider->rect.y + collider->rect.height
        );

        if (collider->trigger ||
            !R2D_CollisionLayersMatch(layer, mask, collider->layer, collider->mask) ||
            !overlaps_y) {
            continue;
        }

        if (movement.x > 0.0f &&
            start_bounds.x + start_bounds.width <= collider->rect.x &&
            bounds.x + movement.x + bounds.width > collider->rect.x) {
            const float candidate_x = collider->rect.x - bounds.width;
            const float penetration = bounds.x + movement.x - candidate_x;
            movement.x = candidate_x - bounds.x;
            applied.x = movement.x;
            if (result != 0) {
                result->collided_x = true;
                R2D_CollisionAppendHit(result->hits, R2D_COLLISION_MAX_HITS, &result->hit_count, i, collider, (Vector2) { -1.0f, 0.0f }, penetration);
            }
        } else if (movement.x < 0.0f &&
            start_bounds.x >= collider->rect.x + collider->rect.width &&
            bounds.x + movement.x < collider->rect.x + collider->rect.width) {
            const float candidate_x = collider->rect.x + collider->rect.width;
            const float penetration = candidate_x - (bounds.x + movement.x);
            movement.x = candidate_x - bounds.x;
            applied.x = movement.x;
            if (result != 0) {
                result->collided_x = true;
                R2D_CollisionAppendHit(result->hits, R2D_COLLISION_MAX_HITS, &result->hit_count, i, collider, (Vector2) { 1.0f, 0.0f }, penetration);
            }
        }
    }

    bounds.x += movement.x;
    bounds.y += movement.y;
    applied.y = movement.y;

    for (int i = 0; i < collider_count; ++i) {
        const R2D_Collider *collider = &colliders[i];
        const bool overlaps_x = R2D_RangesOverlap(
            bounds.x,
            bounds.x + bounds.width,
            collider->rect.x,
            collider->rect.x + collider->rect.width
        );

        if (collider->trigger ||
            !R2D_CollisionLayersMatch(layer, mask, collider->layer, collider->mask) ||
            !overlaps_x) {
            continue;
        }

        if (movement.y > 0.0f &&
            start_bounds.y + start_bounds.height <= collider->rect.y &&
            bounds.y > collider->rect.y - bounds.height) {
            const float candidate_y = collider->rect.y - bounds.height;
            const float penetration = bounds.y - candidate_y;
            bounds.y = candidate_y;
            applied.y -= penetration;
            if (result != 0) {
                result->collided_y = true;
                R2D_CollisionAppendHit(result->hits, R2D_COLLISION_MAX_HITS, &result->hit_count, i, collider, (Vector2) { 0.0f, -1.0f }, penetration);
            }
        } else if (movement.y < 0.0f &&
            start_bounds.y >= collider->rect.y + collider->rect.height &&
            bounds.y < collider->rect.y + collider->rect.height) {
            const float candidate_y = collider->rect.y + collider->rect.height;
            const float penetration = candidate_y - bounds.y;
            bounds.y = candidate_y;
            applied.y += penetration;
            if (result != 0) {
                result->collided_y = true;
                R2D_CollisionAppendHit(result->hits, R2D_COLLISION_MAX_HITS, &result->hit_count, i, collider, (Vector2) { 0.0f, 1.0f }, penetration);
            }
        }
    }

    position = (Vector2) { bounds.x, bounds.y };
    R2D_MoveAndSlideRecordTriggers(bounds, layer, mask, colliders, collider_count, result);

    if (result != 0) {
        result->position = position;
        result->movement = applied;
    }

    return position;
}
