#include "r2d/r2d.h"

#include <math.h>

#define TOPDOWN_WALL_COUNT 7

typedef struct TopDownExample {
    R2D_InputMap input;
    R2D_Camera camera;
    Vector2 player;
    Rectangle walls[TOPDOWN_WALL_COUNT];
} TopDownExample;

static const Rectangle TOPDOWN_WORLD_BOUNDS = { 0.0f, 0.0f, 640.0f, 400.0f };

static Rectangle TopDown_PlayerBounds(Vector2 position)
{
    return R2D_Rect(position.x - 6.0f, position.y - 6.0f, 12.0f, 12.0f);
}

static Vector2 TopDown_WorldToScreen(const R2D_Camera *camera, Vector2 world)
{
    const Vector2 camera_position = R2D_CameraPixelPosition(camera);

    return (Vector2) {
        floorf(world.x - camera_position.x),
        floorf(world.y - camera_position.y)
    };
}

static void TopDown_DrawRectangleCamera(const R2D_Camera *camera, Rectangle rect, Color color)
{
    const Vector2 position = TopDown_WorldToScreen(camera, (Vector2) { rect.x, rect.y });

    rect.x = position.x;
    rect.y = position.y;
    DrawRectangleRec(rect, color);
}

static void TopDown_DrawRectangleLinesCamera(const R2D_Camera *camera, Rectangle rect, Color color)
{
    const Vector2 position = TopDown_WorldToScreen(camera, (Vector2) { rect.x, rect.y });

    rect.x = position.x;
    rect.y = position.y;
    DrawRectangleLinesEx(rect, 1.0f, color);
}

static void TopDown_Init(void *user_data)
{
    TopDownExample *example = (TopDownExample *)user_data;

    R2D_InputInit(&example->input);
    R2D_InputBindKey(&example->input, "left", KEY_LEFT);
    R2D_InputBindKey(&example->input, "left", KEY_A);
    R2D_InputBindKey(&example->input, "right", KEY_RIGHT);
    R2D_InputBindKey(&example->input, "right", KEY_D);
    R2D_InputBindKey(&example->input, "up", KEY_UP);
    R2D_InputBindKey(&example->input, "up", KEY_W);
    R2D_InputBindKey(&example->input, "down", KEY_DOWN);
    R2D_InputBindKey(&example->input, "down", KEY_S);

    example->camera = R2D_CameraCreate(R2D_DEFAULT_VIRTUAL_WIDTH, R2D_DEFAULT_VIRTUAL_HEIGHT);
    example->player = (Vector2) { 64.0f, 64.0f };
    example->walls[0] = R2D_Rect(0.0f, 0.0f, 640.0f, 16.0f);
    example->walls[1] = R2D_Rect(0.0f, 384.0f, 640.0f, 16.0f);
    example->walls[2] = R2D_Rect(0.0f, 0.0f, 16.0f, 400.0f);
    example->walls[3] = R2D_Rect(624.0f, 0.0f, 16.0f, 400.0f);
    example->walls[4] = R2D_Rect(128.0f, 72.0f, 64.0f, 128.0f);
    example->walls[5] = R2D_Rect(304.0f, 176.0f, 128.0f, 32.0f);
    example->walls[6] = R2D_Rect(488.0f, 72.0f, 40.0f, 176.0f);
    R2D_CameraFollow(&example->camera, example->player);
    R2D_CameraClampToRect(&example->camera, TOPDOWN_WORLD_BOUNDS);
}

static void TopDown_Update(float dt, void *user_data)
{
    TopDownExample *example = (TopDownExample *)user_data;
    R2D_Collider colliders[TOPDOWN_WALL_COUNT];
    R2D_CollisionResult result = { 0 };
    Rectangle player_bounds;
    Vector2 moved_bounds;
    Vector2 movement;

    R2D_InputUpdate(&example->input);
    movement = (Vector2) {
        R2D_InputAxis(&example->input, "left", "right") * 80.0f * dt,
        R2D_InputAxis(&example->input, "up", "down") * 80.0f * dt
    };

    for (int i = 0; i < TOPDOWN_WALL_COUNT; ++i) {
        colliders[i] = R2D_ColliderRect(example->walls[i], 1u, 1u, false, 0);
    }

    player_bounds = TopDown_PlayerBounds(example->player);
    moved_bounds = R2D_MoveAndSlide(player_bounds, movement, 1u, 1u, colliders, TOPDOWN_WALL_COUNT, &result);
    example->player = (Vector2) {
        moved_bounds.x + player_bounds.width * 0.5f,
        moved_bounds.y + player_bounds.height * 0.5f
    };

    R2D_CameraFollow(&example->camera, example->player);
    R2D_CameraClampToRect(&example->camera, TOPDOWN_WORLD_BOUNDS);
}

static void TopDown_Draw(void *user_data)
{
    const TopDownExample *example = (const TopDownExample *)user_data;
    const Rectangle player = TopDown_PlayerBounds(example->player);

    for (int y = 16; y < 384; y += 16) {
        for (int x = 16; x < 624; x += 16) {
            const Vector2 screen = TopDown_WorldToScreen(&example->camera, (Vector2) { (float)x, (float)y });
            const int screen_x = (int)screen.x;
            const int screen_y = (int)screen.y;

            if (screen_x > -16 && screen_x < 320 && screen_y > -16 && screen_y < 200) {
                DrawRectangleLines(screen_x, screen_y, 16, 16, R2D_ColorFromHex(0x243447ff));
            }
        }
    }

    for (int i = 0; i < TOPDOWN_WALL_COUNT; ++i) {
        TopDown_DrawRectangleCamera(&example->camera, example->walls[i], R2D_ColorFromHex(0x3a506bff));
    }

    TopDown_DrawRectangleCamera(&example->camera, player, R2D_ColorFromHex(0x06d6a0ff));
    TopDown_DrawRectangleLinesCamera(&example->camera, player, R2D_ColorFromHex(0xf8f8f2ff));

    DrawText("Top-down reference", 2, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Input axes, camera follow and MoveAndSlide against room walls.", 2, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("WASD or arrows move", 102, 188, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    TopDownExample example = { 0 };

    config.title = "Retro2D Top-Down Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_Run(&context, (R2D_App) {
        TopDown_Init,
        TopDown_Update,
        TopDown_Draw,
        0,
        &example
    });

    R2D_Close(&context);
    return 0;
}
