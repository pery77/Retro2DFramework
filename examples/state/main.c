#include "r2d/r2d.h"

typedef struct StateExample {
    R2D_InputMap input;
    R2D_StateMachine states;
    R2D_Context *context;
    float time;
} StateExample;

static R2D_State StateExample_TitleState(void);
static R2D_State StateExample_GameState(void);
static R2D_State StateExample_PauseState(void);
static R2D_State StateExample_ExitState(void);

static void StateExample_InitInput(StateExample *example)
{
    R2D_InputInit(&example->input);
    R2D_InputBindKey(&example->input, "submit", KEY_Z);
    R2D_InputBindKey(&example->input, "submit", KEY_ENTER);
    R2D_InputBindGamepadButton(&example->input, "submit", GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

    R2D_InputBindKey(&example->input, "pause", KEY_ESCAPE);
    R2D_InputBindGamepadButton(&example->input, "pause", GAMEPAD_BUTTON_MIDDLE_LEFT);
}

static void StateExample_Init(void *user_data)
{
    StateExample *example = (StateExample *)user_data;

    StateExample_InitInput(example);
    R2D_StateMachineInit(&example->states, example);
    R2D_StateMachineSet(&example->states, StateExample_TitleState());
}

static void StateExample_TitleUpdate(float dt, void *state_data, void *user_data)
{
    (void)dt;
    (void)state_data;

    StateExample *example = (StateExample *)user_data;

    if (R2D_InputPressed(&example->input, "submit")) {
        R2D_StateMachineSet(&example->states, StateExample_GameState());
    }
}

static void StateExample_TitleDraw(void *state_data, void *user_data)
{
    (void)state_data;
    (void)user_data;

    R2D_TextStyle title = R2D_DefaultTextStyle(18, R2D_ColorFromHex(0xffd166ff));
    title.use_shadow = true;

    R2D_DrawTextAligned("Title State", R2D_Rect(0.0f, 54.0f, 320.0f, 24.0f), title, R2D_TEXT_ALIGN_CENTER);
    R2D_DrawTextAligned("Z / Enter changes to Game", R2D_Rect(0.0f, 96.0f, 320.0f, 16.0f), R2D_DefaultTextStyle(10, R2D_ColorFromHex(0xf8f8f2ff)), R2D_TEXT_ALIGN_CENTER);
}

static void StateExample_GameUpdate(float dt, void *state_data, void *user_data)
{
    (void)state_data;

    StateExample *example = (StateExample *)user_data;

    example->time += dt;
    if (R2D_InputPressed(&example->input, "pause")) {
        R2D_StateMachinePush(&example->states, StateExample_PauseState());
    }
}

static void StateExample_GameDraw(void *state_data, void *user_data)
{
    (void)state_data;

    const StateExample *example = (const StateExample *)user_data;
    const int x = 140 + (int)(sinf(example->time * 2.0f) * 52.0f);

    DrawText("Game State", 16, 14, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("ESC pushes Pause on top. The game stays underneath.", 16, 34, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawCircle(x, 106, 14.0f, R2D_ColorFromHex(0x50fa7bff));
    DrawRectangleLines(62, 82, 196, 48, R2D_ColorFromHex(0x3a506bff));
}

static void StateExample_PauseUpdate(float dt, void *state_data, void *user_data)
{
    (void)dt;
    (void)state_data;

    StateExample *example = (StateExample *)user_data;

    if (R2D_InputPressed(&example->input, "pause")) {
        R2D_StateMachinePop(&example->states);
    }

    if (R2D_InputPressed(&example->input, "submit")) {
        example->time = 0;
        R2D_StateMachineSet(&example->states, StateExample_ExitState());
    }
}

static void StateExample_PauseDraw(void *state_data, void *user_data)
{
    (void)state_data;
    (void)user_data;

    DrawRectangle(0, 0, 320, 200, R2D_ColorFromHex(0x00000099));
    R2D_DrawUiPanel(R2D_Rect(92.0f, 72.0f, 136.0f, 54.0f), R2D_DefaultUiStyle());
    R2D_DrawTextAligned("Pause Overlay", R2D_Rect(92.0f, 82.0f, 136.0f, 16.0f), R2D_DefaultTextStyle(12, R2D_ColorFromHex(0xffd166ff)), R2D_TEXT_ALIGN_CENTER);
    R2D_DrawTextAligned("ESC resume", R2D_Rect(92.0f, 102.0f, 136.0f, 12.0f), R2D_DefaultTextStyle(10, R2D_ColorFromHex(0xf8f8f2ff)), R2D_TEXT_ALIGN_CENTER);
    R2D_DrawTextAligned("Z / Enter exit", R2D_Rect(92.0f, 114.0f, 136.0f, 12.0f), R2D_DefaultTextStyle(10, R2D_ColorFromHex(0x8ecae6ff)), R2D_TEXT_ALIGN_CENTER);
}

static void StateExample_ExitUpdate(float dt, void *state_data, void *user_data)
{
    (void)dt;
    (void)state_data;
    
    StateExample *example = (StateExample *)user_data;
    example->time += dt;

    if (example->time > 1.0f) {
        R2D_RequestClose(example->context);
    }
}

static void StateExample_ExitDraw(void *state_data, void *user_data)
{
    (void)state_data;
    (void)user_data;

    R2D_TextStyle title = R2D_DefaultTextStyle(18, R2D_ColorFromHex(0xffd166ff));
    title.use_shadow = true;

    R2D_DrawTextAligned("Exit State", R2D_Rect(0.0f, 72.0f, 320.0f, 24.0f), title, R2D_TEXT_ALIGN_CENTER);
    R2D_DrawTextAligned("closing app...", R2D_Rect(0.0f, 106.0f, 320.0f, 16.0f), R2D_DefaultTextStyle(10, R2D_ColorFromHex(0xf8f8f2ff)), R2D_TEXT_ALIGN_CENTER);
}

static R2D_State StateExample_TitleState(void)
{
    return (R2D_State) {
        "Title",
        0,
        StateExample_TitleUpdate,
        StateExample_TitleDraw,
        0,
        0
    };
}

static R2D_State StateExample_GameState(void)
{
    return (R2D_State) {
        "Game",
        0,
        StateExample_GameUpdate,
        StateExample_GameDraw,
        0,
        0
    };
}

static R2D_State StateExample_PauseState(void)
{
    return (R2D_State) {
        "Pause",
        0,
        StateExample_PauseUpdate,
        StateExample_PauseDraw,
        0,
        0
    };
}

static R2D_State StateExample_ExitState(void)
{
    return (R2D_State) {
        "Exit",
        0,
        StateExample_ExitUpdate,
        StateExample_ExitDraw,
        0,
        0
    };
}

static void StateExample_Update(float dt, void *user_data)
{
    StateExample *example = (StateExample *)user_data;

    R2D_InputUpdate(&example->input);
    R2D_StateMachineUpdate(&example->states, dt);
}

static void StateExample_Draw(void *user_data)
{
    StateExample *example = (StateExample *)user_data;

    R2D_StateMachineDrawStack(&example->states);
}

static void StateExample_Shutdown(void *user_data)
{
    StateExample *example = (StateExample *)user_data;

    R2D_StateMachineClear(&example->states);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    StateExample example = { 0 };

    config.title = "Retro2D State Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    example.context = &context;

    R2D_Run(&context, (R2D_App) {
        StateExample_Init,
        StateExample_Update,
        StateExample_Draw,
        StateExample_Shutdown,
        &example
    });

    R2D_Close(&context);
    return 0;
}
