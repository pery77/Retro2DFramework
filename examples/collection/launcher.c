#include "launcher.h"

void CollectionLauncherInit(CollectionLauncher *launcher)
{
    launcher->selected = 0;
    launcher->blink = 0.0f;
}

void CollectionLauncherUpdate(CollectionLauncher *launcher, const R2D_InputMap *input, int game_count, float dt)
{
    if (game_count <= 0) {
        return;
    }

    launcher->blink += dt;

    if (R2D_InputPressed(input, "up")) {
        launcher->selected = (launcher->selected + game_count - 1) % game_count;
    }

    if (R2D_InputPressed(input, "down")) {
        launcher->selected = (launcher->selected + 1) % game_count;
    }
}

void CollectionLauncherDraw(const CollectionLauncher *launcher, const CollectionGameDef *games, int game_count)
{
    const int selected = launcher->selected;
    const Color text = R2D_ColorFromHex(0xf8f8f2ff);
    const Color gold = R2D_ColorFromHex(0xffd166ff);
    const Color cyan = R2D_ColorFromHex(0x8ecae6ff);
    const Color panel = R2D_ColorFromHex(0x101820ff);
    const Color panel_alt = R2D_ColorFromHex(0x162333ff);

    ClearBackground(R2D_ColorFromHex(0x15151fff));
    DrawText("R2D Cartridge Collection", 12, 10, 14, gold);
    DrawText("One executable, many internal game modules.", 12, 30, 8, text);

    R2D_DrawUiPanel(R2D_Rect(12.0f, 52.0f, 128.0f, 116.0f), R2D_DefaultUiStyle());
    for (int i = 0; i < game_count; ++i) {
        const int y = 62 + i * 30;
        const bool is_selected = i == selected;

        DrawRectangle(20, y - 4, 112, 24, is_selected ? panel_alt : panel);
        DrawRectangleLines(20, y - 4, 112, 24, is_selected ? gold : R2D_ColorFromHex(0x3a506bff));
        DrawText(games[i].title, 28, y + 3, 8, is_selected ? gold : text);
        DrawText(games[i].id, 28, y + 13, 6, cyan);
    }

    if (game_count > 0) {
        const CollectionGameDef *game = &games[selected];
        R2D_DrawUiPanel(R2D_Rect(158.0f, 52.0f, 148.0f, 116.0f), R2D_DefaultUiStyle());
        DrawText(game->title, 170, 66, 12, gold);
        DrawText(game->description, 170, 88, 8, text);
        DrawText("Enter / Z launch", 170, 138, 8, cyan);
    }

    DrawText("Up/Down select   Enter/Z start", 12, 184, 8, text);
}
