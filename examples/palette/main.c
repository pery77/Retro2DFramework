#include "r2d/r2d.h"

#include <stdio.h>

#define PALETTE_VARIANTS 4
#define PALETTE_SIZE 16

typedef struct PaletteExample {
    R2D_Context *context;
    Image source_image;
    Texture2D variants[PALETTE_VARIANTS];
    R2D_Palette palettes[PALETTE_VARIANTS];
    int selected;
    float flash;
    float fade;
} PaletteExample;

static const unsigned int BASE_COLORS[PALETTE_SIZE] = {
    0x140c1cff,
    0x442434ff,
    0x30346dff,
    0x4e4a4eff,
    0x854c30ff,
    0x346524ff,
    0xd04648ff,
    0x757161ff,
    0x597dceff,
    0xd27d2cff,
    0x8595a1ff,
    0x6daa2cff,
    0xd2aa99ff,
    0x6dc2caff,
    0xdad45eff,
    0xdeeed6ff
};

static const unsigned int DAWN_COLORS[PALETTE_SIZE] = {
    0x1a0f1fff,
    0x4c2638ff,
    0x3a346dff,
    0x5b4b52ff,
    0x8f4930ff,
    0x455f29ff,
    0xd35d4bff,
    0x84705dff,
    0x6d82ceff,
    0xd98636ff,
    0x9394a4ff,
    0x83a936ff,
    0xd9a78eff,
    0x79bcc2ff,
    0xe0c65dff,
    0xffd6a5ff
};

static const unsigned int FOREST_COLORS[PALETTE_SIZE] = {
    0x091512ff,
    0x1c2d25ff,
    0x25375fff,
    0x3f4b43ff,
    0x5f4b2fff,
    0x2e6535ff,
    0x9d4b3fff,
    0x687456ff,
    0x4e83aaff,
    0xaa7632ff,
    0x7f9a8dff,
    0x7fb069ff,
    0xbda98bff,
    0x7ec9aeff,
    0xd8d077ff,
    0xf2e8c9ff
};

static const unsigned int GHOST_COLORS[PALETTE_SIZE] = {
    0x0d1020ff,
    0x25243aff,
    0x30346dff,
    0x4a536aff,
    0x55506bff,
    0x426263ff,
    0x7a586fff,
    0x6d7488ff,
    0x6f8fc6ff,
    0x8e7893ff,
    0x8fa5b6ff,
    0x8bb3a2ff,
    0xb9afc0ff,
    0x9fd3c7ff,
    0xd6d6aaff,
    0xf8f8f2ff
};

static const char *PALETTE_NAMES[PALETTE_VARIANTS] = {
    "base",
    "dawn",
    "forest",
    "ghost"
};

static Image PaletteExample_LoadSpriteImage(void)
{
    Image atlas = LoadImage(R2D_AssetPath("textures/DawnLike/Items/Food.png"));
    Image sprite;

    if (atlas.data == 0) {
        return GenImageColor(16, 16, BLANK);
    }

    sprite = ImageFromImage(atlas, (Rectangle) { 16.0f, 16.0f, 16.0f, 16.0f });
    ImageFormat(&sprite, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    UnloadImage(atlas);
    return sprite;
}

static void PaletteExample_RebuildTextures(PaletteExample *example)
{
    for (int i = 0; i < PALETTE_VARIANTS; ++i) {
        if (IsTextureValid(example->variants[i])) {
            UnloadTexture(example->variants[i]);
        }

        example->variants[i] = R2D_LoadTextureFromPalette(
            example->source_image,
            &example->palettes[0],
            &example->palettes[i],
            1.0f
        );
    }
}

static void PaletteExample_Init(void *user_data)
{
    PaletteExample *example = (PaletteExample *)user_data;

    example->palettes[0] = R2D_PaletteFromHex(BASE_COLORS, PALETTE_SIZE);
    example->palettes[1] = R2D_PaletteFromHex(DAWN_COLORS, PALETTE_SIZE);
    example->palettes[2] = R2D_PaletteFromHex(FOREST_COLORS, PALETTE_SIZE);
    example->palettes[3] = R2D_PaletteFromHex(GHOST_COLORS, PALETTE_SIZE);
    example->source_image = PaletteExample_LoadSpriteImage();
    PaletteExample_RebuildTextures(example);
}

static void PaletteExample_Update(float dt, void *user_data)
{
    PaletteExample *example = (PaletteExample *)user_data;

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        example->selected = (example->selected + 1) % PALETTE_VARIANTS;
        example->flash = 1.0f;
    }

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        example->selected = (example->selected + PALETTE_VARIANTS - 1) % PALETTE_VARIANTS;
        example->flash = 1.0f;
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        example->fade += dt;
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        example->fade -= dt;
    }

    example->fade = R2D_Clamp01(example->fade);
    if (example->flash > 0.0f) {
        example->flash -= dt * 2.8f;
        if (example->flash < 0.0f) {
            example->flash = 0.0f;
        }
    }
}

