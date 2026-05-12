#include "r2d/r2d.h"

#include <stdio.h>
#include <string.h>

static Font R2D_ResolveFont(Font font)
{
    if (font.texture.id == 0) {
        return GetFontDefault();
    }

    return font;
}

static float R2D_Clamp01(float value)
{
    if (value < 0.0f) {
        return 0.0f;
    }

    if (value > 1.0f) {
        return 1.0f;
    }

    return value;
}

static int R2D_TextLineHeight(R2D_TextStyle style)
{
    const int extra = style.line_spacing > 0 ? style.line_spacing : 2;

    return style.font_size + extra;
}

static void R2D_DrawTextLine(const char *text, Vector2 position, R2D_TextStyle style)
{
    const Font font = R2D_ResolveFont(style.font);
    const float font_size = (float)style.font_size;
    const float spacing = (float)style.spacing;

    if (text == 0) {
        return;
    }

    if (style.use_outline) {
        DrawTextEx(font, text, (Vector2) { position.x - 1.0f, position.y }, font_size, spacing, style.outline);
        DrawTextEx(font, text, (Vector2) { position.x + 1.0f, position.y }, font_size, spacing, style.outline);
        DrawTextEx(font, text, (Vector2) { position.x, position.y - 1.0f }, font_size, spacing, style.outline);
        DrawTextEx(font, text, (Vector2) { position.x, position.y + 1.0f }, font_size, spacing, style.outline);
    }

    if (style.use_shadow) {
        DrawTextEx(
            font,
            text,
            (Vector2) { position.x + style.shadow_offset.x, position.y + style.shadow_offset.y },
            font_size,
            spacing,
            style.shadow
        );
    }

    DrawTextEx(font, text, position, font_size, spacing, style.tint);
}

static float R2D_DrawWrappedLine(const char *line, Rectangle bounds, float y, R2D_TextStyle style)
{
    char word[128];
    char current[256];
    int word_length = 0;
    int current_length = 0;
    float line_y = y;
    const float max_y = bounds.y + bounds.height;
    const float line_height = (float)R2D_TextLineHeight(style);

    current[0] = '\0';

    for (const char *at = line; ; ++at) {
        const bool end = *at == '\0';
        const bool split = *at == ' ' || *at == '\t' || end;

        if (!split && word_length < (int)sizeof(word) - 1) {
            word[word_length++] = *at;
        }

        if (split) {
            char candidate[384];
            word[word_length] = '\0';

            if (word_length > 0) {
                if (current_length == 0) {
                    snprintf(candidate, sizeof(candidate), "%s", word);
                } else {
                    snprintf(candidate, sizeof(candidate), "%s %s", current, word);
                }

                if (current_length > 0 && R2D_MeasureTextStyled(candidate, style).x > bounds.width) {
                    if (line_y + line_height <= max_y) {
                        R2D_DrawTextLine(current, (Vector2) { bounds.x, line_y }, style);
                    }
                    line_y += line_height;
                    snprintf(current, sizeof(current), "%s", word);
                } else {
                    snprintf(current, sizeof(current), "%s", candidate);
                }

                current_length = (int)strlen(current);
                word_length = 0;
            }

            if (end) {
                break;
            }
        }
    }

    if (current_length > 0) {
        if (line_y + line_height <= max_y) {
            R2D_DrawTextLine(current, (Vector2) { bounds.x, line_y }, style);
        }
        line_y += line_height;
    }

    return line_y;
}

R2D_TextStyle R2D_DefaultTextStyle(int font_size, Color tint)
{
    if (font_size <= 0) {
        font_size = 8;
    }

    return (R2D_TextStyle) {
        GetFontDefault(),
        font_size,
        1,
        2,
        tint,
        R2D_ColorFromHex(0x000000aa),
        R2D_ColorFromHex(0x000000ff),
        { 1.0f, 1.0f },
        false,
        false
    };
}

Font R2D_LoadBitmapFont(const char *path)
{
    const char *asset_path = R2D_AssetPath(path);
    Font font = LoadFont(asset_path);

    if (font.texture.id != 0) {
        SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
    }

    return font;
}

void R2D_UnloadBitmapFont(Font *font)
{
    if (font == 0 || font->texture.id == 0) {
        return;
    }

    UnloadFont(*font);
    *font = (Font) { 0 };
}

Vector2 R2D_MeasureTextStyled(const char *text, R2D_TextStyle style)
{
    if (text == 0) {
        text = "";
    }

    return MeasureTextEx(R2D_ResolveFont(style.font), text, (float)style.font_size, (float)style.spacing);
}

void R2D_DrawTextStyled(const char *text, Vector2 position, R2D_TextStyle style)
{
    R2D_DrawTextLine(text, position, style);
}

