#include "r2d/r2d.h"

typedef enum TemplateState {
    TEMPLATE_STATE_TITLE = 0,
    TEMPLATE_STATE_PLAY,
    TEMPLATE_STATE_PAUSE
} TemplateState;

typedef struct TemplateGame {
    R2D_InputMap input;
    R2D_EntityWorld entities;
    TemplateState state;
    Vector2 player;
} TemplateGame;

static void Template_SetupInput(R2D_InputMap *input)
{
    R2D_InputInit(input);
    R2D_InputBindKey(input, "left", KEY_LEFT);
    R2D_InputBindKey(input, "left", KEY_A);
    R2D_InputBindKey(input, "right", KEY_RIGHT);
    R2D_InputBindKey(input, "right", KEY_D);
    R2D_InputBindKey(input, "up", KEY_UP);
    R2D_InputBindKey(input, "up", KEY_W);
    R2D_InputBindKey(input, "down", KEY_DOWN);
    R2D_InputBindKey(input, "down", KEY_S);
    R2D_InputBindKey(input, "submit", KEY_ENTER);
    R2D_InputBindKey(input, "pause", KEY_ESCAPE);
}

static void Template_Init(void *user_data)
{
    TemplateGame *game = (TemplateGame *)user_data;

    Template_SetupInput(&game->input);
    R2D_EntityWorldInit(&game->entities, game);
    game->state = TEMPLATE_STATE_TITLE;
    game->player = (Vector2) { 152.0f, 92.0f };
    R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "template game initialized");
}

static void Template_Update(float dt, void *user_data)
{
    TemplateGame *game = (TemplateGame *)user_data;

    R2D_InputUpdate(&game->input);

    if (game->state == TEMPLATE_STATE_TITLE) {
        if (R2D_InputPressed(&game->input, "submit")) {
            game->state = TEMPLATE_STATE_PLAY;
        }
        return;
    }

    if (R2D_InputPressed(&game->input, "pause")) {
        game->state = game->state == TEMPLATE_STATE_PLAY ? TEMPLATE_STATE_PAUSE : TEMPLATE_STATE_PLAY;
    }

    if (game->state == TEMPLATE_STATE_PLAY) {
        const float x = R2D_InputAxis(&game->input, "left", "right");
        const float y = R2D_InputAxis(&game->input, "up", "down");
        Vector2 movement = { x * 80.0f * dt, y * 80.0f * dt };

        game->player.x += movement.x;
        game->player.y += movement.y;
        game->player.x = Clamp(game->player.x, 8.0f, 304.0f);
        game->player.y = Clamp(game->player.y, 34.0f, 176.0f);
    }
}

static void Template_Draw(void *user_data)
{
    const TemplateGame *game = (const TemplateGame *)user_data;
    const char *state_text = game->state == TEMPLATE_STATE_TITLE ? "TITLE" : game->state == TEMPLATE_STATE_PLAY ? "PLAY" : "PAUSE";

    ClearBackground(R2D_ColorFromHex(0x15151fff));
    DrawText("Clean game template", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Input, state, update and draw kept in one small loop.", 12, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    DrawRectangle(8, 46, 304, 136, R2D_ColorFromHex(0x101820ff));
    DrawRectangleLines(8, 46, 304, 136, R2D_ColorFromHex(0x3a506bff));
    DrawCircleV(game->player, 8.0f, R2D_ColorFromHex(0x06d6a0ff));

    DrawText(state_text, 12, 188, 8, R2D_ColorFromHex(0x8ecae6ff));
    DrawText("Enter start   Arrows/WASD move   Esc pause", 78, 188, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

static void Template_Shutdown(void *user_data)
{
    TemplateGame *game = (TemplateGame *)user_data;

    R2D_EntityWorldClear(&game->entities);
    R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "template game shutdown");
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    TemplateGame game = { 0 };

    config.title = "Retro2D Template Game";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_Run(&context, (R2D_App) {
        Template_Init,
        Template_Update,
        Template_Draw,
        Template_Shutdown,
        &game
    });

    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
