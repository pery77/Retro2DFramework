#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>

#define RELIC_RUN_WALL_COUNT 8
#define RELIC_RUN_RELIC_COUNT 4
#define RELIC_RUN_ENEMY_COUNT 3

typedef enum RelicRunState {
    RELIC_RUN_TITLE = 0,
    RELIC_RUN_PLAY,
    RELIC_RUN_WIN,
    RELIC_RUN_LOSE
} RelicRunState;

typedef enum RelicRunEntityType {
    RELIC_RUN_ENTITY_ENEMY = 1
} RelicRunEntityType;

typedef struct RelicRunEnemyPatrol {
    Vector2 from;
    Vector2 to;
    float speed;
} RelicRunEnemyPatrol;

typedef struct RelicRun {
    R2D_InputMap input;
    R2D_Camera camera;
    R2D_EntityWorld entities;
    R2D_Crt *crt;
    RelicRunState state;
    Vector2 player;
    Rectangle walls[RELIC_RUN_WALL_COUNT];
    Vector2 relics[RELIC_RUN_RELIC_COUNT];
    bool relic_collected[RELIC_RUN_RELIC_COUNT];
    RelicRunEnemyPatrol enemy_patrols[RELIC_RUN_ENEMY_COUNT];
    int relic_count;
    bool debug_draw;
} RelicRun;

static const Rectangle RELIC_RUN_WORLD = { 0.0f, 0.0f, 640.0f, 400.0f };
static const Rectangle RELIC_RUN_EXIT = { 576.0f, 320.0f, 32.0f, 32.0f };

static Rectangle RelicRun_PlayerBounds(Vector2 position)
{
    return R2D_Rect(position.x - 6.0f, position.y - 6.0f, 12.0f, 12.0f);
}

static Rectangle RelicRun_RelicBounds(Vector2 position)
{
    return R2D_Rect(position.x - 5.0f, position.y - 5.0f, 10.0f, 10.0f);
}

static Rectangle RelicRun_EnemyBounds(Vector2 position)
{
    return R2D_Rect(position.x, position.y, 14.0f, 14.0f);
}

static Vector2 RelicRun_Normalize(Vector2 value)
{
    const float length = sqrtf(value.x * value.x + value.y * value.y);

    if (length <= 0.0f) {
        return (Vector2) { 1.0f, 0.0f };
    }

    return (Vector2) { value.x / length, value.y / length };
}

static void RelicRun_UpdateEnemy(R2D_Entity *entity, float dt, void *world_data)
{
    RelicRunEnemyPatrol *patrol = (RelicRunEnemyPatrol *)entity->user_data;
    Vector2 target;
    Vector2 to_target;
    float distance;
    const float step = patrol != 0 ? patrol->speed * dt : 0.0f;

    (void)world_data;

    if (patrol == 0 || step <= 0.0f) {
        return;
    }

    target = entity->velocity.x * (patrol->to.x - patrol->from.x) +
        entity->velocity.y * (patrol->to.y - patrol->from.y) >= 0.0f ? patrol->to : patrol->from;
    to_target = (Vector2) {
        target.x - entity->position.x,
        target.y - entity->position.y
    };
    distance = sqrtf(to_target.x * to_target.x + to_target.y * to_target.y);

    if (distance <= step) {
        entity->position = target;
        entity->velocity.x *= -1.0f;
        entity->velocity.y *= -1.0f;
    } else {
        entity->position.x += entity->velocity.x * step;
        entity->position.y += entity->velocity.y * step;
    }
}

static void RelicRun_SetupInput(R2D_InputMap *input)
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
    R2D_InputBindKey(input, "debug", KEY_F3);
    R2D_InputBindKey(input, "crt", KEY_C);
}

static void RelicRun_SpawnEnemies(RelicRun *game)
{
    static const RelicRunEnemyPatrol patrols[RELIC_RUN_ENEMY_COUNT] = {
        { { 224.0f, 80.0f }, { 384.0f, 80.0f }, 48.0f },
        { { 520.0f, 96.0f }, { 520.0f, 304.0f }, 42.0f },
        { { 112.0f, 248.0f }, { 272.0f, 248.0f }, 54.0f }
    };

    R2D_EntityWorldClear(&game->entities);

    for (int i = 0; i < RELIC_RUN_ENEMY_COUNT; ++i) {
        R2D_Entity *enemy;

        game->enemy_patrols[i] = patrols[i];
        enemy = R2D_EntitySpawn(&game->entities, RELIC_RUN_ENTITY_ENEMY, game->enemy_patrols[i].from);
        if (enemy == 0) {
            continue;
        }

        enemy->bounds = RelicRun_EnemyBounds(enemy->position);
        enemy->velocity = RelicRun_Normalize((Vector2) {
            game->enemy_patrols[i].to.x - game->enemy_patrols[i].from.x,
            game->enemy_patrols[i].to.y - game->enemy_patrols[i].from.y
        });
        enemy->user_data = &game->enemy_patrols[i];
        enemy->update = RelicRun_UpdateEnemy;
    }
}

