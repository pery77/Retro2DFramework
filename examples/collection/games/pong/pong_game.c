#include "pong_game.h"

typedef struct PongGame {
    float left_y;
    float right_y;
    Vector2 ball;
    Vector2 velocity;
    int left_score;
    int right_score;
} PongGame;

static void Pong_ResetBall(PongGame *game, float direction)
{
    game->ball = (Vector2) { 160.0f, 100.0f };
    game->velocity = (Vector2) { 92.0f * direction, 54.0f };
}

static void Pong_Init(void *state, CollectionHost *host)
{
    (void)host;

    PongGame *game = (PongGame *)state;
    game->left_y = 82.0f;
    game->right_y = 82.0f;
    game->left_score = 0;
    game->right_score = 0;
    Pong_ResetBall(game, 1.0f);
}

static void Pong_Update(void *state, CollectionHost *host, float dt)
{
    PongGame *game = (PongGame *)state;
    const float player_speed = 96.0f;
    const Rectangle left_paddle = { 24.0f, game->left_y, 6.0f, 36.0f };
    const Rectangle right_paddle = { 290.0f, game->right_y, 6.0f, 36.0f };
    const Rectangle ball_rect = { game->ball.x - 3.0f, game->ball.y - 3.0f, 6.0f, 6.0f };

    if (R2D_InputPressed(host->input, "back")) {
        host->return_to_launcher = true;
        return;
    }

    game->left_y += R2D_InputAxis(host->input, "up", "down") * player_speed * dt;
    game->left_y = R2D_Clamp(game->left_y, 42.0f, 154.0f);

    if (game->ball.y < game->right_y + 18.0f) {
        game->right_y -= player_speed * 0.75f * dt;
    } else {
        game->right_y += player_speed * 0.75f * dt;
    }
    game->right_y = R2D_Clamp(game->right_y, 42.0f, 154.0f);

    game->ball.x += game->velocity.x * dt;
    game->ball.y += game->velocity.y * dt;

    if (game->ball.y < 40.0f || game->ball.y > 184.0f) {
        game->velocity.y = -game->velocity.y;
    }

    if (CheckCollisionRecs(ball_rect, left_paddle) && game->velocity.x < 0.0f) {
        game->velocity.x = -game->velocity.x * 1.04f;
    }

    if (CheckCollisionRecs(ball_rect, right_paddle) && game->velocity.x > 0.0f) {
        game->velocity.x = -game->velocity.x * 1.04f;
    }

    if (game->ball.x < 8.0f) {
        ++game->right_score;
        Pong_ResetBall(game, 1.0f);
    } else if (game->ball.x > 312.0f) {
        ++game->left_score;
        Pong_ResetBall(game, -1.0f);
    }
}

static void Pong_Draw(void *state, const CollectionHost *host)
{
    (void)host;

    const PongGame *game = (const PongGame *)state;
    char score[32];

    ClearBackground(R2D_ColorFromHex(0x101820ff));
    DrawText("Pong", 12, 10, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Esc returns to launcher", 218, 14, 8, R2D_ColorFromHex(0xf8f8f2ff));

    DrawLine(160, 38, 160, 188, R2D_ColorFromHex(0x3a506bff));
    DrawRectangle(16, 36, 288, 2, R2D_ColorFromHex(0x3a506bff));
    DrawRectangle(16, 188, 288, 2, R2D_ColorFromHex(0x3a506bff));
    DrawRectangle(24, (int)game->left_y, 6, 36, R2D_ColorFromHex(0xf8f8f2ff));
    DrawRectangle(290, (int)game->right_y, 6, 36, R2D_ColorFromHex(0xf8f8f2ff));
    DrawCircleV(game->ball, 4.0f, R2D_ColorFromHex(0xffd166ff));

    snprintf(score, sizeof(score), "%d   %d", game->left_score, game->right_score);
    R2D_DrawTextAligned(score, R2D_Rect(0.0f, 18.0f, 320.0f, 20.0f), R2D_DefaultTextStyle(12, R2D_ColorFromHex(0x8ecae6ff)), R2D_TEXT_ALIGN_CENTER);
}

static void Pong_Shutdown(void *state, CollectionHost *host)
{
    (void)state;
    (void)host;
}

CollectionGameDef Pong_GetGameDef(void)
{
    return (CollectionGameDef) {
        "pong",
        "Pong",
        "A tiny single-file game.",
        sizeof(PongGame),
        Pong_Init,
        Pong_Update,
        Pong_Draw,
        Pong_Shutdown
    };
}
