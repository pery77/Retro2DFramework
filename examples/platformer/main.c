#include "r2d/r2d.h"

typedef struct PlatformerExample {
    R2D_InputMap input;
    Vector2 position;
    Vector2 velocity;
    bool grounded;
} PlatformerExample;

static Rectangle Platformer_PlayerBounds(Vector2 position)
{
    return R2D_Rect(position.x, position.y, 12.0f, 18.0f);
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
    example->position = (Vector2) { 36.0f, 120.0f };
}

static void Platformer_Update(float dt, void *user_data)
{
    PlatformerExample *example = (PlatformerExample *)user_data;
    const float axis = R2D_InputAxis(&example->input, "left", "right");
    const float floor_y = 162.0f;
    const float platform_y = 116.0f;
    Rectangle bounds;

    R2D_InputUpdate(&example->input);

    example->velocity.x = axis * 80.0f;
    example->velocity.y += 360.0f * dt;

    if (example->grounded && R2D_InputPressed(&example->input, "jump")) {
        example->velocity.y = -150.0f;
        example->grounded = false;
    }

    example->position.x += example->velocity.x * dt;
    example->position.y += example->velocity.y * dt;
    example->position.x = Clamp(example->position.x, 8.0f, 300.0f);

    bounds = Platformer_PlayerBounds(example->position);
    example->grounded = false;

    if (bounds.y + bounds.height >= floor_y) {
        example->position.y = floor_y - bounds.height;
        example->velocity.y = 0.0f;
        example->grounded = true;
    }

    bounds = Platformer_PlayerBounds(example->position);
    if (example->velocity.y >= 0.0f &&
        bounds.x + bounds.width > 124.0f &&
        bounds.x < 218.0f &&
        bounds.y + bounds.height >= platform_y &&
        bounds.y + bounds.height <= platform_y + 8.0f) {
        example->position.y = platform_y - bounds.height;
        example->velocity.y = 0.0f;
        example->grounded = true;
    }
}

static void Platformer_Draw(void *user_data)
{
    const PlatformerExample *example = (const PlatformerExample *)user_data;
    Rectangle player = Platformer_PlayerBounds(example->position);

    DrawText("Platformer reference", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Horizontal control, gravity, jump, floor and one-way landing.", 12, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    DrawRectangle(0, 162, 320, 38, R2D_ColorFromHex(0x3a506bff));
    DrawRectangle(124, 116, 94, 8, R2D_ColorFromHex(0x8ecae6ff));
    DrawRectangleRec(player, R2D_ColorFromHex(0x06d6a0ff));
    DrawRectangleLinesEx(player, 1.0f, R2D_ColorFromHex(0xf8f8f2ff));

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
