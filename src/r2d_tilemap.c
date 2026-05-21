#include "r2d/r2d.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R2D_TILED_FLIP_HORIZONTAL 0x80000000u
#define R2D_TILED_FLIP_VERTICAL 0x40000000u
#define R2D_TILED_FLIP_DIAGONAL 0x20000000u
#define R2D_TILED_GID_MASK (~(R2D_TILED_FLIP_HORIZONTAL | R2D_TILED_FLIP_VERTICAL | R2D_TILED_FLIP_DIAGONAL))

static const char *R2D_TilemapFindFirstObjectInArray(const char *begin, const char *end);

static const char *R2D_TilemapSkipSpace(const char *text)
{
    while (text != 0 && *text != '\0' && isspace((unsigned char)*text)) {
        ++text;
    }

    return text;
}

static const char *R2D_TilemapFindMatching(const char *begin, char open, char close)
{
    const char *cursor = begin;
    int depth = 0;
    bool in_string = false;
    bool escaped = false;

    while (cursor != 0 && *cursor != '\0') {
        const char c = *cursor;

        if (in_string) {
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                in_string = false;
            }
        } else if (c == '"') {
            in_string = true;
        } else if (c == open) {
            ++depth;
        } else if (c == close) {
            --depth;
            if (depth == 0) {
                return cursor;
            }
        }

        ++cursor;
    }

    return 0;
}

