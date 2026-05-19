#include "r2d/r2d.h"

typedef struct Hello {
    R2D_Context *context;
} Hello;

static void Hello_DrawLine(int y, const char *title, const char *target, Color color)
{
    DrawText(title, 24, y, 10, color);
    DrawText(target, 168, y, 10, R2D_ColorFromHex(0x8ecae6ff));
}

static void Hello_Draw(void *user_data)
{
    // Screen height is 200 px,text size is 10 px, so we need to ajust content to fit.
    const Hello *hello = (const Hello *)user_data;
    R2D_TextStyle title = R2D_DefaultTextStyle(18, R2D_ColorFromHex(0xffd166ff));
    R2D_TextStyle body = R2D_DefaultTextStyle(10, R2D_ColorFromHex(0xf8f8f2ff));

    title.use_shadow = true;

    R2D_DrawTextAligned(
        "Retro2DFramework",
        R2D_Rect(0.0f, 2.0f, (float)R2D_VirtualWidth(hello->context), 24.0f),
        title,
        R2D_TEXT_ALIGN_CENTER
    );

    R2D_DrawTextWrapped(
        "This target is intentionally small. Each system has its own example so the code can work as documentation.",
        R2D_Rect(34.0f, 34.0f, 252.0f, 36.0f),
        body
    );

    DrawLine(24, 76, 296, 76, R2D_ColorFromHex(0x3a506bff));
    Hello_DrawLine(84, "Collect game", "r2d_collect", R2D_ColorFromHex(0xf8f8f2ff));
    Hello_DrawLine(100, "Input actions", "r2d_input_example", R2D_ColorFromHex(0xf8f8f2ff));
    Hello_DrawLine(116, "Retro UI/text", "r2d_ui_example", R2D_ColorFromHex(0xf8f8f2ff));
    Hello_DrawLine(132, "Audio", "r2d_audio_example", R2D_ColorFromHex(0xf8f8f2ff));
    Hello_DrawLine(148, "States", "r2d_state_example", R2D_ColorFromHex(0xf8f8f2ff));
    // If need more space, we can use scrolling or some other solution, but for now we just need to fit the content.

    DrawText("Alt+Enter or F11 fullscreen   F12 screenshot", 46, 185, 8, R2D_ColorFromHex(0x8ecae6ff));
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    Hello hello = { 0 };

    config.title = "Retro2DFramework Hello";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    hello.context = &context;

    R2D_Run(&context, (R2D_App) {
        0,
        0,
        Hello_Draw,
        0,
        &hello
    });

    R2D_Close(&context);
    return 0;
}
