#include "r2d/r2d.h"

#include <stdio.h>

#define COLLISION_LAYER_CURSOR 0x01u
#define COLLISION_LAYER_SOLID 0x02u
#define COLLISION_LAYER_TRIGGER 0x04u
#define COLLISION_LAYER_SENSOR 0x08u
#define COLLISION_COLLIDER_COUNT 7

typedef struct CollisionExample {
    R2D_Context *context;
    R2D_Collider colliders[COLLISION_COLLIDER_COUNT];
    Vector2 object_position;
    R2D_CollisionResult move_result;
    int trigger_hits;
    int point_hits;
    int circle_hits;
} CollisionExample;

static Rectangle CollisionExample_ObjectBounds(Vector2 position)
{
    return R2D_Rect(position.x, position.y, 14.0f, 14.0f);
}

static R2D_Collider CollisionExample_Collider(Rectangle rect, unsigned int layer, bool trigger)
{
    return R2D_ColliderRect(rect, layer, COLLISION_LAYER_CURSOR, trigger, 0);
}

static void CollisionExample_Init(void *user_data)
{
    CollisionExample *example = (CollisionExample *)user_data;

    example->object_position = (Vector2) { 24.0f, 24.0f };
    example->trigger_hits = 0;
    example->point_hits = 0;
    example->circle_hits = 0;

    example->colliders[0] = CollisionExample_Collider(R2D_Rect(70.0f, 42.0f, 40.0f, 86.0f), COLLISION_LAYER_SOLID, false);
    example->colliders[1] = CollisionExample_Collider(R2D_Rect(148.0f, 92.0f, 92.0f, 20.0f), COLLISION_LAYER_SOLID, false);
    example->colliders[2] = CollisionExample_Collider(R2D_Rect(260.0f, 34.0f, 22.0f, 128.0f), COLLISION_LAYER_SOLID, false);
    example->colliders[3] = CollisionExample_Collider(R2D_Rect(22.0f, 148.0f, 78.0f, 30.0f), COLLISION_LAYER_TRIGGER, true);
    example->colliders[4] = CollisionExample_Collider(R2D_Rect(132.0f, 136.0f, 54.0f, 38.0f), COLLISION_LAYER_TRIGGER, true);
    example->colliders[5] = CollisionExample_Collider(R2D_Rect(214.0f, 148.0f, 24.0f, 24.0f), COLLISION_LAYER_SENSOR, true);
    example->colliders[6] = CollisionExample_Collider(R2D_Rect(292.0f, 174.0f, 12.0f, 12.0f), COLLISION_LAYER_SENSOR, true);

    HideCursor();
}

static void CollisionExample_Update(float dt, void *user_data)
{
    CollisionExample *example = (CollisionExample *)user_data;
    const Vector2 mouse = R2D_MouseVirtualPosition(example->context);
    const Vector2 desired = {
        mouse.x - 7.0f,
        mouse.y - 7.0f
    };
    const Vector2 movement = {
        desired.x - example->object_position.x,
        desired.y - example->object_position.y
    };
    const Rectangle current_bounds = CollisionExample_ObjectBounds(example->object_position);
    R2D_CollisionHit hits[R2D_COLLISION_MAX_HITS];
    const Vector2 center = {
        example->object_position.x + 7.0f,
        example->object_position.y + 7.0f
    };

    (void)dt;

    example->object_position = R2D_MoveAndSlide(
        current_bounds,
        movement,
        COLLISION_LAYER_CURSOR,
        COLLISION_LAYER_SOLID | COLLISION_LAYER_TRIGGER | COLLISION_LAYER_SENSOR,
        example->colliders,
        COLLISION_COLLIDER_COUNT,
        &example->move_result
    );

    example->trigger_hits = R2D_CollisionQueryRect(
        CollisionExample_ObjectBounds(example->object_position),
        COLLISION_LAYER_CURSOR,
        COLLISION_LAYER_TRIGGER,
        example->colliders,
        COLLISION_COLLIDER_COUNT,
        hits,
        R2D_COLLISION_MAX_HITS
    );

    example->point_hits = R2D_CollisionQueryPoint(
        center,
        COLLISION_LAYER_CURSOR,
        COLLISION_LAYER_SENSOR,
        example->colliders,
        COLLISION_COLLIDER_COUNT,
        hits,
        R2D_COLLISION_MAX_HITS
    );

    example->circle_hits = R2D_CollisionQueryCircle(
        center,
        24.0f,
        COLLISION_LAYER_CURSOR,
        COLLISION_LAYER_SENSOR,
        example->colliders,
        COLLISION_COLLIDER_COUNT,
        hits,
        R2D_COLLISION_MAX_HITS
    );
}

