#include "r2d/r2d.h"

#include <stdio.h>

typedef struct InputExample {
    R2D_InputMap input;
    float flash;
} InputExample;

static void InputExample_Init(void *user_data)
{
    InputExample *example = (InputExample *)user_data;

    R2D_InputInit(&example->input);

    /* Bind gameplay actions once, then use action names everywhere else. */
    R2D_InputBindKey(&example->input, "left", KEY_LEFT);
    R2D_InputBindKey(&example->input, "left", KEY_A);
    R2D_InputBindGamepadButton(&example->input, "left", GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    R2D_InputBindGamepadAxis(&example->input, "left", GAMEPAD_AXIS_LEFT_X, false);

    R2D_InputBindKey(&example->input, "right", KEY_RIGHT);
    R2D_InputBindKey(&example->input, "right", KEY_D);
    R2D_InputBindGamepadButton(&example->input, "right", GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    R2D_InputBindGamepadAxis(&example->input, "right", GAMEPAD_AXIS_LEFT_X, true);

    R2D_InputBindKey(&example->input, "up", KEY_UP);
    R2D_InputBindKey(&example->input, "up", KEY_W);
    R2D_InputBindGamepadButton(&example->input, "up", GAMEPAD_BUTTON_LEFT_FACE_UP);
    R2D_InputBindGamepadAxis(&example->input, "up", GAMEPAD_AXIS_LEFT_Y, false);

    R2D_InputBindKey(&example->input, "down", KEY_DOWN);
    R2D_InputBindKey(&example->input, "down", KEY_S);
    R2D_InputBindGamepadButton(&example->input, "down", GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    R2D_InputBindGamepadAxis(&example->input, "down", GAMEPAD_AXIS_LEFT_Y, true);

    R2D_InputBindKey(&example->input, "action", KEY_Z);
    R2D_InputBindMouseButton(&example->input, "action", MOUSE_BUTTON_LEFT);
    R2D_InputBindGamepadButton(&example->input, "action", GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

static void InputExample_Update(float dt, void *user_data)
{
    InputExample *example = (InputExample *)user_data;

    R2D_InputUpdate(&example->input);

    if (R2D_InputPressed(&example->input, "action")) {
        example->flash = 0.25f;
    }

    if (example->flash > 0.0f) {
        example->flash -= dt;
    }
}

static void InputExample_DrawButton(int x, int y, const char *label, bool down)
{
    R2D_UiStyle ui = R2D_DefaultUiStyle();

    R2D_DrawUiButton(R2D_Rect((float)x, (float)y, 42.0f, 18.0f), label, false, down, ui);
}

static void InputExample_Draw(void *user_data)
{
    const InputExample *example = (const InputExample *)user_data;
    const float x = R2D_InputAxis(&example->input, "left", "right");
    const float y = R2D_InputAxis(&example->input, "up", "down");
    const int center_x = 232;
    const int center_y = 100;
    char text[80];

    DrawText("Input example", 16, 14, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("WASD / arrows / gamepad move the dot.", 16, 34, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("Z or mouse click flashes action.", 16, 46, 8, R2D_ColorFromHex(0xf8f8f2ff));

    R2D_DrawUiPanel(R2D_Rect(20.0f, 70.0f, 128.0f, 80.0f), R2D_DefaultUiStyle());
    InputExample_DrawButton(63, 78, "UP", R2D_InputDown(&example->input, "up"));
    InputExample_DrawButton(63, 122, "DOWN", R2D_InputDown(&example->input, "down"));
    InputExample_DrawButton(24, 100, "LEFT", R2D_InputDown(&example->input, "left"));
    InputExample_DrawButton(102, 100, "RIGHT", R2D_InputDown(&example->input, "right"));

    R2D_DrawUiPanel(R2D_Rect(178.0f, 58.0f, 108.0f, 108.0f), R2D_DefaultUiStyle());
    DrawCircleLines(center_x, center_y, 38.0f, R2D_ColorFromHex(0x8ecae6ff));
    DrawLine(center_x - 38, center_y, center_x + 38, center_y, R2D_ColorFromHex(0x3a506bff));
    DrawLine(center_x, center_y - 38, center_x, center_y + 38, R2D_ColorFromHex(0x3a506bff));
    DrawCircle(center_x + (int)(x * 36.0f), center_y + (int)(y * 36.0f), 5.0f, R2D_ColorFromHex(0xffd166ff));

    snprintf(text, sizeof(text), "axis %.2f %.2f", x, y);
    DrawText(text, 190, 146, 8, R2D_ColorFromHex(0xf8f8f2ff));

    if (example->flash > 0.0f) {
        DrawRectangleLinesEx(R2D_Rect(52.0f, 162.0f, 96.0f, 18.0f), 2.0f, R2D_ColorFromHex(0xffd166ff));
    }
    DrawText(R2D_InputDown(&example->input, "action") ? "ACTION DOWN" : "ACTION READY", 60, 168, 8, R2D_ColorFromHex(0xf8f8f2ff));
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    InputExample example = { 0 };

    config.title = "Retro2D Input Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    R2D_Run(&context, (R2D_App) {
        InputExample_Init,
        InputExample_Update,
        InputExample_Draw,
        0,
        &example
    });

    R2D_Close(&context);
    return 0;
}