static void RelicRun_ResetRun(RelicRun *game)
{
    game->player = (Vector2) { 48.0f, 48.0f };
    game->relics[0] = (Vector2) { 112.0f, 96.0f };
    game->relics[1] = (Vector2) { 352.0f, 80.0f };
    game->relics[2] = (Vector2) { 240.0f, 280.0f };
    game->relics[3] = (Vector2) { 512.0f, 240.0f };

    for (int i = 0; i < RELIC_RUN_RELIC_COUNT; ++i) {
        game->relic_collected[i] = false;
    }

    game->relic_count = 0;
    RelicRun_SpawnEnemies(game);
    R2D_CameraFollow(&game->camera, game->player);
    R2D_CameraClampToRect(&game->camera, RELIC_RUN_WORLD);
}

static void RelicRun_Reset(RelicRun *game)
{
    game->state = RELIC_RUN_TITLE;
    RelicRun_ResetRun(game);
}

static void RelicRun_Init(void *user_data)
{
    RelicRun *game = (RelicRun *)user_data;

    RelicRun_SetupInput(&game->input);
    game->camera = R2D_CameraCreate(R2D_DEFAULT_VIRTUAL_WIDTH, R2D_DEFAULT_VIRTUAL_HEIGHT);
    R2D_EntityWorldInit(&game->entities, game);
    game->walls[0] = R2D_Rect(0.0f, 0.0f, 640.0f, 16.0f);
    game->walls[1] = R2D_Rect(0.0f, 384.0f, 640.0f, 16.0f);
    game->walls[2] = R2D_Rect(0.0f, 0.0f, 16.0f, 400.0f);
    game->walls[3] = R2D_Rect(624.0f, 0.0f, 16.0f, 400.0f);
    game->walls[4] = R2D_Rect(160.0f, 64.0f, 32.0f, 176.0f);
    game->walls[5] = R2D_Rect(288.0f, 160.0f, 176.0f, 24.0f);
    game->walls[6] = R2D_Rect(448.0f, 56.0f, 32.0f, 176.0f);
    game->walls[7] = R2D_Rect(96.0f, 304.0f, 336.0f, 24.0f);
    RelicRun_Reset(game);
    R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "relic run initialized");
}

static void RelicRun_StartPlay(RelicRun *game)
{
    game->state = RELIC_RUN_PLAY;
    RelicRun_ResetRun(game);
}

static Vector2 RelicRun_InputMovement(const R2D_InputMap *input, float speed, float dt)
{
    Vector2 movement = {
        R2D_InputAxis(input, "left", "right"),
        R2D_InputAxis(input, "up", "down")
    };
    const float length = sqrtf(movement.x * movement.x + movement.y * movement.y);

    if (length > 1.0f) {
        movement.x /= length;
        movement.y /= length;
    }

    movement.x *= speed * dt;
    movement.y *= speed * dt;
    return movement;
}