static const char *R2D_TilemapFindKey(const char *begin, const char *end, const char *key)
{
    char pattern[96];
    const char *cursor;

    if (begin == 0 || key == 0) {
        return 0;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    cursor = begin;

    while ((cursor = strstr(cursor, pattern)) != 0 && (end == 0 || cursor < end)) {
        const char *value = R2D_TilemapSkipSpace(cursor + strlen(pattern));

        if (end != 0 && value >= end) {
            return 0;
        }

        if (*value == ':') {
            return R2D_TilemapSkipSpace(value + 1);
        }

        ++cursor;
    }

    return 0;
}

static const char *R2D_TilemapFindTopLevelKey(const char *begin, const char *end, const char *key)
{
    char pattern[96];
    const char *cursor;
    int object_depth = 0;
    int array_depth = 0;
    bool in_string = false;
    bool escaped = false;

    if (begin == 0 || key == 0) {
        return 0;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    cursor = begin;

    while (*cursor != '\0' && (end == 0 || cursor < end)) {
        const char c = *cursor;

        if (in_string) {
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                in_string = false;
            }
        } else if (c == '"') {
            if (object_depth == 1 && array_depth == 0 && strncmp(cursor, pattern, strlen(pattern)) == 0) {
                const char *value = R2D_TilemapSkipSpace(cursor + strlen(pattern));

                if ((end == 0 || value < end) && *value == ':') {
                    return R2D_TilemapSkipSpace(value + 1);
                }
            }

            in_string = true;
        } else if (c == '{') {
            ++object_depth;
        } else if (c == '}') {
            --object_depth;
        } else if (c == '[') {
            ++array_depth;
        } else if (c == ']') {
            --array_depth;
        }

        ++cursor;
    }

    return 0;
}

static bool R2D_TilemapReadInt(const char *begin, const char *end, const char *key, int *value)
{
    const char *text = R2D_TilemapFindKey(begin, end, key);
    char *parsed_end = 0;
    long parsed;

    if (text == 0 || value == 0) {
        return false;
    }

    parsed = strtol(text, &parsed_end, 10);
    if (parsed_end == text) {
        return false;
    }

    *value = (int)parsed;
    return true;
}

static bool R2D_TilemapReadFloat(const char *begin, const char *end, const char *key, float *value)
{
    const char *text = R2D_TilemapFindKey(begin, end, key);
    char *parsed_end = 0;
    float parsed;

    if (text == 0 || value == 0) {
        return false;
    }

    parsed = strtof(text, &parsed_end);
    if (parsed_end == text) {
        return false;
    }

    *value = parsed;
    return true;
}

static bool R2D_TilemapReadBool(const char *begin, const char *end, const char *key, bool fallback)
{
    const char *text = R2D_TilemapFindKey(begin, end, key);

    if (text == 0) {
        return fallback;
    }

    if (strncmp(text, "true", 4) == 0) {
        return true;
    }

    if (strncmp(text, "false", 5) == 0) {
        return false;
    }

    return atoi(text) != 0;
}

static bool R2D_TilemapReadString(
    const char *begin,
    const char *end,
    const char *key,
    char *destination,
    int destination_size
)
{
    const char *text = R2D_TilemapFindKey(begin, end, key);
    int length = 0;

    if (destination_size <= 0) {
        return false;
    }

    destination[0] = '\0';

    if (text == 0 || *text != '"') {
        return false;
    }

    ++text;
    while (*text != '\0' && *text != '"' && (end == 0 || text < end)) {
        if (*text == '\\' && text[1] != '\0') {
            ++text;
        }

        if (length < destination_size - 1) {
            destination[length++] = *text;
        }

        ++text;
    }

    destination[length] = '\0';
    return true;
}

static bool R2D_TilemapReadTopLevelString(
    const char *begin,
    const char *end,
    const char *key,
    char *destination,
    int destination_size
)
{
    const char *text = R2D_TilemapFindTopLevelKey(begin, end, key);
    int length = 0;

    if (destination_size <= 0) {
        return false;
    }

    destination[0] = '\0';

    if (text == 0 || *text != '"') {
        return false;
    }

    ++text;
    while (*text != '\0' && *text != '"' && (end == 0 || text < end)) {
        if (*text == '\\' && text[1] != '\0') {
            ++text;
        }

        if (length < destination_size - 1) {
            destination[length++] = *text;
        }

        ++text;
    }

    destination[length] = '\0';
    return true;
}

static Color R2D_TilemapParseColorString(const char *text)
{
    unsigned int value = 0;
    int length = 0;

    if (text == 0 || text[0] != '#') {
        return BLANK;
    }

    ++text;
    while (text[length] != '\0' && isxdigit((unsigned char)text[length])) {
        ++length;
    }

    value = (unsigned int)strtoul(text, 0, 16);
    if (length == 8) {
        return (Color) {
            (unsigned char)((value >> 16) & 0xffu),
            (unsigned char)((value >> 8) & 0xffu),
            (unsigned char)(value & 0xffu),
            (unsigned char)((value >> 24) & 0xffu)
        };
    }

    if (length == 6) {
        return (Color) {
            (unsigned char)((value >> 16) & 0xffu),
            (unsigned char)((value >> 8) & 0xffu),
            (unsigned char)(value & 0xffu),
            255
        };
    }

    return BLANK;
}

static bool R2D_TilemapParseProperties(
    const char *begin,
    const char *end,
    R2D_TilemapProperty *properties,
    int *property_count
)
{
    const char *array = R2D_TilemapFindKey(begin, end, "properties");
    const char *array_end;
    const char *cursor;
    int count = 0;

    if (property_count != 0) {
        *property_count = 0;
    }

    if (array == 0 || *array != '[' || properties == 0 || property_count == 0) {
        return true;
    }

    array_end = R2D_TilemapFindMatching(array, '[', ']');
    if (array_end == 0 || array_end > end) {
        return false;
    }

    cursor = array;
    while ((cursor = R2D_TilemapFindFirstObjectInArray(cursor, array_end)) != 0) {
        const char *property_end = R2D_TilemapFindMatching(cursor, '{', '}');
        R2D_TilemapProperty property = { 0 };
        char type[32] = { 0 };

        if (property_end == 0 || property_end > array_end) {
            return false;
        }

        if (count >= R2D_TILEMAP_MAX_PROPERTIES) {
            cursor = property_end + 1;
            continue;
        }

        if (!R2D_TilemapReadTopLevelString(cursor, property_end, "name", property.name, sizeof(property.name))) {
            cursor = property_end + 1;
            continue;
        }

        R2D_TilemapReadTopLevelString(cursor, property_end, "type", type, sizeof(type));
        if (strcmp(type, "int") == 0) {
            property.type = R2D_TILEMAP_PROPERTY_INT;
            R2D_TilemapReadInt(cursor, property_end, "value", &property.int_value);
            property.float_value = (float)property.int_value;
            property.bool_value = property.int_value != 0;
        } else if (strcmp(type, "float") == 0) {
            property.type = R2D_TILEMAP_PROPERTY_FLOAT;
            R2D_TilemapReadFloat(cursor, property_end, "value", &property.float_value);
            property.int_value = (int)property.float_value;
            property.bool_value = property.float_value != 0.0f;
        } else if (strcmp(type, "bool") == 0) {
            property.type = R2D_TILEMAP_PROPERTY_BOOL;
            property.bool_value = R2D_TilemapReadBool(cursor, property_end, "value", false);
            property.int_value = property.bool_value ? 1 : 0;
            property.float_value = property.bool_value ? 1.0f : 0.0f;
        } else if (strcmp(type, "color") == 0) {
            property.type = R2D_TILEMAP_PROPERTY_COLOR;
            R2D_TilemapReadString(cursor, property_end, "value", property.string_value, sizeof(property.string_value));
            property.color_value = R2D_TilemapParseColorString(property.string_value);
        } else {
            property.type = R2D_TILEMAP_PROPERTY_STRING;
            R2D_TilemapReadString(cursor, property_end, "value", property.string_value, sizeof(property.string_value));
        }

        properties[count++] = property;
        cursor = property_end + 1;
    }

    *property_count = count;
    return true;
}

static bool R2D_TilemapPathIsAbsolute(const char *path)
{
    if (path == 0 || path[0] == '\0') {
        return false;
    }

    if (path[0] == '/' || path[0] == '\\') {
        return true;
    }

    return isalpha((unsigned char)path[0]) && path[1] == ':';
}

static void R2D_TilemapDirectory(char *directory, int directory_size, const char *path)
{
    const char *slash;
    int length;

    if (directory_size <= 0) {
        return;
    }

    directory[0] = '\0';

    if (path == 0) {
        return;
    }

    slash = strrchr(path, '/');

#if defined(_WIN32)
    {
        const char *backslash = strrchr(path, '\\');
        if (backslash != 0 && (slash == 0 || backslash > slash)) {
            slash = backslash;
        }
    }
#endif

    if (slash == 0) {
        return;
    }

    length = (int)(slash - path + 1);
    if (length >= directory_size) {
        length = directory_size - 1;
    }

    memcpy(directory, path, (size_t)length);
    directory[length] = '\0';
}

static void R2D_TilemapResolvePath(char *destination, int destination_size, const char *base_path, const char *path)
{
    char directory[1024];

    if (destination_size <= 0) {
        return;
    }

    if (path == 0) {
        path = "";
    }

    if (R2D_TilemapPathIsAbsolute(path)) {
        snprintf(destination, (size_t)destination_size, "%s", path);
        return;
    }

    if (strncmp(path, "assets/", 7) == 0 || strncmp(path, "assets\\", 7) == 0) {
        snprintf(destination, (size_t)destination_size, "%s", R2D_AssetPath(path + 7));
        return;
    }

    R2D_TilemapDirectory(directory, sizeof(directory), base_path);
    snprintf(destination, (size_t)destination_size, "%s%s", directory, path);
}

static bool R2D_TilemapXmlReadInt(const char *begin, const char *end, const char *attribute, int *value)
{
    char pattern[64];
    const char *text;
    char *parsed_end = 0;
    long parsed;

    if (begin == 0 || attribute == 0 || value == 0) {
        return false;
    }

    snprintf(pattern, sizeof(pattern), "%s=\"", attribute);
    text = strstr(begin, pattern);

    if (text == 0 || (end != 0 && text >= end)) {
        return false;
    }

    text += strlen(pattern);
    parsed = strtol(text, &parsed_end, 10);
    if (parsed_end == text) {
        return false;
    }

    *value = (int)parsed;
    return true;
}

static bool R2D_TilemapXmlReadString(
    const char *begin,
    const char *end,
    const char *attribute,
    char *destination,
    int destination_size
)
{
    char pattern[64];
    const char *text;
    int length = 0;

    if (destination_size <= 0) {
        return false;
    }

    destination[0] = '\0';

    if (begin == 0 || attribute == 0) {
        return false;
    }

    snprintf(pattern, sizeof(pattern), "%s=\"", attribute);
    text = strstr(begin, pattern);

    if (text == 0 || (end != 0 && text >= end)) {
        return false;
    }

    text += strlen(pattern);
    while (*text != '\0' && *text != '"' && (end == 0 || text < end)) {
        if (length < destination_size - 1) {
            destination[length++] = *text;
        }

        ++text;
    }

    destination[length] = '\0';
    return length > 0;
}

static const char *R2D_TilemapFindFirstObjectInArray(const char *begin, const char *end)
{
    const char *cursor = begin;

    while (cursor != 0 && *cursor != '\0' && (end == 0 || cursor < end)) {
        if (*cursor == '{') {
            return cursor;
        }

        ++cursor;
    }

    return 0;
}

static bool R2D_TilemapParseTileData(const char *layer_begin, const char *layer_end, R2D_TilemapLayer *layer)
{
    const char *data = R2D_TilemapFindKey(layer_begin, layer_end, "data");
    const char *end;
    int count;
    int index = 0;

    if (data == 0 || *data != '[' || layer == 0 || layer->width <= 0 || layer->height <= 0) {
        return false;
    }

    end = R2D_TilemapFindMatching(data, '[', ']');
    if (end == 0 || end > layer_end) {
        return false;
    }

    count = layer->width * layer->height;
    layer->tiles = (unsigned int *)calloc((size_t)count, sizeof(unsigned int));
    if (layer->tiles == 0) {
        return false;
    }

    ++data;
    while (data < end && index < count) {
        char *next = 0;
        unsigned long parsed;

        data = R2D_TilemapSkipSpace(data);
        if (*data == ',') {
            ++data;
            continue;
        }

        parsed = strtoul(data, &next, 10);
        if (next == data) {
            break;
        }

        layer->tiles[index++] = (unsigned int)parsed;
        data = next;
    }

    return index == count;
}

static bool R2D_TilemapAppendAnimation(
    R2D_TilemapTileset *tileset,
    int tile_id,
    const R2D_TilemapAnimationFrame *frames,
    int frame_count
)
{
    R2D_TilemapTileAnimation *animations;
    R2D_TilemapTileAnimation *animation;
    int duration = 0;

    if (tileset == 0 || frames == 0 || frame_count <= 0 || tile_id < 0) {
        return false;
    }

    animations = (R2D_TilemapTileAnimation *)realloc(
        tileset->animations,
        (size_t)(tileset->animation_count + 1) * sizeof(R2D_TilemapTileAnimation)
    );
    if (animations == 0) {
        return false;
    }

    tileset->animations = animations;
    animation = &tileset->animations[tileset->animation_count];
    *animation = (R2D_TilemapTileAnimation) { 0 };
    animation->frames = (R2D_TilemapAnimationFrame *)calloc((size_t)frame_count, sizeof(R2D_TilemapAnimationFrame));
    if (animation->frames == 0) {
        return false;
    }

    for (int i = 0; i < frame_count; ++i) {
        animation->frames[i] = frames[i];
        duration += frames[i].duration_ms > 0 ? frames[i].duration_ms : 1;
    }

    animation->tile_id = tile_id;
    animation->frame_count = frame_count;
    animation->duration_ms = duration;
    tileset->animation_count++;
    return true;
}

static bool R2D_TilemapParseJsonTileAnimations(
    R2D_TilemapTileset *tileset,
    const char *tileset_begin,
    const char *tileset_end
)
{
    const char *tiles = R2D_TilemapFindKey(tileset_begin, tileset_end, "tiles");
    const char *tiles_end;
    const char *tile_begin;

    if (tiles == 0 || *tiles != '[') {
        return true;
    }

    tiles_end = R2D_TilemapFindMatching(tiles, '[', ']');
    if (tiles_end == 0 || tiles_end > tileset_end) {
        return false;
    }

    tile_begin = tiles;
    while ((tile_begin = R2D_TilemapFindFirstObjectInArray(tile_begin, tiles_end)) != 0) {
        const char *tile_end = R2D_TilemapFindMatching(tile_begin, '{', '}');
        const char *animation = 0;
        const char *animation_end = 0;
        const char *frame_begin = 0;
        R2D_TilemapAnimationFrame frames[32];
        int tile_id = -1;
        int frame_count = 0;

        if (tile_end == 0 || tile_end > tiles_end) {
            return false;
        }

        R2D_TilemapReadInt(tile_begin, tile_end, "id", &tile_id);
        animation = R2D_TilemapFindKey(tile_begin, tile_end, "animation");
        if (tile_id < 0 || animation == 0 || *animation != '[') {
            tile_begin = tile_end + 1;
            continue;
        }

        animation_end = R2D_TilemapFindMatching(animation, '[', ']');
        if (animation_end == 0 || animation_end > tile_end) {
            return false;
        }

        frame_begin = animation;
        while ((frame_begin = R2D_TilemapFindFirstObjectInArray(frame_begin, animation_end)) != 0 && frame_count < 32) {
            const char *frame_end = R2D_TilemapFindMatching(frame_begin, '{', '}');

            if (frame_end == 0 || frame_end > animation_end) {
                return false;
            }

            R2D_TilemapReadInt(frame_begin, frame_end, "tileid", &frames[frame_count].tile_id);
            R2D_TilemapReadInt(frame_begin, frame_end, "duration", &frames[frame_count].duration_ms);
            frame_count++;
            frame_begin = frame_end + 1;
        }

        if (frame_count > 0) {
            R2D_TilemapAppendAnimation(tileset, tile_id, frames, frame_count);
        }

        tile_begin = tile_end + 1;
    }

    return true;
}

static bool R2D_TilemapParseTsxTileAnimations(R2D_TilemapTileset *tileset, const char *text)
{
    const char *tile_begin = text;

    while ((tile_begin = strstr(tile_begin, "<tile ")) != 0) {
        const char *tile_tag_end = strchr(tile_begin, '>');
        const char *tile_end = strstr(tile_begin, "</tile>");
        const char *animation = 0;
        const char *animation_end = 0;
        const char *frame = 0;
        R2D_TilemapAnimationFrame frames[32];
        int tile_id = -1;
        int frame_count = 0;

        if (tile_tag_end == 0 || tile_end == 0) {
            return false;
        }

        R2D_TilemapXmlReadInt(tile_begin, tile_tag_end, "id", &tile_id);
        animation = strstr(tile_tag_end, "<animation>");
        if (tile_id < 0 || animation == 0 || animation > tile_end) {
            tile_begin = tile_end + strlen("</tile>");
            continue;
        }

        animation_end = strstr(animation, "</animation>");
        if (animation_end == 0 || animation_end > tile_end) {
            return false;
        }

        frame = animation;
        while ((frame = strstr(frame, "<frame ")) != 0 && frame < animation_end && frame_count < 32) {
            const char *frame_end = strchr(frame, '>');

            if (frame_end == 0 || frame_end > animation_end) {
                return false;
            }

            R2D_TilemapXmlReadInt(frame, frame_end, "tileid", &frames[frame_count].tile_id);
            R2D_TilemapXmlReadInt(frame, frame_end, "duration", &frames[frame_count].duration_ms);
            frame_count++;
            frame = frame_end + 1;
        }

        if (frame_count > 0) {
            R2D_TilemapAppendAnimation(tileset, tile_id, frames, frame_count);
        }

        tile_begin = tile_end + strlen("</tile>");
    }

    return true;
}

static bool R2D_TilemapAppendTileset(
    R2D_Tilemap *tilemap,
    int first_gid,
    int tile_width,
    int tile_height,
    int margin,
    int spacing,
    int columns,
    int tile_count,
    const char *image_path,
    R2D_TilemapTileset **out_tileset
)
{
    Texture2D texture;
    R2D_TilemapTileset *tilesets;
    R2D_TilemapTileset *tileset;
    int usable_width;
    int usable_height;
    int estimated_rows;

    if (tilemap == 0 || first_gid <= 0 || tile_width <= 0 || tile_height <= 0 || image_path == 0 || image_path[0] == '\0') {
        return false;
    }

    if (margin < 0) {
        margin = 0;
    }

    if (spacing < 0) {
        spacing = 0;
    }

    texture = R2D_LoadTexture(image_path);
    if (!IsTextureValid(texture)) {
        TraceLog(LOG_WARNING, "R2D: Failed to load tilemap tileset image: %s", image_path);
        return false;
    }

    SetTextureFilter(texture, TEXTURE_FILTER_POINT);

    usable_width = texture.width - margin * 2;
    usable_height = texture.height - margin * 2;

    if (columns <= 0 && usable_width >= tile_width) {
        columns = (usable_width + spacing) / (tile_width + spacing);
    }

    if (tile_count <= 0 && columns > 0 && usable_height >= tile_height) {
        estimated_rows = (usable_height + spacing) / (tile_height + spacing);
        tile_count = columns * estimated_rows;
    }

    if (columns <= 0 || tile_count <= 0) {
        UnloadTexture(texture);
        return false;
    }

    tilesets = (R2D_TilemapTileset *)realloc(
        tilemap->tilesets,
        (size_t)(tilemap->tileset_count + 1) * sizeof(R2D_TilemapTileset)
    );
    if (tilesets == 0) {
        UnloadTexture(texture);
        return false;
    }

    tilemap->tilesets = tilesets;
    tileset = &tilemap->tilesets[tilemap->tileset_count];
    *tileset = (R2D_TilemapTileset) {
        texture,
        0,
        first_gid,
        tile_width,
        tile_height,
        margin,
        spacing,
        columns,
        tile_count,
        0
    };

    if (tilemap->tileset_count == 0) {
        tilemap->texture = texture;
        tilemap->first_gid = first_gid;
        tilemap->columns = columns;
        tilemap->tile_count = tile_count;
    }

    tilemap->tileset_count++;
    if (out_tileset != 0) {
        *out_tileset = tileset;
    }

    return true;
}

static bool R2D_TilemapParseTileset(R2D_Tilemap *tilemap, const char *text, const char *path)
{
    const char *tilesets = R2D_TilemapFindKey(text, 0, "tilesets");
    const char *tilesets_end;
    const char *tileset_begin;

    if (tilesets == 0 || *tilesets != '[') {
        return false;
    }

    tilesets_end = R2D_TilemapFindMatching(tilesets, '[', ']');
    if (tilesets_end == 0) {
        return false;
    }

    tileset_begin = tilesets;
    while ((tileset_begin = R2D_TilemapFindFirstObjectInArray(tileset_begin, tilesets_end)) != 0) {
        const char *tileset_end = R2D_TilemapFindMatching(tileset_begin, '{', '}');
        int first_gid = 0;
        int tile_width = 0;
        int tile_height = 0;
        int margin = 0;
        int spacing = 0;
        int columns = 0;
        int tile_count = 0;
        char source[512];
        char image[512];
        char image_path[1200] = { 0 };
        char *external_tileset_text = 0;
        R2D_TilemapTileset *appended_tileset = 0;

        if (tileset_end == 0 || tileset_end > tilesets_end) {
            break;
        }

        if (!R2D_TilemapReadInt(tileset_begin, tileset_end, "firstgid", &first_gid)) {
            tileset_begin = tileset_end + 1;
            continue;
        }

        if (R2D_TilemapReadString(tileset_begin, tileset_end, "source", source, sizeof(source))) {
            char tileset_path[1200];
            const char *tileset_tag;
            const char *tileset_tag_end;
            const char *image_tag;
            const char *image_tag_end;

            R2D_TilemapResolvePath(tileset_path, sizeof(tileset_path), path, source);
            external_tileset_text = R2D_LoadAssetText(tileset_path);

            if (external_tileset_text == 0) {
                TraceLog(LOG_WARNING, "R2D: Failed to load Tiled TSX tileset: %s", tileset_path);
                tileset_begin = tileset_end + 1;
                continue;
            }

            tileset_tag = strstr(external_tileset_text, "<tileset");
            image_tag = strstr(external_tileset_text, "<image");

            if (tileset_tag == 0 || image_tag == 0) {
                R2D_UnloadAssetText(external_tileset_text);
                tileset_begin = tileset_end + 1;
                continue;
            }

            tileset_tag_end = strchr(tileset_tag, '>');
            image_tag_end = strchr(image_tag, '>');

            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "tilewidth", &tile_width);
            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "tileheight", &tile_height);
            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "margin", &margin);
            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "spacing", &spacing);
            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "columns", &columns);
            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "tilecount", &tile_count);

            if (!R2D_TilemapXmlReadString(image_tag, image_tag_end, "source", image, sizeof(image))) {
                R2D_UnloadAssetText(external_tileset_text);
                tileset_begin = tileset_end + 1;
                continue;
            }

            R2D_TilemapResolvePath(image_path, sizeof(image_path), tileset_path, image);
        } else {
            if (!R2D_TilemapReadString(tileset_begin, tileset_end, "image", image, sizeof(image))) {
                tileset_begin = tileset_end + 1;
                continue;
            }

            R2D_TilemapResolvePath(image_path, sizeof(image_path), path, image);
        }

        R2D_TilemapReadInt(tileset_begin, tileset_end, "columns", &columns);
        R2D_TilemapReadInt(tileset_begin, tileset_end, "tilecount", &tile_count);
        R2D_TilemapReadInt(tileset_begin, tileset_end, "tilewidth", &tile_width);
        R2D_TilemapReadInt(tileset_begin, tileset_end, "tileheight", &tile_height);
        R2D_TilemapReadInt(tileset_begin, tileset_end, "margin", &margin);
        R2D_TilemapReadInt(tileset_begin, tileset_end, "spacing", &spacing);

        R2D_TilemapAppendTileset(
            tilemap,
            first_gid,
            tile_width,
            tile_height,
            margin,
            spacing,
            columns,
            tile_count,
            image_path,
            &appended_tileset
        );

        if (appended_tileset != 0) {
            if (external_tileset_text != 0) {
                R2D_TilemapParseTsxTileAnimations(appended_tileset, external_tileset_text);
            } else {
                R2D_TilemapParseJsonTileAnimations(appended_tileset, tileset_begin, tileset_end);
            }
        }

        if (external_tileset_text != 0) {
            R2D_UnloadAssetText(external_tileset_text);
        }
        tileset_begin = tileset_end + 1;
    }

    return tilemap->tileset_count > 0;
}

