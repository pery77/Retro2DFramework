#include "r2d/r2d.h"

typedef struct TopDownExample {
    R2D_InputMap input;
    R2D_Camera camera;
    Vector2 player;
    Rectangle walls[5];
} TopDownExample;

static Rectangle TopDown_PlayerBounds(Vector2 position)
{
    return R2D_Rect(position.x - 6.0f, position.y - 6.0f, 12.0f, 12.0f);
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
    example->player = (Vector2) { 56.0f, 56.0f };
    example->walls[0] = R2D_Rect(0.0f, 0.0f, 320.0f, 16.0f);
    example->walls[1] = R2D_Rect(0.0f, 184.0f, 320.0f, 16.0f);
    example->walls[2] = R2D_Rect(0.0f, 0.0f, 16.0f, 200.0f);
    example->walls[3] = R2D_Rect(304.0f, 0.0f, 16.0f, 200.0f);
    example->walls[4] = R2D_Rect(128.0f, 72.0f, 64.0f, 48.0f);
}

static void TopDown_Update(float dt, void *user_data)
{
    TopDownExample *example = (TopDownExample *)user_data;
    R2D_Collider colliders[5];
    R2D_CollisionResult result = { 0 };
    Rectangle player_bounds;
    Vector2 movement;

    R2D_InputUpdate(&example->input);
    movement = (Vector2) {
        R2D_InputAxis(&example->input, "left", "right") * 80.0f * dt,
        R2D_InputAxis(&example->input, "up", "down") * 80.0f * dt
    };

    for (int i = 0; i < 5; ++i) {
        colliders[i] = R2D_ColliderRect(example->walls[i], 1u, 1u, false, 0);
    }

    player_bounds = TopDown_PlayerBounds(example->player);
    movement = R2D_MoveAndSlide(player_bounds, movement, 1u, 1u, colliders, 5, &result);
    example->player.x += movement.x;
    example->player.y += movement.y;

    R2D_CameraFollow(&example->camera, example->player);
}

static void TopDown_Draw(void *user_data)
{
    const TopDownExample *example = (const TopDownExample *)user_data;
    const Rectangle player = TopDown_PlayerBounds(example->player);

    DrawText("Top-down reference", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Input axes, camera follow and MoveAndSlide against room walls.", 12, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    for (int y = 16; y < 184; y += 16) {
        for (int x = 16; x < 304; x += 16) {
            DrawRectangleLines(x, y, 16, 16, R2D_ColorFromHex(0x243447ff));
        }
    }

    for (int i = 0; i < 5; ++i) {
        DrawRectangleRec(example->walls[i], R2D_ColorFromHex(0x3a506bff));
    }

    DrawRectangleRec(player, R2D_ColorFromHex(0x06d6a0ff));
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