void R2D_DrawTextAligned(const char *text, Rectangle bounds, R2D_TextStyle style, R2D_TextAlign align)
{
    Vector2 size;
    Vector2 position;

    if (text == 0) {
        text = "";
    }

    size = R2D_MeasureTextStyled(text, style);
    position = (Vector2) { bounds.x, bounds.y + (bounds.height - size.y) * 0.5f };

    if (align == R2D_TEXT_ALIGN_CENTER) {
        position.x = bounds.x + (bounds.width - size.x) * 0.5f;
    } else if (align == R2D_TEXT_ALIGN_RIGHT) {
        position.x = bounds.x + bounds.width - size.x;
    }

    R2D_DrawTextLine(text, position, style);
}

void R2D_DrawTextWrapped(const char *text, Rectangle bounds, R2D_TextStyle style)
{
    char line[256];
    int line_length = 0;
    float y = bounds.y;
    const int line_height = R2D_TextLineHeight(style);

    if (text == 0) {
        return;
    }

    for (const char *at = text; ; ++at) {
        const bool end = *at == '\0';
        const bool newline = *at == '\n';

        if (!end && !newline && line_length < (int)sizeof(line) - 1) {
            line[line_length++] = *at;
        }

        if (end || newline) {
            line[line_length] = '\0';

            if (y + (float)line_height <= bounds.y + bounds.height) {
                y = R2D_DrawWrappedLine(line, bounds, y, style);
            } else {
                y += (float)line_height;
            }
            line_length = 0;

            if (end) {
                break;
            }
        }
    }
}

R2D_UiStyle R2D_DefaultUiStyle(void)
{
    return (R2D_UiStyle) {
        R2D_ColorFromHex(0x101820dd),
        R2D_ColorFromHex(0x3a506bff),
        R2D_ColorFromHex(0xf8f8f2ff),
        R2D_ColorFromHex(0xffd166ff),
        R2D_ColorFromHex(0x8ecae6ff),
        R2D_ColorFromHex(0x6c7086ff),
        R2D_ColorFromHex(0x50fa7bff),
        10,
        1
    };
}

void R2D_DrawUiPanel(Rectangle rect, R2D_UiStyle style)
{
    DrawRectangleRec(rect, style.panel);
    DrawRectangleLinesEx(rect, (float)style.border_size, style.border);
}

void R2D_DrawUiButton(Rectangle rect, const char *text, bool focused, bool pressed, R2D_UiStyle style)
{
    R2D_TextStyle text_style = R2D_DefaultTextStyle(style.font_size, pressed ? R2D_ColorFromHex(0x101820ff) : style.text);
    const Color fill = pressed ? style.accent : focused ? style.hot : R2D_ColorFromHex(0x1b263bff);

    DrawRectangleRec(rect, fill);
    DrawRectangleLinesEx(rect, (float)(focused ? style.border_size + 1 : style.border_size), style.border);
    R2D_DrawTextAligned(text, rect, text_style, R2D_TEXT_ALIGN_CENTER);
}

void R2D_DrawUiToggle(Rectangle rect, const char *text, bool value, bool focused, R2D_UiStyle style)
{
    R2D_TextStyle text_style = R2D_DefaultTextStyle(style.font_size, style.text);
    Rectangle box = { rect.x, rect.y + 2.0f, 12.0f, 12.0f };

    DrawRectangleRec(box, value ? style.fill : R2D_ColorFromHex(0x1b263bff));
    DrawRectangleLinesEx(box, (float)(focused ? style.border_size + 1 : style.border_size), style.border);
    if (value) {
        DrawRectangle((int)box.x + 3, (int)box.y + 3, 6, 6, R2D_ColorFromHex(0x101820ff));
    }

    R2D_DrawTextStyled(text, (Vector2) { rect.x + 18.0f, rect.y + 4.0f }, text_style);
}

void R2D_DrawUiSlider(Rectangle rect, const char *text, float value, bool focused, R2D_UiStyle style)
{
    R2D_TextStyle text_style = R2D_DefaultTextStyle(style.font_size, style.text);
    Rectangle track = { rect.x + 72.0f, rect.y + 7.0f, rect.width - 76.0f, 4.0f };
    const float clamped = R2D_Clamp01(value);
    const int knob_x = (int)(track.x + track.width * clamped);

    R2D_DrawTextStyled(text, (Vector2) { rect.x, rect.y + 3.0f }, text_style);
    DrawRectangleRec(track, R2D_ColorFromHex(0x1b263bff));
    DrawRectangle((int)track.x, (int)track.y, (int)(track.width * clamped), (int)track.height, style.fill);
    DrawRectangle(knob_x - 2, (int)track.y - 3, 5, 10, focused ? style.accent : style.hot);
}

void R2D_DrawUiBar(Rectangle rect, float value, Color fill, R2D_UiStyle style)
{
    const float clamped = R2D_Clamp01(value);

    DrawRectangleRec(rect, R2D_ColorFromHex(0x1b263bff));
    DrawRectangle((int)rect.x, (int)rect.y, (int)(rect.width * clamped), (int)rect.height, fill);
    DrawRectangleLinesEx(rect, (float)style.border_size, style.border);
}