static bool R2D_TilemapParseLayer(const char *layer_begin, const char *layer_end, R2D_TilemapLayer *layer)
{
    char type[32];

    if (!R2D_TilemapReadTopLevelString(layer_begin, layer_end, "type", type, sizeof(type)) ||
        strcmp(type, "tilelayer") != 0) {
        return false;
    }

    R2D_TilemapReadTopLevelString(layer_begin, layer_end, "name", layer->name, sizeof(layer->name));

    if (!R2D_TilemapReadInt(layer_begin, layer_end, "width", &layer->width) ||
        !R2D_TilemapReadInt(layer_begin, layer_end, "height", &layer->height)) {
        return false;
    }

    layer->visible = R2D_TilemapReadBool(layer_begin, layer_end, "visible", true);
    layer->opacity = 1.0f;
    layer->parallax_x = 1.0f;
    layer->parallax_y = 1.0f;
    R2D_TilemapReadFloat(layer_begin, layer_end, "opacity", &layer->opacity);
    R2D_TilemapReadFloat(layer_begin, layer_end, "offsetx", &layer->offset_x);
    R2D_TilemapReadFloat(layer_begin, layer_end, "offsety", &layer->offset_y);
    R2D_TilemapReadFloat(layer_begin, layer_end, "parallaxx", &layer->parallax_x);
    R2D_TilemapReadFloat(layer_begin, layer_end, "parallaxy", &layer->parallax_y);
    if (!R2D_TilemapParseProperties(layer_begin, layer_end, layer->properties, &layer->property_count)) {
        return false;
    }

    return R2D_TilemapParseTileData(layer_begin, layer_end, layer);
}

