#include "r2d/r2d.h"

#include <string.h>

static int R2D_ColorDistanceSquared(Color a, Color b)
{
    const int dr = (int)a.r - (int)b.r;
    const int dg = (int)a.g - (int)b.g;
    const int db = (int)a.b - (int)b.b;
    const int da = (int)a.a - (int)b.a;

    return dr * dr + dg * dg + db * db + da * da;
}

R2D_Palette R2D_PaletteCreate(const Color *colors, int count)
{
    R2D_Palette palette = { 0 };

    if (colors == 0 || count <= 0) {
        return palette;
    }

    if (count > R2D_PALETTE_MAX_COLORS) {
        count = R2D_PALETTE_MAX_COLORS;
    }

    memcpy(palette.colors, colors, (size_t)count * sizeof(Color));
    palette.count = count;
    return palette;
}

R2D_Palette R2D_PaletteFromHex(const unsigned int *rgba, int count)
{
    R2D_Palette palette = { 0 };

    if (rgba == 0 || count <= 0) {
        return palette;
    }

    if (count > R2D_PALETTE_MAX_COLORS) {
        count = R2D_PALETTE_MAX_COLORS;
    }

    for (int i = 0; i < count; ++i) {
        palette.colors[i] = R2D_ColorFromHex(rgba[i]);
    }

    palette.count = count;
    return palette;
}

Color R2D_PaletteColor(const R2D_Palette *palette, int index, Color fallback)
{
    if (palette == 0 || index < 0 || index >= palette->count) {
        return fallback;
    }

    return palette->colors[index];
}

int R2D_PaletteNearestIndex(const R2D_Palette *palette, Color color)
{
    int best_index = -1;
    int best_distance = 0;

    if (palette == 0 || palette->count <= 0) {
        return -1;
    }

    for (int i = 0; i < palette->count; ++i) {
        const int distance = R2D_ColorDistanceSquared(color, palette->colors[i]);

        if (best_index < 0 || distance < best_distance) {
            best_index = i;
            best_distance = distance;
        }
    }

    return best_index;
}

Color R2D_PaletteNearestColor(const R2D_Palette *palette, Color color, Color fallback)
{
    const int index = R2D_PaletteNearestIndex(palette, color);

    return index >= 0 ? palette->colors[index] : fallback;
}

Color R2D_PaletteMixColor(Color color, Color target, float amount)
{
    return R2D_LerpColor(color, target, amount);
}

Color R2D_PaletteFadeColor(Color color, Color target, float amount)
{
    target.a = color.a;
    return R2D_LerpColor(color, target, amount);
}

Image R2D_ImageRecolorPalette(Image image, const R2D_Palette *from, const R2D_Palette *to, float amount)
{
    Image result;
    Color *pixels;
    const int pixel_count = image.width * image.height;

    if (image.data == 0 || image.width <= 0 || image.height <= 0 || from == 0 || to == 0 ||
        from->count <= 0 || to->count <= 0) {
        return (Image) { 0 };
    }

    result = ImageCopy(image);
    ImageFormat(&result, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    pixels = LoadImageColors(result);

    if (pixels == 0) {
        UnloadImage(result);
        return (Image) { 0 };
    }

    for (int i = 0; i < pixel_count; ++i) {
        const int palette_index = R2D_PaletteNearestIndex(from, pixels[i]);
        Color target = R2D_PaletteColor(to, palette_index, pixels[i]);

        if (pixels[i].a == 0) {
            continue;
        }

        pixels[i] = R2D_LerpColor(pixels[i], target, amount);
    }

    memcpy(result.data, pixels, (size_t)pixel_count * sizeof(Color));
    UnloadImageColors(pixels);
    return result;
}

Texture2D R2D_LoadTextureFromPalette(Image image, const R2D_Palette *from, const R2D_Palette *to, float amount)
{
    Texture2D texture = { 0 };
    Image recolored = R2D_ImageRecolorPalette(image, from, to, amount);

    if (recolored.data == 0) {
        return texture;
    }

    texture = LoadTextureFromImage(recolored);
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    UnloadImage(recolored);
    return texture;
}
