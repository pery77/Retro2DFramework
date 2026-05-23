#include "collection_game.h"
#include "launcher.h"
#include "games/bomber/bomber_game.h"
#include "games/pong/pong_game.h"

#include <stdlib.h>
#include <string.h>

typedef enum CollectionMode {
    COLLECTION_MODE_LAUNCHER = 0,
    COLLECTION_MODE_GAME
} CollectionMode;

typedef struct CollectionExample {
    R2D_InputMap input;
    CollectionHost host;
    CollectionLauncher launcher;
    CollectionGameDef games[8];
    int game_count;
    CollectionMode mode;
    const CollectionGameDef *active_game;
    void *active_state;
} CollectionExample;

static void CollectionSetupInput(R2D_InputMap *input)
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
    R2D_InputBindKey(input, "submit", KEY_Z);
    R2D_InputBindKey(input, "action", KEY_Z);
    R2D_InputBindKey(input, "back", KEY_ESCAPE);
}

static void CollectionRegisterGame(CollectionExample *example, CollectionGameDef game)
{
    if (example->game_count >= (int)(sizeof(example->games) / sizeof(example->games[0]))) {
        return;
    }

    example->games[example->game_count] = game;
    ++example->game_count;
}

static void CollectionStopGame(CollectionExample *example)
{
    if (example->active_game != 0 && example->active_state != 0 && example->active_game->shutdown != 0) {
        example->active_game->shutdown(example->active_state, &example->host);
    }

    free(example->active_state);
    example->active_state = 0;
    example->active_game = 0;
    example->host.return_to_launcher = false;
    example->mode = COLLECTION_MODE_LAUNCHER;
}

static void CollectionStartGame(CollectionExample *example, int index)
{
    const CollectionGameDef *game;

    if (index < 0 || index >= example->game_count) {
        return;
    }

    CollectionStopGame(example);

    game = &example->games[index];
    example->active_state = calloc(1u, game->state_size);
    if (example->active_state == 0) {
        return;
    }

    example->active_game = game;
    example->mode = COLLECTION_MODE_GAME;
    if (game->init != 0) {
        game->init(example->active_state, &example->host);
    }
}

static void CollectionExampleInit(void *user_data)
{
    CollectionExample *example = (CollectionExample *)user_data;

    CollectionSetupInput(&example->input);
    example->host.input = &example->input;
    example->mode = COLLECTION_MODE_LAUNCHER;
    CollectionLauncherInit(&example->launcher);
    CollectionRegisterGame(example, Pong_GetGameDef());
    CollectionRegisterGame(example, Bomber_GetGameDef());
}

static void CollectionExampleUpdate(float dt, void *user_data)
{
    CollectionExample *example = (CollectionExample *)user_data;

    R2D_InputUpdate(&example->input);

    if (example->mode == COLLECTION_MODE_LAUNCHER) {
        CollectionLauncherUpdate(&example->launcher, &example->input, example->game_count, dt);
        if (R2D_InputPressed(&example->input, "submit")) {
            CollectionStartGame(example, example->launcher.selected);
        }
        return;
    }

    if (example->active_game != 0 && example->active_game->update != 0) {
        example->active_game->update(example->active_state, &example->host, dt);
    }

    if (example->host.return_to_launcher) {
        CollectionStopGame(example);
    }
}

static void CollectionExampleDraw(void *user_data)
{
    CollectionExample *example = (CollectionExample *)user_data;

    if (example->mode == COLLECTION_MODE_GAME && example->active_game != 0 && example->active_game->draw != 0) {
        example->active_game->draw(example->active_state, &example->host);
        return;
    }

    CollectionLauncherDraw(&example->launcher, example->games, example->game_count);
}

static void CollectionExampleShutdown(void *user_data)
{
    CollectionExample *example = (CollectionExample *)user_data;

    CollectionStopGame(example);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    CollectionExample example = { 0 };

    config.title = "Retro2D Collection Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_Run(&context, (R2D_App) {
        CollectionExampleInit,
        CollectionExampleUpdate,
        CollectionExampleDraw,
        CollectionExampleShutdown,
        &example
    });

    R2D_Close(&context);
    R2D_LogCloseFile();
    return 0;
}