static int R2D_TilemapCountTileLayers(const char *layers, const char *layers_end)
{
    const char *cursor = layers;
    int count = 0;

    while ((cursor = R2D_TilemapFindFirstObjectInArray(cursor, layers_end)) != 0) {
        const char *object_end = R2D_TilemapFindMatching(cursor, '{', '}');
        char type[32];

        if (object_end == 0 || object_end > layers_end) {
            break;
        }

        if (R2D_TilemapReadTopLevelString(cursor, object_end, "type", type, sizeof(type)) &&
            strcmp(type, "tilelayer") == 0) {
            ++count;
        }

        cursor = object_end + 1;
    }

    return count;
}

static bool R2D_TilemapParseLayers(R2D_Tilemap *tilemap, const char *text)
{
    const char *layers = R2D_TilemapFindKey(text, 0, "layers");
    const char *layers_end;
    const char *cursor;
    int layer_index = 0;

    if (layers == 0 || *layers != '[') {
        return false;
    }

    layers_end = R2D_TilemapFindMatching(layers, '[', ']');
    if (layers_end == 0) {
        return false;
    }

    tilemap->layer_count = R2D_TilemapCountTileLayers(layers, layers_end);
    if (tilemap->layer_count <= 0) {
        return false;
    }

    tilemap->layers = (R2D_TilemapLayer *)calloc((size_t)tilemap->layer_count, sizeof(R2D_TilemapLayer));
    if (tilemap->layers == 0) {
        return false;
    }

    cursor = layers;
    while ((cursor = R2D_TilemapFindFirstObjectInArray(cursor, layers_end)) != 0 && layer_index < tilemap->layer_count) {
        const char *object_end = R2D_TilemapFindMatching(cursor, '{', '}');
        char type[32];

        if (object_end == 0 || object_end > layers_end) {
            break;
        }

        if (R2D_TilemapReadTopLevelString(cursor, object_end, "type", type, sizeof(type)) &&
            strcmp(type, "tilelayer") == 0) {
            if (!R2D_TilemapParseLayer(cursor, object_end, &tilemap->layers[layer_index])) {
                return false;
            }

            ++layer_index;
        }

        cursor = object_end + 1;
    }

    return layer_index == tilemap->layer_count;
}

static int R2D_TilemapCountObjectsInLayer(const char *layer_begin, const char *layer_end)
{
    const char *objects = R2D_TilemapFindKey(layer_begin, layer_end, "objects");
    const char *objects_end;
    const char *cursor;
    int count = 0;

    if (objects == 0 || *objects != '[') {
        return 0;
    }

    objects_end = R2D_TilemapFindMatching(objects, '[', ']');
    if (objects_end == 0 || objects_end > layer_end) {
        return 0;
    }

    cursor = objects;
    while ((cursor = R2D_TilemapFindFirstObjectInArray(cursor, objects_end)) != 0) {
        const char *object_end = R2D_TilemapFindMatching(cursor, '{', '}');

        if (object_end == 0 || object_end > objects_end) {
            break;
        }

        ++count;
        cursor = object_end + 1;
    }

    return count;
}

