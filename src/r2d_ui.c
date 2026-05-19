#include "r2d/r2d.h"

#include <stdio.h>
#include <string.h>

#define R2D_TYPEWRITER_DRAW_MAX 512

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
    return R2D_LoadFont(path);
}

void R2D_UnloadBitmapFont(Font *font)
{
    R2D_UnloadFont(font);
}

Font R2D_LoadFont(const char *path)
{
    const char *asset_path = R2D_AssetPath(path);
    Font font = LoadFont(asset_path);

    if (font.texture.id != 0) {
        SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
    }

    return font;
}

void R2D_UnloadFont(Font *font)
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

void R2D_UiNavInit(R2D_UiNav *nav, int item_count)
{
    if (nav == 0) {
        return;
    }

    *nav = (R2D_UiNav) { 0 };
    R2D_UiNavSetCount(nav, item_count);
}

void R2D_UiNavSetCount(R2D_UiNav *nav, int item_count)
{
    if (nav == 0) {
        return;
    }

    if (item_count < 0) {
        item_count = 0;
    }

    nav->count = item_count;
    if (nav->focus >= nav->count) {
        nav->focus = nav->count > 0 ? nav->count - 1 : 0;
    }
}

void R2D_UiNavMove(R2D_UiNav *nav, int delta)
{
    int next;

    if (nav == 0 || nav->count <= 0 || delta == 0) {
        return;
    }

    next = nav->focus + delta;
    while (next < 0) {
        next += nav->count;
    }
    while (next >= nav->count) {
        next -= nav->count;
    }

    if (next != nav->focus) {
        nav->focus = next;
        nav->changed = true;
    }
}

void R2D_UiNavUpdate(
    R2D_UiNav *nav,
    const R2D_InputMap *input,
    const char *previous_action,
    const char *next_action,
    const char *submit_action
)
{
    if (nav == 0 || input == 0) {
        return;
    }

    nav->submitted = false;
    nav->changed = false;

    if (R2D_InputPressed(input, previous_action)) {
        R2D_UiNavMove(nav, -1);
    }

    if (R2D_InputPressed(input, next_action)) {
        R2D_UiNavMove(nav, 1);
    }

    if (R2D_InputPressed(input, submit_action)) {
        nav->submitted = true;
    }
}

bool R2D_UiNavSubmitted(const R2D_UiNav *nav, int item)
{
    return nav != 0 && nav->submitted && nav->focus == item;
}

bool R2D_UiNavFocused(const R2D_UiNav *nav, int item)
{
    return nav != 0 && nav->focus == item;
}

R2D_NineSlice R2D_NineSliceCreate(Texture2D texture, Rectangle source, int left, int top, int right, int bottom)
{
    if (source.width <= 0.0f || source.height <= 0.0f) {
        source = R2D_Rect(0.0f, 0.0f, (float)texture.width, (float)texture.height);
    }

    return (R2D_NineSlice) {
        texture,
        source,
        left,
        top,
        right,
        bottom
    };
}

void R2D_DrawUiPanel(Rectangle rect, R2D_UiStyle style)
{
    DrawRectangleRec(rect, style.panel);
    DrawRectangleLinesEx(rect, (float)style.border_size, style.border);
}

void R2D_DrawUiNineSlice(R2D_NineSlice slice, Rectangle rect, Color tint)
{
    const float left = (float)slice.left;
    const float top = (float)slice.top;
    const float right = (float)slice.right;
    const float bottom = (float)slice.bottom;
    const float source_center_w = slice.source.width - left - right;
    const float source_center_h = slice.source.height - top - bottom;
    const float dest_center_w = rect.width - left - right;
    const float dest_center_h = rect.height - top - bottom;
    const float sx = slice.source.x;
    const float sy = slice.source.y;

    if (slice.texture.id == 0 || rect.width <= 0.0f || rect.height <= 0.0f ||
        source_center_w <= 0.0f || source_center_h <= 0.0f ||
        dest_center_w <= 0.0f || dest_center_h <= 0.0f) {
        return;
    }

    {
        const Rectangle sources[9] = {
            { sx, sy, left, top },
            { sx + left, sy, source_center_w, top },
            { sx + left + source_center_w, sy, right, top },
            { sx, sy + top, left, source_center_h },
            { sx + left, sy + top, source_center_w, source_center_h },
            { sx + left + source_center_w, sy + top, right, source_center_h },
            { sx, sy + top + source_center_h, left, bottom },
            { sx + left, sy + top + source_center_h, source_center_w, bottom },
            { sx + left + source_center_w, sy + top + source_center_h, right, bottom }
        };
        const Rectangle dests[9] = {
            { rect.x, rect.y, left, top },
            { rect.x + left, rect.y, dest_center_w, top },
            { rect.x + left + dest_center_w, rect.y, right, top },
            { rect.x, rect.y + top, left, dest_center_h },
            { rect.x + left, rect.y + top, dest_center_w, dest_center_h },
            { rect.x + left + dest_center_w, rect.y + top, right, dest_center_h },
            { rect.x, rect.y + top + dest_center_h, left, bottom },
            { rect.x + left, rect.y + top + dest_center_h, dest_center_w, bottom },
            { rect.x + left + dest_center_w, rect.y + top + dest_center_h, right, bottom }
        };

        for (int i = 0; i < 9; ++i) {
            DrawTexturePro(slice.texture, sources[i], dests[i], (Vector2) { 0.0f, 0.0f }, 0.0f, tint);
        }
    }
}