static void RelicRun_UpdatePlay(RelicRun *game, float dt)
{
    R2D_Collider colliders[RELIC_RUN_WALL_COUNT];
    Rectangle player_bounds = RelicRun_PlayerBounds(game->player);
    Vector2 moved_bounds;
    const Vector2 movement = RelicRun_InputMovement(&game->input, 86.0f, dt);

    for (int i = 0; i < RELIC_RUN_WALL_COUNT; ++i) {
        colliders[i] = R2D_ColliderRect(game->walls[i], 1u, 1u, false, 0);
    }

    moved_bounds = R2D_MoveAndSlide(player_bounds, movement, 1u, 1u, colliders, RELIC_RUN_WALL_COUNT, 0);
    game->player = (Vector2) {
        moved_bounds.x + player_bounds.width * 0.5f,
        moved_bounds.y + player_bounds.height * 0.5f
    };

    player_bounds = RelicRun_PlayerBounds(game->player);
    R2D_EntityWorldUpdate(&game->entities, dt);

    for (int i = 0; i < R2D_EntityCount(&game->entities); ++i) {
        const R2D_Entity *enemy = R2D_EntityAtConst(&game->entities, i);

        if (enemy != 0 && enemy->type == RELIC_RUN_ENTITY_ENEMY && CheckCollisionRecs(player_bounds, enemy->bounds)) {
            game->state = RELIC_RUN_LOSE;
            R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "player caught by enemy");
            return;
        }
    }

    for (int i = 0; i < RELIC_RUN_RELIC_COUNT; ++i) {
        if (!game->relic_collected[i] && CheckCollisionRecs(player_bounds, RelicRun_RelicBounds(game->relics[i]))) {
            game->relic_collected[i] = true;
            game->relic_count++;
            R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "relic collected %d/%d", game->relic_count, RELIC_RUN_RELIC_COUNT);
        }
    }

    if (game->relic_count == RELIC_RUN_RELIC_COUNT && CheckCollisionRecs(player_bounds, RELIC_RUN_EXIT)) {
        game->state = RELIC_RUN_WIN;
        R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "relic run complete");
    }
}

static void RelicRun_Update(float dt, void *user_data)
{
    RelicRun *game = (RelicRun *)user_data;

    R2D_InputUpdate(&game->input);

    if (R2D_InputPressed(&game->input, "debug")) {
        game->debug_draw = !game->debug_draw;
    }

    if (R2D_InputPressed(&game->input, "crt") && game->crt != 0) {
        R2D_CrtSetEnabled(game->crt, !game->crt->enabled);
    }

    if (game->state == RELIC_RUN_TITLE) {
        if (R2D_InputPressed(&game->input, "submit")) {
            RelicRun_StartPlay(game);
        }
    } else if (game->state == RELIC_RUN_PLAY) {
        RelicRun_UpdatePlay(game, dt);
    } else if ((game->state == RELIC_RUN_WIN || game->state == RELIC_RUN_LOSE) && R2D_InputPressed(&game->input, "submit")) {
        RelicRun_Reset(game);
    }

    R2D_CameraFollow(&game->camera, game->player);
    R2D_CameraClampToRect(&game->camera, RELIC_RUN_WORLD);
}

static void RelicRun_DrawGrid(const RelicRun *game)
{
    for (int y = 16; y < 384; y += 16) {
        for (int x = 16; x < 624; x += 16) {
            const Vector2 screen = R2D_CameraWorldToPixelScreen(&game->camera, (Vector2) { (float)x, (float)y });
            const int screen_x = (int)screen.x;
            const int screen_y = (int)screen.y;

            if (screen_x > -16 && screen_x < R2D_DEFAULT_VIRTUAL_WIDTH &&
                screen_y > -16 && screen_y < R2D_DEFAULT_VIRTUAL_HEIGHT) {
                DrawRectangleLines(screen_x, screen_y, 16, 16, R2D_ColorFromHex(0x243447ff));
            }
        }
    }
}

static void RelicRun_DrawWorld(const RelicRun *game)
{
    const Rectangle player_bounds = RelicRun_PlayerBounds(game->player);
    const Color exit_color =
        game->relic_count == RELIC_RUN_RELIC_COUNT ? R2D_ColorFromHex(0x06d6a0ff) : R2D_ColorFromHex(0x6d597aff);

    RelicRun_DrawGrid(game);
    R2D_DrawRectangleCamera(&game->camera, RELIC_RUN_EXIT, exit_color);
    R2D_DrawRectangleLinesCamera(&game->camera, RELIC_RUN_EXIT, 1.0f, R2D_ColorFromHex(0xf8f8f2ff));

    for (int i = 0; i < RELIC_RUN_WALL_COUNT; ++i) {
        R2D_DrawRectangleCamera(&game->camera, game->walls[i], R2D_ColorFromHex(0x3a506bff));
    }

    for (int i = 0; i < RELIC_RUN_RELIC_COUNT; ++i) {
        if (!game->relic_collected[i]) {
            const Vector2 screen = R2D_CameraWorldToPixelScreen(&game->camera, game->relics[i]);
            DrawCircleV(screen, 6.0f, R2D_ColorFromHex(0xffd166ff));
            DrawCircleV(screen, 3.0f, R2D_ColorFromHex(0xf72585ff));
        }
    }

    for (int i = 0; i < R2D_EntityCount(&game->entities); ++i) {
        const R2D_Entity *enemy = R2D_EntityAtConst(&game->entities, i);

        if (enemy != 0 && enemy->type == RELIC_RUN_ENTITY_ENEMY) {
            R2D_DrawRectangleCamera(&game->camera, enemy->bounds, R2D_ColorFromHex(0xf72585ff));
            R2D_DrawRectangleLinesCamera(&game->camera, enemy->bounds, 1.0f, R2D_ColorFromHex(0xf8f8f2ff));
        }
    }

    R2D_DrawRectangleCamera(&game->camera, player_bounds, R2D_ColorFromHex(0x8ecae6ff));
    R2D_DrawRectangleLinesCamera(&game->camera, player_bounds, 1.0f, R2D_ColorFromHex(0xf8f8f2ff));

    if (game->debug_draw) {
        R2D_DebugInfo info = R2D_DebugInfoDefault();
        info.title = "RELIC RUN";
        info.line = "camera, input, collision, pickups, enemies";
        info.entity_count = R2D_EntityCount(&game->entities);
        info.tile_x = (int)(game->player.x / 16.0f);
        info.tile_y = (int)(game->player.y / 16.0f);
        R2D_DebugDrawOverlay(&info, 6, 38);
        DrawRectangleLines(0, 0, game->camera.viewport_width, game->camera.viewport_height, R2D_ColorFromHex(0xf8f8f255));
    }
}