static int R2D_TilemapCountObjects(const char *layers, const char *layers_end)
{
    const char *cursor = layers;
    int count = 0;

    while ((cursor = R2D_TilemapFindFirstObjectInArray(cursor, layers_end)) != 0) {
        const char *object_end = R2D_TilemapFindMatching(cursor, '{', '}');
        char type[32];

        if (object_end == 0 || object_end > layers_end) {
            break;
        }

        if (R2D_TilemapReadTopLevelString(cursor, object_end, "type", type, sizeof(type)) &&
            strcmp(type, "objectgroup") == 0) {
            count += R2D_TilemapCountObjectsInLayer(cursor, object_end);
        }

        cursor = object_end + 1;
    }

    return count;
}

static bool R2D_TilemapParseObject(
    const char *object_begin,
    const char *object_end,
    R2D_TilemapObject *object
)
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    if (object == 0 ||
        !R2D_TilemapReadFloat(object_begin, object_end, "x", &x) ||
        !R2D_TilemapReadFloat(object_begin, object_end, "y", &y)) {
        return false;
    }

    R2D_TilemapReadTopLevelString(object_begin, object_end, "name", object->name, sizeof(object->name));
    R2D_TilemapReadTopLevelString(object_begin, object_end, "type", object->type, sizeof(object->type));
    R2D_TilemapReadFloat(object_begin, object_end, "width", &width);
    R2D_TilemapReadFloat(object_begin, object_end, "height", &height);
    if (!R2D_TilemapParseProperties(object_begin, object_end, object->properties, &object->property_count)) {
        return false;
    }

    object->rect = (Rectangle) { x, y, width, height };
    return true;
}

static bool R2D_TilemapParseObjectsInLayer(
    R2D_Tilemap *tilemap,
    const char *layer_begin,
    const char *layer_end,
    int *object_index
)
{
    const char *objects = R2D_TilemapFindKey(layer_begin, layer_end, "objects");
    const char *objects_end;
    const char *cursor;

    if (objects == 0 || *objects != '[') {
        return true;
    }

    objects_end = R2D_TilemapFindMatching(objects, '[', ']');
    if (objects_end == 0 || objects_end > layer_end) {
        return false;
    }

    cursor = objects;
    while ((cursor = R2D_TilemapFindFirstObjectInArray(cursor, objects_end)) != 0) {
        const char *object_end = R2D_TilemapFindMatching(cursor, '{', '}');

        if (object_end == 0 || object_end > objects_end || *object_index >= tilemap->object_count) {
            return false;
        }

        if (!R2D_TilemapParseObject(cursor, object_end, &tilemap->objects[*object_index])) {
            return false;
        }

        ++(*object_index);
        cursor = object_end + 1;
    }

    return true;
}

static bool R2D_TilemapParseObjects(R2D_Tilemap *tilemap, const char *text)
{
    const char *layers = R2D_TilemapFindKey(text, 0, "layers");
    const char *layers_end;
    const char *cursor;
    int object_index = 0;

    if (layers == 0 || *layers != '[') {
        return false;
    }

    layers_end = R2D_TilemapFindMatching(layers, '[', ']');
    if (layers_end == 0) {
        return false;
    }

    tilemap->object_count = R2D_TilemapCountObjects(layers, layers_end);
    if (tilemap->object_count <= 0) {
        return true;
    }

    tilemap->objects = (R2D_TilemapObject *)calloc((size_t)tilemap->object_count, sizeof(R2D_TilemapObject));
    if (tilemap->objects == 0) {
        return false;
    }

    cursor = layers;
    while ((cursor = R2D_TilemapFindFirstObjectInArray(cursor, layers_end)) != 0) {
        const char *object_end = R2D_TilemapFindMatching(cursor, '{', '}');
        char type[32];

        if (object_end == 0 || object_end > layers_end) {
            return false;
        }

        if (R2D_TilemapReadTopLevelString(cursor, object_end, "type", type, sizeof(type)) &&
            strcmp(type, "objectgroup") == 0 &&
            !R2D_TilemapParseObjectsInLayer(tilemap, cursor, object_end, &object_index)) {
            return false;
        }

        cursor = object_end + 1;
    }

    return object_index == tilemap->object_count;
}

bool R2D_TilemapLoadTiledJson(R2D_Tilemap *tilemap, const char *path)
{
    char *text;
    char orientation[32];
    bool ok;

    if (tilemap == 0 || path == 0) {
        return false;
    }

    if (tilemap->is_ready) {
        R2D_TilemapUnload(tilemap);
    }

    memset(tilemap, 0, sizeof(*tilemap));
    text = R2D_LoadAssetText(path);

    if (text == 0) {
        TraceLog(LOG_WARNING, "R2D: Failed to load tilemap: %s", path);
        return false;
    }

    ok = R2D_TilemapReadString(text, 0, "orientation", orientation, sizeof(orientation)) &&
        strcmp(orientation, "orthogonal") == 0 &&
        !R2D_TilemapReadBool(text, 0, "infinite", false) &&
        R2D_TilemapReadInt(text, 0, "width", &tilemap->width) &&
        R2D_TilemapReadInt(text, 0, "height", &tilemap->height) &&
        R2D_TilemapReadInt(text, 0, "tilewidth", &tilemap->tile_width) &&
        R2D_TilemapReadInt(text, 0, "tileheight", &tilemap->tile_height) &&
        R2D_TilemapParseTileset(tilemap, text, path) &&
        R2D_TilemapParseLayers(tilemap, text) &&
        R2D_TilemapParseObjects(tilemap, text);

    if (ok) {
        tilemap->is_ready = true;
        TraceLog(LOG_INFO, "R2D: Tiled tilemap loaded: %s", path);
    } else {
        TraceLog(LOG_WARNING, "R2D: Unsupported or invalid Tiled JSON tilemap: %s", path);
        R2D_TilemapUnload(tilemap);
    }

    R2D_UnloadAssetText(text);
    return ok;
}

void R2D_TilemapUnload(R2D_Tilemap *tilemap)
{
    if (tilemap == 0) {
        return;
    }

    if (tilemap->layers != 0) {
        for (int i = 0; i < tilemap->layer_count; ++i) {
            free(tilemap->layers[i].tiles);
        }

        free(tilemap->layers);
    }

    if (tilemap->objects != 0) {
        free(tilemap->objects);
    }

    if (tilemap->tilesets != 0) {
        for (int i = 0; i < tilemap->tileset_count; ++i) {
            if (tilemap->tilesets[i].animations != 0) {
                for (int j = 0; j < tilemap->tilesets[i].animation_count; ++j) {
                    free(tilemap->tilesets[i].animations[j].frames);
                }

                free(tilemap->tilesets[i].animations);
            }

            if (IsTextureValid(tilemap->tilesets[i].texture)) {
                UnloadTexture(tilemap->tilesets[i].texture);
            }
        }

        free(tilemap->tilesets);
    }

    memset(tilemap, 0, sizeof(*tilemap));
}

bool R2D_TilemapIsReady(const R2D_Tilemap *tilemap)
{
    return tilemap != 0 &&
        tilemap->is_ready &&
        tilemap->tilesets != 0 &&
        tilemap->tileset_count > 0;
}