void R2D_DrawUiButton(Rectangle rect, const char *text, bool focused, bool pressed, R2D_UiStyle style)
{
    R2D_TextStyle text_style = R2D_DefaultTextStyle(style.font_size, pressed ? R2D_ColorFromHex(0x101820ff) : style.text);
    const Color fill = pressed ? style.accent : focused ? style.hot : R2D_ColorFromHex(0x1b263bff);

    DrawRectangleRec(rect, fill);
    DrawRectangleLinesEx(rect, (float)(focused ? style.border_size + 1 : style.border_size), style.border);
    R2D_DrawTextAligned(text, rect, text_style, R2D_TEXT_ALIGN_CENTER);
}

void R2D_DrawUiMenuItem(Rectangle rect, const char *text, bool focused, bool selected, R2D_UiStyle style)
{
    R2D_TextStyle text_style = R2D_DefaultTextStyle(style.font_size, focused ? R2D_ColorFromHex(0x101820ff) : style.text);
    const Color fill = focused ? style.accent : selected ? R2D_ColorFromHex(0x26344dff) : R2D_ColorFromHex(0x1b263bff);

    DrawRectangleRec(rect, fill);
    DrawRectangleLinesEx(rect, (float)(focused ? style.border_size + 1 : style.border_size), style.border);
    R2D_DrawTextStyled(focused ? ">" : selected ? "*" : " ", (Vector2) { rect.x + 4.0f, rect.y + 4.0f }, text_style);
    R2D_DrawTextStyled(text, (Vector2) { rect.x + 16.0f, rect.y + 4.0f }, text_style);
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

void R2D_DrawUiSelector(Rectangle rect, const char *text, const char *value, bool focused, R2D_UiStyle style)
{
    R2D_TextStyle text_style = R2D_DefaultTextStyle(style.font_size, style.text);
    Rectangle value_rect = { rect.x + rect.width - 76.0f, rect.y, 68.0f, rect.height };

    DrawRectangleRec(rect, focused ? R2D_ColorFromHex(0x26344dff) : R2D_ColorFromHex(0x1b263bff));
    DrawRectangleLinesEx(rect, (float)(focused ? style.border_size + 1 : style.border_size), style.border);
    R2D_DrawTextStyled(text, (Vector2) { rect.x + 8.0f, rect.y + 5.0f }, text_style);
    R2D_DrawTextAligned(value, value_rect, text_style, R2D_TEXT_ALIGN_CENTER);
    DrawText("<", (int)value_rect.x - 8, (int)value_rect.y + 5, style.font_size, focused ? style.accent : style.hot);
    DrawText(">", (int)(value_rect.x + value_rect.width + 2.0f), (int)value_rect.y + 5, style.font_size, focused ? style.accent : style.hot);
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

void R2D_DrawUiDialog(Rectangle rect, const char *title, const char *text, R2D_UiStyle style)
{
    R2D_TextStyle title_style = R2D_DefaultTextStyle(style.font_size + 2, style.accent);
    R2D_TextStyle text_style = R2D_DefaultTextStyle(style.font_size, style.text);

    R2D_DrawUiPanel(rect, style);
    R2D_DrawTextStyled(title, (Vector2) { rect.x + 8.0f, rect.y + 6.0f }, title_style);
    DrawLine((int)rect.x + 6, (int)rect.y + 22, (int)(rect.x + rect.width - 6.0f), (int)rect.y + 22, style.border);
    R2D_DrawTextWrapped(text, R2D_Rect(rect.x + 8.0f, rect.y + 28.0f, rect.width - 16.0f, rect.height - 34.0f), text_style);
}

void R2D_TypewriterStart(R2D_Typewriter *typewriter, const char *text, float chars_per_second)
{
    if (typewriter == 0) {
        return;
    }

    if (text == 0) {
        text = "";
    }

    if (chars_per_second <= 0.0f) {
        chars_per_second = 30.0f;
    }

    typewriter->text = text;
    typewriter->chars_per_second = chars_per_second;
    typewriter->timer = 0.0f;
    typewriter->visible_count = 0;
    typewriter->text_length = (int)strlen(text);
    typewriter->done = typewriter->text_length == 0;
}

void R2D_TypewriterReset(R2D_Typewriter *typewriter)
{
    if (typewriter == 0) {
        return;
    }

    R2D_TypewriterStart(typewriter, typewriter->text, typewriter->chars_per_second);
}

void R2D_TypewriterUpdate(R2D_Typewriter *typewriter, float dt)
{
    int step;

    if (typewriter == 0 || typewriter->done) {
        return;
    }

    typewriter->timer += dt * typewriter->chars_per_second;
    step = (int)typewriter->timer;

    if (step <= 0) {
        return;
    }

    typewriter->timer -= (float)step;
    typewriter->visible_count += step;

    if (typewriter->visible_count >= typewriter->text_length) {
        R2D_TypewriterComplete(typewriter);
    }
}

void R2D_TypewriterComplete(R2D_Typewriter *typewriter)
{
    if (typewriter == 0) {
        return;
    }

    typewriter->visible_count = typewriter->text_length;
    typewriter->timer = 0.0f;
    typewriter->done = true;
}

bool R2D_TypewriterDone(const R2D_Typewriter *typewriter)
{
    return typewriter == 0 || typewriter->done;
}

void R2D_DrawTypewriter(R2D_Typewriter typewriter, Rectangle bounds, R2D_TextStyle style)
{
    char visible[R2D_TYPEWRITER_DRAW_MAX];
    int count;

    if (typewriter.text == 0) {
        return;
    }

    count = typewriter.visible_count;
    if (count < 0) {
        count = 0;
    }
    if (count > typewriter.text_length) {
        count = typewriter.text_length;
    }
    if (count >= R2D_TYPEWRITER_DRAW_MAX) {
        count = R2D_TYPEWRITER_DRAW_MAX - 1;
    }

    memcpy(visible, typewriter.text, (size_t)count);
    visible[count] = '\0';
    R2D_DrawTextWrapped(visible, bounds, style);
}
