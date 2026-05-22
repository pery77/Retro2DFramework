#include "r2d/r2d.h"

typedef struct PlatformerExample {
    R2D_InputMap input;
    Vector2 position;
    Vector2 velocity;
    bool grounded;
} PlatformerExample;

static const float PLATFORMER_FLOOR_Y = 162.0f;
static const Rectangle PLATFORMER_LEDGE = { 124.0f, 116.0f, 94.0f, 8.0f };
static const float PLATFORMER_PLAYER_WIDTH = 12.0f;
static const float PLATFORMER_PLAYER_HEIGHT = 18.0f;

static float Platformer_ClampFloat(float value, float min, float max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

static Rectangle Platformer_PlayerBounds(Vector2 position)
{
    return R2D_Rect(position.x, position.y, PLATFORMER_PLAYER_WIDTH, PLATFORMER_PLAYER_HEIGHT);
}

static void Platformer_Init(void *user_data)
{
    PlatformerExample *example = (PlatformerExample *)user_data;

    R2D_InputInit(&example->input);
    R2D_InputBindKey(&example->input, "left", KEY_LEFT);
    R2D_InputBindKey(&example->input, "left", KEY_A);
    R2D_InputBindKey(&example->input, "right", KEY_RIGHT);
    R2D_InputBindKey(&example->input, "right", KEY_D);
    R2D_InputBindKey(&example->input, "jump", KEY_SPACE);
    example->position = (Vector2) { 36.0f, PLATFORMER_FLOOR_Y - PLATFORMER_PLAYER_HEIGHT };
    example->grounded = true;
}

static void Platformer_Update(float dt, void *user_data)
{
    PlatformerExample *example = (PlatformerExample *)user_data;
    float axis;
    Rectangle bounds;

    R2D_InputUpdate(&example->input);
    axis = R2D_InputAxis(&example->input, "left", "right");

    example->velocity.x = axis * 80.0f;
    example->velocity.y += 220.0f * dt;

    if (example->grounded && R2D_InputPressed(&example->input, "jump")) {
        example->velocity.y = -150.0f;
        example->grounded = false;
    }

    example->position.x += example->velocity.x * dt;
    example->position.y += example->velocity.y * dt;
    example->position.x = Platformer_ClampFloat(example->position.x, 8.0f, 300.0f);

    bounds = Platformer_PlayerBounds(example->position);
    example->grounded = false;

    if (bounds.y + bounds.height >= PLATFORMER_FLOOR_Y) {
        example->position.y = PLATFORMER_FLOOR_Y - bounds.height;
        example->velocity.y = 0.0f;
        example->grounded = true;
    }

    bounds = Platformer_PlayerBounds(example->position);
    if (example->velocity.y >= 0.0f &&
        bounds.x + bounds.width > PLATFORMER_LEDGE.x &&
        bounds.x < PLATFORMER_LEDGE.x + PLATFORMER_LEDGE.width &&
        bounds.y + bounds.height >= PLATFORMER_LEDGE.y &&
        bounds.y + bounds.height <= PLATFORMER_LEDGE.y + PLATFORMER_LEDGE.height) {
        example->position.y = PLATFORMER_LEDGE.y - bounds.height;
        example->velocity.y = 0.0f;
        example->grounded = true;
    }
}

static void Platformer_Draw(void *user_data)
{
    const PlatformerExample *example = (const PlatformerExample *)user_data;
    Rectangle player = Platformer_PlayerBounds(example->position);

    DrawText("Platformer reference", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Horizontal control, gravity, jump, floor and one-way landing.", 6, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    DrawRectangle(0, (int)PLATFORMER_FLOOR_Y, 320, 38, R2D_ColorFromHex(0x3a506bff));
    DrawRectangleLines(0, (int)PLATFORMER_FLOOR_Y, 320, 1, R2D_ColorFromHex(0xf8f8f255));
    DrawRectangleRec(PLATFORMER_LEDGE, R2D_ColorFromHex(0x8ecae6ff));
    DrawRectangleRec(player, R2D_ColorFromHex(0x06d6a0ff));
    DrawRectangleLinesEx(player, 1.0f, R2D_ColorFromHex(0xf8f8f2ff));
    DrawRectangle((int)player.x + 3, (int)player.y + 4, 2, 2, R2D_ColorFromHex(0x15151fff));
    DrawRectangle((int)player.x + 8, (int)player.y + 4, 2, 2, R2D_ColorFromHex(0x15151fff));

    DrawText("A/D or arrows move   Space jump", 72, 188, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    PlatformerExample example = { 0 };

    config.title = "Retro2D Platformer Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_Run(&context, (R2D_App) {
        Platformer_Init,
        Platformer_Update,
        Platformer_Draw,
        0,
        &example
    });

    R2D_Close(&context);
    return 0;
}