int R2D_TilemapLayerIndex(const R2D_Tilemap *tilemap, const char *name)
{
    if (tilemap == 0 || name == 0) {
        return -1;
    }

    for (int i = 0; i < tilemap->layer_count; ++i) {
        if (strcmp(tilemap->layers[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

unsigned int R2D_TilemapTileAt(const R2D_Tilemap *tilemap, int layer_index, int x, int y)
{
    const R2D_TilemapLayer *layer;

    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return 0;
    }

    layer = &tilemap->layers[layer_index];
    if (x < 0 || y < 0 || x >= layer->width || y >= layer->height || layer->tiles == 0) {
        return 0;
    }

    return layer->tiles[y * layer->width + x];
}

int R2D_TilemapLayerPropertyCount(const R2D_Tilemap *tilemap, int layer_index)
{
    if (tilemap == 0 || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return 0;
    }

    return tilemap->layers[layer_index].property_count;
}

const R2D_TilemapProperty *R2D_TilemapLayerPropertyAt(
    const R2D_Tilemap *tilemap,
    int layer_index,
    int property_index
)
{
    const R2D_TilemapLayer *layer;

    if (tilemap == 0 || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return 0;
    }

    layer = &tilemap->layers[layer_index];
    if (property_index < 0 || property_index >= layer->property_count) {
        return 0;
    }

    return &layer->properties[property_index];
}

const R2D_TilemapProperty *R2D_TilemapLayerFindProperty(
    const R2D_Tilemap *tilemap,
    int layer_index,
    const char *name
)
{
    const R2D_TilemapLayer *layer;

    if (tilemap == 0 || name == 0 || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return 0;
    }

    layer = &tilemap->layers[layer_index];
    for (int i = 0; i < layer->property_count; ++i) {
        if (strcmp(layer->properties[i].name, name) == 0) {
            return &layer->properties[i];
        }
    }

    return 0;
}

const R2D_TilemapProperty *R2D_TilemapObjectFindProperty(const R2D_TilemapObject *object, const char *name)
{
    if (object == 0 || name == 0) {
        return 0;
    }

    for (int i = 0; i < object->property_count; ++i) {
        if (strcmp(object->properties[i].name, name) == 0) {
            return &object->properties[i];
        }
    }

    return 0;
}

const char *R2D_TilemapPropertyString(const R2D_TilemapProperty *property, const char *fallback)
{
    if (property == 0 || property->type != R2D_TILEMAP_PROPERTY_STRING) {
        return fallback;
    }

    return property->string_value;
}

int R2D_TilemapPropertyInt(const R2D_TilemapProperty *property, int fallback)
{
    if (property == 0) {
        return fallback;
    }

    if (property->type == R2D_TILEMAP_PROPERTY_INT || property->type == R2D_TILEMAP_PROPERTY_FLOAT ||
        property->type == R2D_TILEMAP_PROPERTY_BOOL) {
        return property->int_value;
    }

    return fallback;
}

float R2D_TilemapPropertyFloat(const R2D_TilemapProperty *property, float fallback)
{
    if (property == 0) {
        return fallback;
    }

    if (property->type == R2D_TILEMAP_PROPERTY_FLOAT || property->type == R2D_TILEMAP_PROPERTY_INT ||
        property->type == R2D_TILEMAP_PROPERTY_BOOL) {
        return property->float_value;
    }

    return fallback;
}

bool R2D_TilemapPropertyBool(const R2D_TilemapProperty *property, bool fallback)
{
    if (property == 0) {
        return fallback;
    }

    if (property->type == R2D_TILEMAP_PROPERTY_BOOL || property->type == R2D_TILEMAP_PROPERTY_INT ||
        property->type == R2D_TILEMAP_PROPERTY_FLOAT) {
        return property->bool_value;
    }

    return fallback;
}

Color R2D_TilemapPropertyColor(const R2D_TilemapProperty *property, Color fallback)
{
    if (property == 0 || property->type != R2D_TILEMAP_PROPERTY_COLOR) {
        return fallback;
    }

    return property->color_value;
}

Vector2 R2D_TilemapWorldToTile(const R2D_Tilemap *tilemap, Vector2 position)
{
    if (!R2D_TilemapIsReady(tilemap) || tilemap->tile_width <= 0 || tilemap->tile_height <= 0) {
        return (Vector2) { 0.0f, 0.0f };
    }

    return (Vector2) {
        floorf(position.x / (float)tilemap->tile_width),
        floorf(position.y / (float)tilemap->tile_height)
    };
}

Rectangle R2D_TilemapTileBounds(const R2D_Tilemap *tilemap, int x, int y)
{
    if (!R2D_TilemapIsReady(tilemap)) {
        return (Rectangle) { 0 };
    }

    return (Rectangle) {
        (float)(x * tilemap->tile_width),
        (float)(y * tilemap->tile_height),
        (float)tilemap->tile_width,
        (float)tilemap->tile_height
    };
}

bool R2D_TilemapSolidAt(const R2D_Tilemap *tilemap, int layer_index, Vector2 position)
{
    const Vector2 tile = R2D_TilemapWorldToTile(tilemap, position);

    return R2D_TilemapTileAt(tilemap, layer_index, (int)tile.x, (int)tile.y) != 0;
}

bool R2D_TilemapRectCollides(const R2D_Tilemap *tilemap, int layer_index, Rectangle rect)
{
    int left;
    int right;
    int top;
    int bottom;

    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || rect.width <= 0.0f || rect.height <= 0.0f) {
        return false;
    }

    left = (int)floorf(rect.x / (float)tilemap->tile_width);
    right = (int)floorf((rect.x + rect.width - 0.001f) / (float)tilemap->tile_width);
    top = (int)floorf(rect.y / (float)tilemap->tile_height);
    bottom = (int)floorf((rect.y + rect.height - 0.001f) / (float)tilemap->tile_height);

    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            if (R2D_TilemapTileAt(tilemap, layer_index, x, y) != 0) {
                return true;
            }
        }
    }

    return false;
}

bool R2D_TilemapGridBlocked(const R2D_Tilemap *tilemap, int layer_index, int x, int y)
{
    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return true;
    }

    if (x < 0 || y < 0 || x >= tilemap->width || y >= tilemap->height) {
        return true;
    }

    return R2D_TilemapTileAt(tilemap, layer_index, x, y) != 0;
}

typedef struct R2D_TilemapGridQuery {
    const R2D_Tilemap *tilemap;
    int layer_index;
} R2D_TilemapGridQuery;

static bool R2D_TilemapGridBlockedCallback(int x, int y, void *user_data)
{
    const R2D_TilemapGridQuery *query = (const R2D_TilemapGridQuery *)user_data;

    return query == 0 || R2D_TilemapGridBlocked(query->tilemap, query->layer_index, x, y);
}

int R2D_TilemapFindPath(
    const R2D_Tilemap *tilemap,
    int layer_index,
    R2D_GridPoint start,
    R2D_GridPoint goal,
    R2D_GridPoint *out_path,
    int max_path
)
{
    R2D_TilemapGridQuery query = { tilemap, layer_index };

    if (!R2D_TilemapIsReady(tilemap)) {
        return 0;
    }

    return R2D_GridAStar(
        start,
        goal,
        tilemap->width,
        tilemap->height,
        R2D_TilemapGridBlockedCallback,
        &query,
        out_path,
        max_path
    );
}

int R2D_TilemapFloodFill(
    const R2D_Tilemap *tilemap,
    int layer_index,
    R2D_GridPoint start,
    R2D_GridPoint *out_points,
    int max_points
)
{
    R2D_TilemapGridQuery query = { tilemap, layer_index };

    if (!R2D_TilemapIsReady(tilemap)) {
        return 0;
    }

    return R2D_GridFloodFill(
        start,
        tilemap->width,
        tilemap->height,
        R2D_TilemapGridBlockedCallback,
        &query,
        out_points,
        max_points
    );
}

bool R2D_TilemapLineOfSight(const R2D_Tilemap *tilemap, int layer_index, R2D_GridPoint start, R2D_GridPoint end)
{
    R2D_TilemapGridQuery query = { tilemap, layer_index };

    if (!R2D_TilemapIsReady(tilemap)) {
        return false;
    }

    return R2D_GridLineOfSight(start, end, R2D_TilemapGridBlockedCallback, &query);
}