static Color CollisionExample_ColliderColor(const CollisionExample *example, int index)
{
    const R2D_Collider *collider = &example->colliders[index];

    if ((collider->layer & COLLISION_LAYER_SOLID) != 0) {
        return R2D_ColorFromHex(0xef476fff);
    }

    if ((collider->layer & COLLISION_LAYER_TRIGGER) != 0) {
        return example->trigger_hits > 0 ? R2D_ColorFromHex(0x06d6a0ff) : R2D_ColorFromHex(0x118ab2ff);
    }

    return (example->point_hits > 0 || example->circle_hits > 0) ?
        R2D_ColorFromHex(0xffd166ff) :
        R2D_ColorFromHex(0x7b61ffff);
}

static void CollisionExample_DrawCollider(const CollisionExample *example, int index)
{
    const R2D_Collider *collider = &example->colliders[index];
    const Color color = CollisionExample_ColliderColor(example, index);
    Color fill = color;

    fill.a = collider->trigger ? 76 : 140;
    DrawRectangleRec(collider->rect, fill);
    DrawRectangleLinesEx(collider->rect, 1.0f, color);
}

static void CollisionExample_Draw(void *user_data)
{
    const CollisionExample *example = (const CollisionExample *)user_data;
    const Rectangle object = CollisionExample_ObjectBounds(example->object_position);
    const Vector2 center = {
        object.x + object.width * 0.5f,
        object.y + object.height * 0.5f
    };
    char text[96];

    DrawText("Collision example", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Move the hidden system mouse. The square slides, triggers and senses.", 12, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    for (int i = 0; i < COLLISION_COLLIDER_COUNT; ++i) {
        CollisionExample_DrawCollider(example, i);
    }

    DrawCircleLines((int)center.x, (int)center.y, 24.0f, example->circle_hits > 0 ? R2D_ColorFromHex(0xffd166ff) : R2D_ColorFromHex(0x4d5b6aff));
    DrawRectangleRec(object, R2D_ColorFromHex(0xf8f8f2ff));
    DrawRectangleLinesEx(object, 1.0f, R2D_ColorFromHex(0x101820ff));
    DrawLine((int)center.x - 4, (int)center.y, (int)center.x + 4, (int)center.y, R2D_ColorFromHex(0x101820ff));
    DrawLine((int)center.x, (int)center.y - 4, (int)center.x, (int)center.y + 4, R2D_ColorFromHex(0x101820ff));

    DrawRectangle(10, 184, 300, 12, R2D_ColorFromHex(0x101820cc));
    snprintf(
        text,
        sizeof(text),
        "solid x:%d y:%d  triggers:%d  point:%d  circle:%d",
        example->move_result.collided_x ? 1 : 0,
        example->move_result.collided_y ? 1 : 0,
        example->trigger_hits,
        example->point_hits,
        example->circle_hits
    );
    DrawText(text, 14, 187, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

static void CollisionExample_Shutdown(void *user_data)
{
    (void)user_data;
    ShowCursor();
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    CollisionExample example = { 0 };

    config.title = "Retro2D Collision Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    example.context = &context;
    R2D_Run(&context, (R2D_App) {
        CollisionExample_Init,
        CollisionExample_Update,
        CollisionExample_Draw,
        CollisionExample_Shutdown,
        &example
    });

    R2D_Close(&context);
    return 0;
}
