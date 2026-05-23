#include "r2d/r2d.h"

typedef enum @PROJECT_DEFINE@_State {
    @PROJECT_DEFINE@_STATE_TITLE = 0,
    @PROJECT_DEFINE@_STATE_PLAY,
    @PROJECT_DEFINE@_STATE_PAUSE
} @PROJECT_DEFINE@_State;

typedef struct @PROJECT_DEFINE@_Game {
    R2D_InputMap input;
    R2D_Crt *crt;
    @PROJECT_DEFINE@_State state;
    Vector2 player;
} @PROJECT_DEFINE@_Game;

static const Rectangle GAME_PLAY_AREA = { 8.0f, 46.0f, 304.0f, 136.0f };
static const float GAME_PLAYER_RADIUS = 8.0f;

static void Game_ResetPlayer(@PROJECT_DEFINE@_Game *game)
{
    game->player = (Vector2) {
        GAME_PLAY_AREA.x + GAME_PLAY_AREA.width * 0.5f,
        GAME_PLAY_AREA.y + GAME_PLAY_AREA.height * 0.5f
    };
}

static void Game_SetupInput(R2D_InputMap *input)
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

static void Game_Init(void *user_data)
{
    @PROJECT_DEFINE@_Game *game = (@PROJECT_DEFINE@_Game *)user_data;

    Game_SetupInput(&game->input);
    game->state = @PROJECT_DEFINE@_STATE_TITLE;
    Game_ResetPlayer(game);
    R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "@PROJECT_NAME@ initialized");
}

static void Game_Update(float dt, void *user_data)
{
    @PROJECT_DEFINE@_Game *game = (@PROJECT_DEFINE@_Game *)user_data;

    R2D_InputUpdate(&game->input);

    if (game->state == @PROJECT_DEFINE@_STATE_TITLE) {
        if (R2D_InputPressed(&game->input, "submit")) {
            Game_ResetPlayer(game);
            game->state = @PROJECT_DEFINE@_STATE_PLAY;
        }
        return;
    }

    if (R2D_InputPressed(&game->input, "pause")) {
        game->state = game->state == @PROJECT_DEFINE@_STATE_PLAY ?
            @PROJECT_DEFINE@_STATE_PAUSE :
            @PROJECT_DEFINE@_STATE_PLAY;
    }

    if (R2D_InputPressed(&game->input, "crt") && game->crt != 0) {
        R2D_CrtSetEnabled(game->crt, !game->crt->enabled);
    }

    if (game->state == @PROJECT_DEFINE@_STATE_PLAY) {
        game->player.x += R2D_InputAxis(&game->input, "left", "right") * 80.0f * dt;
        game->player.y += R2D_InputAxis(&game->input, "up", "down") * 80.0f * dt;
        game->player.x = R2D_Clamp(
            game->player.x,
            GAME_PLAY_AREA.x + GAME_PLAYER_RADIUS,
            GAME_PLAY_AREA.x + GAME_PLAY_AREA.width - GAME_PLAYER_RADIUS
        );
        game->player.y = R2D_Clamp(
            game->player.y,
            GAME_PLAY_AREA.y + GAME_PLAYER_RADIUS,
            GAME_PLAY_AREA.y + GAME_PLAY_AREA.height - GAME_PLAYER_RADIUS
        );
    }
}

static void Game_Draw(void *user_data)
{
    const @PROJECT_DEFINE@_Game *game = (const @PROJECT_DEFINE@_Game *)user_data;
    const bool in_game = game->state == @PROJECT_DEFINE@_STATE_PLAY || game->state == @PROJECT_DEFINE@_STATE_PAUSE;

    ClearBackground(R2D_ColorFromHex(0x15151fff));
    DrawText("@PROJECT_NAME@", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Enter start   WASD move   Esc pause   C CRT", 12, 30, 8, R2D_ColorFromHex(0xf8f8f2ff));

    DrawRectangleRec(GAME_PLAY_AREA, R2D_ColorFromHex(0x101820ff));
    DrawRectangleLinesEx(GAME_PLAY_AREA, 1.0f, R2D_ColorFromHex(0x3a506bff));

    if (in_game) {
        DrawCircleV(game->player, GAME_PLAYER_RADIUS + 2.0f, R2D_ColorFromHex(0xf8f8f2ff));
        DrawCircleV(game->player, GAME_PLAYER_RADIUS, R2D_ColorFromHex(0x06d6a0ff));
    } else {
        R2D_DrawTextAligned(
            "Press Enter",
            GAME_PLAY_AREA,
            R2D_DefaultTextStyle(14, R2D_ColorFromHex(0xf8f8f2ff)),
            R2D_TEXT_ALIGN_CENTER
        );
    }

    if (game->state == @PROJECT_DEFINE@_STATE_PAUSE) {
        DrawRectangle(112, 88, 96, 24, R2D_ColorFromHex(0x101820ee));
        DrawRectangleLines(112, 88, 96, 24, R2D_ColorFromHex(0xffd166ff));
        DrawText("PAUSE", 138, 96, 8, R2D_ColorFromHex(0xf8f8f2ff));
    }
}

static void Game_Shutdown(void *user_data)
{
    (void)user_data;
    R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "@PROJECT_NAME@ shutdown");
}

int main(int argc, char **argv)
{
    R2D_Context context = { 0 };
    R2D_RuntimeConfig runtime = R2D_RuntimeConfigDefault();
    @PROJECT_DEFINE@_Game game = { 0 };
    R2D_Crt crt = { 0 };

    R2D_RuntimeConfigLoad(&runtime, "r2d.ini");
    R2D_RuntimeConfigApplyArgs(&runtime, argc, argv);
    runtime.config.title = "@PROJECT_NAME@";
    runtime.config.clear_color = R2D_ColorFromHex(0x15151fff);
    R2D_SetDevelopmentAssetDir("@PROJECT_DIR@/assets");

    if (!R2D_Init(&context, runtime.config)) {
        return 1;
    }

    R2D_RuntimeConfigApplyAudio(&runtime);
    R2D_CrtInit(&crt);
    R2D_RuntimeConfigApplyCrt(&runtime, &crt);
    R2D_SetCrt(&context, &crt);
    game.crt = &crt;

    R2D_Run(&context, (R2D_App) {
        Game_Init,
        Game_Update,
        Game_Draw,
        Game_Shutdown,
        &game
    });

    R2D_CrtClose(&crt);
    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