int R2D_TilemapCollisionRects(
    const R2D_Tilemap *tilemap,
    int layer_index,
    Rectangle area,
    R2D_Collider *colliders,
    int max_colliders,
    unsigned int layer,
    unsigned int mask
)
{
    int left;
    int right;
    int top;
    int bottom;
    int count = 0;

    if (!R2D_TilemapIsReady(tilemap) ||
        layer_index < 0 ||
        layer_index >= tilemap->layer_count ||
        colliders == 0 ||
        max_colliders <= 0 ||
        area.width <= 0.0f ||
        area.height <= 0.0f) {
        return 0;
    }

    left = (int)floorf(area.x / (float)tilemap->tile_width);
    right = (int)floorf((area.x + area.width - 0.001f) / (float)tilemap->tile_width);
    top = (int)floorf(area.y / (float)tilemap->tile_height);
    bottom = (int)floorf((area.y + area.height - 0.001f) / (float)tilemap->tile_height);

    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            if (R2D_TilemapTileAt(tilemap, layer_index, x, y) == 0) {
                continue;
            }

            colliders[count++] = R2D_ColliderRect(
                R2D_TilemapTileBounds(tilemap, x, y),
                layer,
                mask,
                false,
                0
            );

            if (count >= max_colliders) {
                return count;
            }
        }
    }

    return count;
}

Vector2 R2D_TilemapMoveAndSlide(
    const R2D_Tilemap *tilemap,
    int layer_index,
    Rectangle bounds,
    Vector2 movement,
    R2D_CollisionResult *result
)
{
    R2D_Collider colliders[128];
    Rectangle sweep = bounds;
    int collider_count;

    if (movement.x < 0.0f) {
        sweep.x += movement.x;
        sweep.width -= movement.x;
    } else {
        sweep.width += movement.x;
    }

    if (movement.y < 0.0f) {
        sweep.y += movement.y;
        sweep.height -= movement.y;
    } else {
        sweep.height += movement.y;
    }

    collider_count = R2D_TilemapCollisionRects(
        tilemap,
        layer_index,
        sweep,
        colliders,
        (int)(sizeof(colliders) / sizeof(colliders[0])),
        1u,
        1u
    );

    return R2D_MoveAndSlide(bounds, movement, 1u, 1u, colliders, collider_count, result);
}

int R2D_TilemapObjectCount(const R2D_Tilemap *tilemap)
{
    return tilemap != 0 ? tilemap->object_count : 0;
}

const R2D_TilemapObject *R2D_TilemapObjectAt(const R2D_Tilemap *tilemap, int index)
{
    if (tilemap == 0 || index < 0 || index >= tilemap->object_count) {
        return 0;
    }

    return &tilemap->objects[index];
}

const R2D_TilemapObject *R2D_TilemapFindObject(const R2D_Tilemap *tilemap, const char *name)
{
    if (tilemap == 0 || name == 0) {
        return 0;
    }

    for (int i = 0; i < tilemap->object_count; ++i) {
        if (strcmp(tilemap->objects[i].name, name) == 0) {
            return &tilemap->objects[i];
        }
    }

    return 0;
}

const R2D_TilemapObject *R2D_TilemapFindObjectByType(const R2D_Tilemap *tilemap, const char *type)
{
    if (tilemap == 0 || type == 0) {
        return 0;
    }

    for (int i = 0; i < tilemap->object_count; ++i) {
        if (strcmp(tilemap->objects[i].type, type) == 0) {
            return &tilemap->objects[i];
        }
    }

    return 0;
}

bool R2D_TilemapObjectIsTrigger(const R2D_TilemapObject *object)
{
    const R2D_TilemapProperty *trigger_property;

    if (object == 0) {
        return false;
    }

    if (strcmp(object->type, "trigger") == 0 || strcmp(object->type, "sensor") == 0) {
        return true;
    }

    trigger_property = R2D_TilemapObjectFindProperty(object, "trigger");
    return R2D_TilemapPropertyBool(trigger_property, false);
}

int R2D_TilemapTriggerColliders(
    const R2D_Tilemap *tilemap,
    R2D_Collider *colliders,
    int max_colliders,
    unsigned int layer,
    unsigned int mask
)
{
    int count = 0;

    if (tilemap == 0 || colliders == 0 || max_colliders <= 0) {
        return 0;
    }

    for (int i = 0; i < tilemap->object_count && count < max_colliders; ++i) {
        const R2D_TilemapObject *object = &tilemap->objects[i];

        if (!R2D_TilemapObjectIsTrigger(object) || object->rect.width <= 0.0f || object->rect.height <= 0.0f) {
            continue;
        }

        colliders[count++] = R2D_ColliderRect(object->rect, layer, mask, true, (void *)object);
    }

    return count;
}

static void R2D_TilemapVisibleRange(
    const R2D_Tilemap *tilemap,
    const R2D_TilemapLayer *layer,
    Rectangle view,
    int *start_x,
    int *start_y,
    int *end_x,
    int *end_y
)
{
    int left = (int)floorf(view.x / (float)tilemap->tile_width) - 1;
    int top = (int)floorf(view.y / (float)tilemap->tile_height) - 1;
    int right = (int)floorf((view.x + view.width) / (float)tilemap->tile_width) + 1;
    int bottom = (int)floorf((view.y + view.height) / (float)tilemap->tile_height) + 1;

    if (left < 0) {
        left = 0;
    }

    if (top < 0) {
        top = 0;
    }

    if (right >= layer->width) {
        right = layer->width - 1;
    }

    if (bottom >= layer->height) {
        bottom = layer->height - 1;
    }

    *start_x = left;
    *start_y = top;
    *end_x = right;
    *end_y = bottom;
}

static const R2D_TilemapTileset *R2D_TilemapTilesetForGid(const R2D_Tilemap *tilemap, unsigned int gid)
{
    const R2D_TilemapTileset *match = 0;

    if (tilemap == 0 || gid == 0) {
        return 0;
    }

    for (int i = 0; i < tilemap->tileset_count; ++i) {
        const R2D_TilemapTileset *tileset = &tilemap->tilesets[i];
        const int tile = (int)gid - tileset->first_gid;

        if (tile >= 0 && tile < tileset->tile_count) {
            match = tileset;
        }
    }

    return match;
}

static int R2D_TilemapAnimatedTile(const R2D_TilemapTileset *tileset, int tile)
{
    int time_ms;

    if (tileset == 0 || tile < 0 || tileset->animation_count <= 0) {
        return tile;
    }

    for (int i = 0; i < tileset->animation_count; ++i) {
        const R2D_TilemapTileAnimation *animation = &tileset->animations[i];
        int cursor = 0;

        if (animation->tile_id != tile || animation->frame_count <= 0 || animation->duration_ms <= 0) {
            continue;
        }

        time_ms = (int)(GetTime() * 1000.0) % animation->duration_ms;
        for (int frame = 0; frame < animation->frame_count; ++frame) {
            const int duration = animation->frames[frame].duration_ms > 0 ? animation->frames[frame].duration_ms : 1;

            cursor += duration;
            if (time_ms < cursor) {
                return animation->frames[frame].tile_id;
            }
        }

        return animation->frames[animation->frame_count - 1].tile_id;
    }

    return tile;
}

