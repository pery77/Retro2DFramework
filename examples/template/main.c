#include "r2d/r2d.h"

typedef enum TemplateState {
    TEMPLATE_STATE_TITLE = 0,
    TEMPLATE_STATE_PLAY,
    TEMPLATE_STATE_PAUSE
} TemplateState;

typedef struct TemplateGame {
    R2D_InputMap input;
    R2D_EntityWorld entities;
    R2D_Crt *crt;
    TemplateState state;
    Vector2 player;
} TemplateGame;

static const Rectangle TEMPLATE_PLAY_AREA = { 8.0f, 46.0f, 304.0f, 136.0f };
static const float TEMPLATE_PLAYER_RADIUS = 8.0f;

static float Template_ClampFloat(float value, float min, float max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

static void Template_ResetPlayer(TemplateGame *game)
{
    if (game == 0) {
        return;
    }

    game->player = (Vector2) {
        TEMPLATE_PLAY_AREA.x + TEMPLATE_PLAY_AREA.width * 0.5f,
        TEMPLATE_PLAY_AREA.y + TEMPLATE_PLAY_AREA.height * 0.5f
    };
}

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
    R2D_InputBindKey(input, "crt", KEY_C);
}

static void Template_Init(void *user_data)
{
    TemplateGame *game = (TemplateGame *)user_data;

    Template_SetupInput(&game->input);
    R2D_EntityWorldInit(&game->entities, game);
    game->state = TEMPLATE_STATE_TITLE;
    Template_ResetPlayer(game);
    R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "template game initialized");
}

static void Template_Update(float dt, void *user_data)
{
    TemplateGame *game = (TemplateGame *)user_data;

    R2D_InputUpdate(&game->input);

    if (game->state == TEMPLATE_STATE_TITLE) {
        if (R2D_InputPressed(&game->input, "submit")) {
            Template_ResetPlayer(game);
            game->state = TEMPLATE_STATE_PLAY;
        }
        return;
    }

    if (R2D_InputPressed(&game->input, "pause")) {
        game->state = game->state == TEMPLATE_STATE_PLAY ? TEMPLATE_STATE_PAUSE : TEMPLATE_STATE_PLAY;
    }

    if (R2D_InputPressed(&game->input, "crt") && game->crt != 0) {
        R2D_CrtSetEnabled(game->crt, !game->crt->enabled);
    }

    if (game->state == TEMPLATE_STATE_PLAY) {
        const float x = R2D_InputAxis(&game->input, "left", "right");
        const float y = R2D_InputAxis(&game->input, "up", "down");
        Vector2 movement = { x * 80.0f * dt, y * 80.0f * dt };
        game->player.x += movement.x;
        game->player.y += movement.y;
        game->player.x = Template_ClampFloat(
            game->player.x,
            TEMPLATE_PLAY_AREA.x + TEMPLATE_PLAYER_RADIUS,
            TEMPLATE_PLAY_AREA.x + TEMPLATE_PLAY_AREA.width - TEMPLATE_PLAYER_RADIUS
        );
        game->player.y = Template_ClampFloat(
            game->player.y,
            TEMPLATE_PLAY_AREA.y + TEMPLATE_PLAYER_RADIUS,
            TEMPLATE_PLAY_AREA.y + TEMPLATE_PLAY_AREA.height - TEMPLATE_PLAYER_RADIUS
        );
    }
}

static void Template_Draw(void *user_data)
{
    const TemplateGame *game = (const TemplateGame *)user_data;
    const char *state_text = game->state == TEMPLATE_STATE_TITLE ? "TITLE" : game->state == TEMPLATE_STATE_PLAY ? "PLAY" : "PAUSE";
    const bool in_game = game->state == TEMPLATE_STATE_PLAY || game->state == TEMPLATE_STATE_PAUSE;

    ClearBackground(R2D_ColorFromHex(0x15151fff));
    DrawText("Clean game template", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Input, state, update and draw kept in one small loop.", 12, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    DrawRectangleRec(TEMPLATE_PLAY_AREA, R2D_ColorFromHex(0x101820ff));
    DrawRectangleLinesEx(TEMPLATE_PLAY_AREA, 1.0f, R2D_ColorFromHex(0x3a506bff));

    if (in_game) {
        DrawCircleV(game->player, TEMPLATE_PLAYER_RADIUS + 2.0f, R2D_ColorFromHex(0xf8f8f2ff));
        DrawCircleV(game->player, TEMPLATE_PLAYER_RADIUS, R2D_ColorFromHex(0x06d6a0ff));
        DrawCircleV(game->player, 3.0f, R2D_ColorFromHex(0x15151fff));
    } else {
        R2D_DrawTextAligned(
            "Press Enter",
            TEMPLATE_PLAY_AREA,
            R2D_DefaultTextStyle(14, R2D_ColorFromHex(0xf8f8f2ff)),
            R2D_TEXT_ALIGN_CENTER
        );
    }

    DrawText(state_text, 12, 188, 8, R2D_ColorFromHex(0x8ecae6ff));
    DrawText("Enter start, Arrows/WASD move, Esc pause, C CRT", 48, 188, 8, R2D_ColorFromHex(0xf8f8f2ff));
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
    R2D_Crt crt = { 0 };

    config.title = "Retro2D Template Game";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_CrtInit(&crt);
    R2D_SetCrt(&context, &crt);
    game.crt = &crt;

    R2D_Run(&context, (R2D_App) {
        Template_Init,
        Template_Update,
        Template_Draw,
        Template_Shutdown,
        &game
    });

    R2D_CrtClose(&crt);
    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