static void PaletteExample_DrawSwatches(const R2D_Palette *palette, int x, int y)
{
    for (int i = 1; i < palette->count; ++i) {
        DrawRectangle(x + (i - 1) * 16, y, 14, 14, palette->colors[i]);
        DrawRectangleLines(x + (i - 1) * 16, y, 14, 14, R2D_ColorFromHex(0xf8f8f255));
    }
}

static void PaletteExample_Draw(void *user_data)
{
    const PaletteExample *example = (const PaletteExample *)user_data;
    const Texture2D current = example->variants[example->selected];
    Color flash = R2D_PaletteMixColor(BLANK, R2D_ColorFromHex(0xffffffff), example->flash);
    Color fade = R2D_PaletteFadeColor(WHITE, R2D_ColorFromHex(0x15151fff), example->fade);
    char text[96];

    DrawText("Palette example", 8, 12, 14, R2D_ColorFromHex(0xffd166ff));
    DrawText("Left/Right palette. Up/Down fade.", 8, 25, 8, R2D_ColorFromHex(0xf8f8f2ff));
    DrawText("Palette swaps rebuild textures.", 8, 35, 8, R2D_ColorFromHex(0xf8f8f2ff));

    for (int i = 0; i < PALETTE_VARIANTS; ++i) {
        const int x = 24 + i * 72;
        const int y = 62;
        const Color outline = i == example->selected ? R2D_ColorFromHex(0xffd166ff) : R2D_ColorFromHex(0x3a506bff);

        DrawRectangleLines(x - 4, y - 4, 56, 58, outline);
        DrawTextureEx(example->variants[i], (Vector2) { (float)x, (float)y }, 0.0f, 1.5f, WHITE);
        DrawText(PALETTE_NAMES[i], x - 1, y + 50, 8, R2D_ColorFromHex(0xf8f8f2ff));
    }

    flash.a = (unsigned char)(example->flash * 140.0f);
    DrawTextureEx(current, (Vector2) { 124.0f, 126.0f }, 0.0f, 2.5f, WHITE);
    DrawRectangle(124, 126, 80, 80, flash);
    DrawRectangle(124, 126, 80, 80, (Color) { fade.r, fade.g, fade.b, (unsigned char)(example->fade * 180.0f) });
    PaletteExample_DrawSwatches(&example->palettes[example->selected], 96, 180);

    snprintf(text, sizeof(text), "active:%s  flash:%0.2f  fade:%0.2f", PALETTE_NAMES[example->selected], example->flash, example->fade);
    DrawText(text, 12, 188, 8, R2D_ColorFromHex(0x8ecae6ff));
}

static void PaletteExample_Shutdown(void *user_data)
{
    PaletteExample *example = (PaletteExample *)user_data;

    for (int i = 0; i < PALETTE_VARIANTS; ++i) {
        if (IsTextureValid(example->variants[i])) {
            UnloadTexture(example->variants[i]);
        }
    }

    if (example->source_image.data != 0) {
        UnloadImage(example->source_image);
    }
}

int main(void)
{
    R2D_Context context = { 0 };
    R2D_Config config = R2D_DefaultConfig();
    PaletteExample example = { 0 };

    config.title = "Retro2D Palette Example";
    config.clear_color = R2D_ColorFromHex(0x15151fff);

    if (!R2D_Init(&context, config)) {
        return 1;
    }

    example.context = &context;
    R2D_Run(&context, (R2D_App) {
        PaletteExample_Init,
        PaletteExample_Update,
        PaletteExample_Draw,
        PaletteExample_Shutdown,
        &example
    });

    R2D_Close(&context);
    return 0;
}