static void R2D_TilemapDrawLayerRange(
    const R2D_Tilemap *tilemap,
    int layer_index,
    int start_x,
    int start_y,
    int end_x,
    int end_y,
    Vector2 position
)
{
    const R2D_TilemapLayer *layer;
    Color tint = WHITE;

    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return;
    }

    layer = &tilemap->layers[layer_index];
    if (!layer->visible || layer->tiles == 0) {
        return;
    }

    if (start_x < 0) {
        start_x = 0;
    }

    if (start_y < 0) {
        start_y = 0;
    }

    if (end_x >= layer->width) {
        end_x = layer->width - 1;
    }

    if (end_y >= layer->height) {
        end_y = layer->height - 1;
    }

    if (start_x > end_x || start_y > end_y) {
        return;
    }

    position.x += layer->offset_x;
    position.y += layer->offset_y;
    tint.a = (unsigned char)(R2D_Clamp01(layer->opacity) * 255.0f);

    for (int y = start_y; y <= end_y; ++y) {
        for (int x = start_x; x <= end_x; ++x) {
            const unsigned int raw_gid = layer->tiles[y * layer->width + x];
            const unsigned int gid = raw_gid & R2D_TILED_GID_MASK;
            const R2D_TilemapTileset *tileset = R2D_TilemapTilesetForGid(tilemap, gid);
            int tile;
            Rectangle source;
            Rectangle destination;

            if (tileset == 0) {
                continue;
            }

            tile = R2D_TilemapAnimatedTile(tileset, (int)gid - tileset->first_gid);
            source = (Rectangle) {
                (float)(tileset->margin + (tile % tileset->columns) * (tileset->tile_width + tileset->spacing)),
                (float)(tileset->margin + (tile / tileset->columns) * (tileset->tile_height + tileset->spacing)),
                (float)tileset->tile_width,
                (float)tileset->tile_height
            };

            if ((raw_gid & R2D_TILED_FLIP_HORIZONTAL) != 0) {
                source.x += source.width;
                source.width *= -1.0f;
            }

            if ((raw_gid & R2D_TILED_FLIP_VERTICAL) != 0) {
                source.y += source.height;
                source.height *= -1.0f;
            }

            destination = (Rectangle) {
                position.x + (float)(x * tilemap->tile_width),
                position.y + (float)(y * tilemap->tile_height),
                (float)tilemap->tile_width,
                (float)tilemap->tile_height
            };

            DrawTexturePro(
                tileset->texture,
                source,
                destination,
                (Vector2) { 0.0f, 0.0f },
                0.0f,
                tint
            );
        }
    }
}

void R2D_TilemapDraw(const R2D_Tilemap *tilemap, Vector2 position)
{
    if (!R2D_TilemapIsReady(tilemap)) {
        return;
    }

    for (int i = 0; i < tilemap->layer_count; ++i) {
        R2D_TilemapDrawLayer(tilemap, i, position);
    }
}

void R2D_TilemapDrawLayer(const R2D_Tilemap *tilemap, int layer_index, Vector2 position)
{
    const R2D_TilemapLayer *layer;

    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return;
    }

    layer = &tilemap->layers[layer_index];
    R2D_TilemapDrawLayerRange(tilemap, layer_index, 0, 0, layer->width - 1, layer->height - 1, position);
}

void R2D_TilemapDrawVisible(const R2D_Tilemap *tilemap, Rectangle view, Vector2 position)
{
    if (!R2D_TilemapIsReady(tilemap)) {
        return;
    }

    for (int i = 0; i < tilemap->layer_count; ++i) {
        R2D_TilemapDrawLayerVisible(tilemap, i, view, position);
    }
}

void R2D_TilemapDrawLayerVisible(const R2D_Tilemap *tilemap, int layer_index, Rectangle view, Vector2 position)
{
    const R2D_TilemapLayer *layer;
    int start_x;
    int start_y;
    int end_x;
    int end_y;

    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return;
    }

    layer = &tilemap->layers[layer_index];
    R2D_TilemapVisibleRange(tilemap, layer, view, &start_x, &start_y, &end_x, &end_y);
    R2D_TilemapDrawLayerRange(tilemap, layer_index, start_x, start_y, end_x, end_y, position);
}

void R2D_TilemapDrawLayerParallax(
    const R2D_Tilemap *tilemap,
    int layer_index,
    Rectangle camera_view,
    Vector2 screen_position
)
{
    const R2D_TilemapLayer *layer;
    Rectangle layer_view;
    Vector2 layer_position;
    int start_x;
    int start_y;
    int end_x;
    int end_y;

    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return;
    }

    layer = &tilemap->layers[layer_index];
    layer_view = R2D_Rect(
        camera_view.x * layer->parallax_x - layer->offset_x,
        camera_view.y * layer->parallax_y - layer->offset_y,
        camera_view.width,
        camera_view.height
    );
    layer_position = (Vector2) {
        screen_position.x - camera_view.x * layer->parallax_x,
        screen_position.y - camera_view.y * layer->parallax_y
    };

    R2D_TilemapVisibleRange(tilemap, layer, layer_view, &start_x, &start_y, &end_x, &end_y);
    R2D_TilemapDrawLayerRange(tilemap, layer_index, start_x, start_y, end_x, end_y, layer_position);
}

void R2D_TilemapDrawCollisionDebug(const R2D_Tilemap *tilemap, int layer_index, Vector2 position, Color color)
{
    const R2D_TilemapLayer *layer;
    Color fill = color;

    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return;
    }

    layer = &tilemap->layers[layer_index];
    if (layer->tiles == 0) {
        return;
    }

    fill.a = (unsigned char)(fill.a / 4);

    for (int y = 0; y < layer->height; ++y) {
        for (int x = 0; x < layer->width; ++x) {
            Rectangle rect;

            if (layer->tiles[y * layer->width + x] == 0) {
                continue;
            }

            rect = (Rectangle) {
                position.x + (float)(x * tilemap->tile_width),
                position.y + (float)(y * tilemap->tile_height),
                (float)tilemap->tile_width,
                (float)tilemap->tile_height
            };

            DrawRectangleRec(rect, fill);
            DrawRectangleLinesEx(rect, 1.0f, color);
        }
    }
}

void R2D_TilemapDrawCollisionDebugVisible(
    const R2D_Tilemap *tilemap,
    int layer_index,
    Rectangle view,
    Vector2 position,
    Color color
)
{
    const R2D_TilemapLayer *layer;
    Color fill = color;
    int start_x;
    int start_y;
    int end_x;
    int end_y;

    if (!R2D_TilemapIsReady(tilemap) || layer_index < 0 || layer_index >= tilemap->layer_count) {
        return;
    }

    layer = &tilemap->layers[layer_index];
    if (layer->tiles == 0) {
        return;
    }

    R2D_TilemapVisibleRange(tilemap, layer, view, &start_x, &start_y, &end_x, &end_y);
    fill.a = (unsigned char)(fill.a / 4);

    for (int y = start_y; y <= end_y; ++y) {
        for (int x = start_x; x <= end_x; ++x) {
            Rectangle rect;

            if (layer->tiles[y * layer->width + x] == 0) {
                continue;
            }

            rect = (Rectangle) {
                position.x + (float)(x * tilemap->tile_width),
                position.y + (float)(y * tilemap->tile_height),
                (float)tilemap->tile_width,
                (float)tilemap->tile_height
            };

            DrawRectangleRec(rect, fill);
            DrawRectangleLinesEx(rect, 1.0f, color);
        }
    }
}

void R2D_TilemapDrawObjectsDebug(const R2D_Tilemap *tilemap, Vector2 position, Color color)
{
    Color fill = color;

    if (tilemap == 0 || tilemap->objects == 0) {
        return;
    }

    fill.a = (unsigned char)(fill.a / 5);

    for (int i = 0; i < tilemap->object_count; ++i) {
        const R2D_TilemapObject *object = &tilemap->objects[i];
        const Rectangle rect = {
            position.x + object->rect.x,
            position.y + object->rect.y,
            object->rect.width,
            object->rect.height
        };

        DrawRectangleRec(rect, fill);
        DrawRectangleLinesEx(rect, 1.0f, color);

        if (object->name[0] != '\0') {
            DrawText(object->name, (int)rect.x + 2, (int)rect.y - 9, 8, color);
        } else if (object->type[0] != '\0') {
            DrawText(object->type, (int)rect.x + 2, (int)rect.y - 9, 8, color);
        }
    }
}