static void RelicRun_DrawHud(const RelicRun *game)
{
    char text[64];

    snprintf(text, sizeof(text), "RELICS %d/%d", game->relic_count, RELIC_RUN_RELIC_COUNT);
    DrawRectangle(6, 6, 94, 24, R2D_ColorFromHex(0x101820dd));
    DrawRectangleLines(6, 6, 94, 24, R2D_ColorFromHex(0xffd166ff));
    DrawText(text, 12, 12, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("F3 debug   C CRT", 198, 12, 8, R2D_ColorFromHex(0x8ecae6ff));
}

static void RelicRun_Draw(void *user_data)
{
    const RelicRun *game = (const RelicRun *)user_data;

    ClearBackground(R2D_ColorFromHex(0x15151fff));
    RelicRun_DrawWorld(game);
    RelicRun_DrawHud(game);

    if (game->state == RELIC_RUN_TITLE) {
        DrawRectangle(48, 58, 224, 80, R2D_ColorFromHex(0x101820ee));
        DrawRectangleLines(48, 58, 224, 80, R2D_ColorFromHex(0xffd166ff));
        DrawText("RELIC RUN", 100, 72, 20, R2D_ColorFromHex(0xffd166ff));
        DrawText("Enter start", 116, 104, 10, R2D_ColorFromHex(0xf8f8f2ff));
        DrawText("WASD / arrows move", 94, 120, 8, R2D_ColorFromHex(0x8ecae6ff));
    } else if (game->state == RELIC_RUN_WIN) {
        DrawRectangle(54, 64, 212, 72, R2D_ColorFromHex(0x101820ee));
        DrawRectangleLines(54, 64, 212, 72, R2D_ColorFromHex(0x06d6a0ff));
        DrawText("ALL RELICS FOUND", 78, 82, 14, R2D_ColorFromHex(0x06d6a0ff));
        DrawText("Enter return to title", 86, 112, 8, R2D_ColorFromHex(0xf8f8f2ff));
    } else if (game->state == RELIC_RUN_LOSE) {
        DrawRectangle(54, 64, 212, 72, R2D_ColorFromHex(0x101820ee));
        DrawRectangleLines(54, 64, 212, 72, R2D_ColorFromHex(0xf72585ff));
        DrawText("CAUGHT", 120, 82, 16, R2D_ColorFromHex(0xf72585ff));
        DrawText("Enter return to title", 86, 112, 8, R2D_ColorFromHex(0xf8f8f2ff));
    }
}

static void RelicRun_Shutdown(void *user_data)
{
    RelicRun *game = (RelicRun *)user_data;

    R2D_EntityWorldClear(&game->entities);
    R2D_LogInfo(R2D_LOG_SUBSYSTEM_GAME, "relic run shutdown");
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    RelicRun game = { 0 };
    R2D_Crt crt = { 0 };

    config.title = "Retro2D Relic Run";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_CrtInit(&crt);
    R2D_SetCrt(&context, &crt);
    game.crt = &crt;

    R2D_Run(&context, (R2D_App) {
        RelicRun_Init,
        RelicRun_Update,
        RelicRun_Draw,
        RelicRun_Shutdown,
        &game
    });

    R2D_CrtClose(&crt);
    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
