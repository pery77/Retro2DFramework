#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>

typedef enum UiItem {
    UI_ITEM_START = 0,
    UI_ITEM_TOGGLE,
    UI_ITEM_SLIDER,
    UI_ITEM_SELECTOR,
    UI_ITEM_COUNT
} UiItem;

typedef struct UiExample {
    R2D_InputMap input;
    R2D_UiNav nav;
    R2D_NineSlice window;
    R2D_Typewriter typewriter;
    Texture2D ui_texture;
    Font title_font;
    Font box_font;
    bool toggle;
    float slider;
    int selector;
    R2D_Crt *crt;
} UiExample;

static const char *UI_TEXT =
    "Nine-slice windows and typewriter text are useful for RPG dialogs, "
    "pause menus and tutorials.";

static void UiExample_InitInput(UiExample *example)
{
    R2D_InputInit(&example->input);
    R2D_InputBindKey(&example->input, "up", KEY_UP);
    R2D_InputBindKey(&example->input, "up", KEY_W);
    R2D_InputBindGamepadButton(&example->input, "up", GAMEPAD_BUTTON_LEFT_FACE_UP);

    R2D_InputBindKey(&example->input, "down", KEY_DOWN);
    R2D_InputBindKey(&example->input, "down", KEY_S);
    R2D_InputBindGamepadButton(&example->input, "down", GAMEPAD_BUTTON_LEFT_FACE_DOWN);

    R2D_InputBindKey(&example->input, "left", KEY_LEFT);
    R2D_InputBindKey(&example->input, "left", KEY_A);
    R2D_InputBindGamepadButton(&example->input, "left", GAMEPAD_BUTTON_LEFT_FACE_LEFT);

    R2D_InputBindKey(&example->input, "right", KEY_RIGHT);
    R2D_InputBindKey(&example->input, "right", KEY_D);
    R2D_InputBindGamepadButton(&example->input, "right", GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

    R2D_InputBindKey(&example->input, "submit", KEY_Z);
    R2D_InputBindKey(&example->input, "submit", KEY_ENTER);
    R2D_InputBindGamepadButton(&example->input, "submit", GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

    R2D_InputBindKey(&example->input, "skip", KEY_X);
    R2D_InputBindGamepadButton(&example->input, "skip", GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
}

static void UiExample_Init(void *user_data)
{
    UiExample *example = (UiExample *)user_data;

    UiExample_InitInput(example);
    R2D_UiNavInit(&example->nav, UI_ITEM_COUNT);
    example->toggle = true;
    example->slider = 0.55f;
    example->selector = 1;

    example->title_font = R2D_LoadBitmapFont("fonts/alagard.png");
    example->box_font = R2D_LoadFont("textures/DawnLike/GUI/SDS_6x6.ttf");
    example->ui_texture = R2D_LoadTexture("textures/DawnLike/GUI/GUI0.png");

    /* This source rectangle is one of the flat panels in the UI sheet. */
    example->window = R2D_NineSliceCreate(
        example->ui_texture,
        R2D_Rect(208.0f, 112.0f, 48.0f, 48.0f),
        16,
        16,
        16,
        16
    );

    R2D_TypewriterStart(&example->typewriter, UI_TEXT, 34.0f);
}

static void UiExample_Update(float dt, void *user_data)
{
    UiExample *example = (UiExample *)user_data;

    R2D_InputUpdate(&example->input);
    R2D_UiNavUpdate(&example->nav, &example->input, "up", "down", "submit");

    if (R2D_UiNavSubmitted(&example->nav, UI_ITEM_START)) {
        R2D_TypewriterReset(&example->typewriter);
    }

    if (R2D_UiNavSubmitted(&example->nav, UI_ITEM_TOGGLE)) {
        example->toggle = !example->toggle;
        R2D_CrtSetEnabled(example->crt, !example->crt->enabled);
    }

    if (R2D_UiNavFocused(&example->nav, UI_ITEM_SLIDER)) {
        if (R2D_InputDown(&example->input, "left")) {
            example->slider = fmaxf(0.0f, example->slider - dt);
        }
        if (R2D_InputDown(&example->input, "right")) {
            example->slider = fminf(1.0f, example->slider + dt);
        }
    }

    if (R2D_UiNavFocused(&example->nav, UI_ITEM_SELECTOR) && R2D_InputPressed(&example->input, "left")) {
        example->selector = example->selector > 0 ? example->selector - 1 : 2;
    }

    if (R2D_UiNavFocused(&example->nav, UI_ITEM_SELECTOR) &&
        (R2D_InputPressed(&example->input, "right") || R2D_UiNavSubmitted(&example->nav, UI_ITEM_SELECTOR))) {
        example->selector = (example->selector + 1) % 3;
    }

    if (R2D_InputPressed(&example->input, "skip")) {
        if (R2D_TypewriterDone(&example->typewriter)) {
            R2D_TypewriterReset(&example->typewriter);
        } else {
            R2D_TypewriterComplete(&example->typewriter);
        }
    }

    R2D_TypewriterUpdate(&example->typewriter, dt);
}

static void UiExample_Draw(void *user_data)
{
    const UiExample *example = (const UiExample *)user_data;
    const char *values[] = { "EASY", "NORMAL", "HARD" };
    R2D_UiStyle ui = R2D_DefaultUiStyle();
    R2D_TextStyle title = R2D_DefaultTextStyle(18, R2D_ColorFromHex(0xffd166ff));
    R2D_TextStyle body = R2D_DefaultTextStyle(6, R2D_ColorFromHex(0x30346dff));

    title.font = example->title_font;
    title.use_shadow = true;
    body.font = example->box_font;
    body.font_size = 6;
    body.spacing = 1;
    body.line_spacing = 1;

    R2D_DrawTextStyled("UI example", (Vector2) { 2.0f, 6.0f }, title);
    DrawText("Up/down focus, left/right edit, Z submit, X skip/replay text", 2, 28, 8, R2D_ColorFromHex(0x8ecae6ff));

    R2D_DrawUiPanel(R2D_Rect(18.0f, 56.0f, 136.0f, 86.0f), ui);
    R2D_DrawUiMenuItem(R2D_Rect(26.0f, 64.0f, 120.0f, 16.0f), "Restart text", R2D_UiNavFocused(&example->nav, UI_ITEM_START), false, ui);
    R2D_DrawUiToggle(R2D_Rect(26.0f, 84.0f, 116.0f, 18.0f), "CRT ready", example->toggle, R2D_UiNavFocused(&example->nav, UI_ITEM_TOGGLE), ui);
    R2D_DrawUiSlider(R2D_Rect(26.0f, 106.0f, 116.0f, 18.0f), "vol", example->slider, R2D_UiNavFocused(&example->nav, UI_ITEM_SLIDER), ui);
    R2D_DrawUiSelector(R2D_Rect(26.0f, 128.0f, 120.0f, 20.0f), "mode", values[example->selector], R2D_UiNavFocused(&example->nav, UI_ITEM_SELECTOR), ui);

    R2D_DrawUiNineSlice(example->window, R2D_Rect(170.0f, 62.0f, 126.0f, 76.0f), WHITE);
    R2D_DrawTypewriter(example->typewriter, R2D_Rect(181.0f, 68.0f, 108.0f, 52.0f), body);
    DrawText(R2D_TypewriterDone(&example->typewriter) ? "X replay" : "X skip", 232, 122, 8, R2D_ColorFromHex(0x101820ff));

    R2D_DrawUiBar(R2D_Rect(52.0f, 164.0f, 216.0f, 8.0f), example->slider, R2D_ColorFromHex(0x50fa7bff), ui);
}

static void UiExample_Shutdown(void *user_data)
{
    UiExample *example = (UiExample *)user_data;

    R2D_UnloadBitmapFont(&example->title_font);
    R2D_UnloadFont(&example->box_font);
    UnloadTexture(example->ui_texture);
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    UiExample example = { 0 };
    R2D_Crt crt = { 0 };
    example.crt = &crt;

    config.title = "Retro2D UI Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    
    if (!R2D_Init(&context, config)) {
        return 1;
    }
    
    R2D_CrtInit(&crt);
    R2D_SetCrt(&context, &crt);

    R2D_Run(&context, (R2D_App) {
        UiExample_Init,
        UiExample_Update,
        UiExample_Draw,
        UiExample_Shutdown,
        &example
    });

    R2D_CrtClose(&crt);
    R2D_Close(&context);
    return 0;
}
