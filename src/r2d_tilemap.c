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

static unsigned int R2D_TilemapMaxGid(const char *text)
{
    const char *cursor = text;
    unsigned int max_gid = 0;

    while ((cursor = R2D_TilemapFindKey(cursor, 0, "data")) != 0) {
        const char *data_end;

        if (*cursor != '[') {
            ++cursor;
            continue;
        }

        data_end = R2D_TilemapFindMatching(cursor, '[', ']');
        if (data_end == 0) {
            break;
        }

        ++cursor;
        while (cursor < data_end) {
            char *next = 0;
            const unsigned int gid = (unsigned int)strtoul(cursor, &next, 10) & R2D_TILED_GID_MASK;

            if (next == cursor) {
                ++cursor;
                continue;
            }

            if (gid > max_gid) {
                max_gid = gid;
            }

            cursor = next;
        }
    }

    return max_gid;
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

static bool R2D_TilemapParseTileset(R2D_Tilemap *tilemap, const char *text, const char *path)
{
    const char *tilesets = R2D_TilemapFindKey(text, 0, "tilesets");
    const char *tilesets_end;
    const char *tileset_begin;
    const unsigned int max_gid = R2D_TilemapMaxGid(text);
    int chosen_first_gid = -1;
    int chosen_tile_width = 0;
    int chosen_tile_height = 0;
    int chosen_columns = 0;
    int chosen_tile_count = 0;
    char chosen_image_path[1200] = { 0 };

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
        int columns = 0;
        int tile_count = 0;
        char source[512];
        char image[512];
        char image_path[1200] = { 0 };

        if (tileset_end == 0 || tileset_end > tilesets_end) {
            break;
        }

        if (!R2D_TilemapReadInt(tileset_begin, tileset_end, "firstgid", &first_gid)) {
            tileset_begin = tileset_end + 1;
            continue;
        }

        if (first_gid <= chosen_first_gid || (max_gid != 0 && (unsigned int)first_gid > max_gid)) {
            tileset_begin = tileset_end + 1;
            continue;
        }

        if (R2D_TilemapReadString(tileset_begin, tileset_end, "source", source, sizeof(source))) {
            char tileset_path[1200];
            char *tileset_text;
            const char *tileset_tag;
            const char *tileset_tag_end;
            const char *image_tag;
            const char *image_tag_end;

            R2D_TilemapResolvePath(tileset_path, sizeof(tileset_path), path, source);
            tileset_text = LoadFileText(tileset_path);

            if (tileset_text == 0) {
                TraceLog(LOG_WARNING, "R2D: Failed to load Tiled TSX tileset: %s", tileset_path);
                tileset_begin = tileset_end + 1;
                continue;
            }

            tileset_tag = strstr(tileset_text, "<tileset");
            image_tag = strstr(tileset_text, "<image");

            if (tileset_tag == 0 || image_tag == 0) {
                UnloadFileText(tileset_text);
                tileset_begin = tileset_end + 1;
                continue;
            }

            tileset_tag_end = strchr(tileset_tag, '>');
            image_tag_end = strchr(image_tag, '>');

            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "tilewidth", &tile_width);
            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "tileheight", &tile_height);
            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "columns", &columns);
            R2D_TilemapXmlReadInt(tileset_tag, tileset_tag_end, "tilecount", &tile_count);

            if (!R2D_TilemapXmlReadString(image_tag, image_tag_end, "source", image, sizeof(image))) {
                UnloadFileText(tileset_text);
                tileset_begin = tileset_end + 1;
                continue;
            }

            R2D_TilemapResolvePath(image_path, sizeof(image_path), tileset_path, image);
            UnloadFileText(tileset_text);
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

        chosen_first_gid = first_gid;
        chosen_tile_width = tile_width;
        chosen_tile_height = tile_height;
        chosen_columns = columns;
        chosen_tile_count = tile_count;
        snprintf(chosen_image_path, sizeof(chosen_image_path), "%s", image_path);
        tileset_begin = tileset_end + 1;
    }

    if (chosen_first_gid < 0 || chosen_image_path[0] == '\0') {
        return false;
    }

    tilemap->first_gid = chosen_first_gid;
    tilemap->tile_width = chosen_tile_width;
    tilemap->tile_height = chosen_tile_height;
    tilemap->columns = chosen_columns;
    tilemap->tile_count = chosen_tile_count;
    tilemap->texture = LoadTexture(chosen_image_path);

    if (!IsTextureValid(tilemap->texture)) {
        TraceLog(LOG_WARNING, "R2D: Failed to load tilemap tileset image: %s", chosen_image_path);
        return false;
    }

    SetTextureFilter(tilemap->texture, TEXTURE_FILTER_POINT);

    if (tilemap->columns <= 0 && tilemap->tile_width > 0) {
        tilemap->columns = tilemap->texture.width / tilemap->tile_width;
    }

    if (tilemap->tile_count <= 0 && tilemap->tile_width > 0 && tilemap->tile_height > 0) {
        tilemap->tile_count = (tilemap->texture.width / tilemap->tile_width) *
            (tilemap->texture.height / tilemap->tile_height);
    }

    return tilemap->columns > 0 && tilemap->tile_count > 0;
}

static bool R2D_TilemapParseLayer(const char *layer_begin, const char *layer_end, R2D_TilemapLayer *layer)
{
    char type[32];

    if (!R2D_TilemapReadString(layer_begin, layer_end, "type", type, sizeof(type)) ||
        strcmp(type, "tilelayer") != 0) {
        return false;
    }

    R2D_TilemapReadString(layer_begin, layer_end, "name", layer->name, sizeof(layer->name));

    if (!R2D_TilemapReadInt(layer_begin, layer_end, "width", &layer->width) ||
        !R2D_TilemapReadInt(layer_begin, layer_end, "height", &layer->height)) {
        return false;
    }

    layer->visible = R2D_TilemapReadBool(layer_begin, layer_end, "visible", true);
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

    R2D_TilemapReadString(object_begin, object_end, "name", object->name, sizeof(object->name));
    R2D_TilemapReadString(object_begin, object_end, "type", object->type, sizeof(object->type));
    R2D_TilemapReadFloat(object_begin, object_end, "width", &width);
    R2D_TilemapReadFloat(object_begin, object_end, "height", &height);

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
    text = LoadFileText(path);

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

    UnloadFileText(text);
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

    if (IsTextureValid(tilemap->texture)) {
        UnloadTexture(tilemap->texture);
    }

    memset(tilemap, 0, sizeof(*tilemap));
}

bool R2D_TilemapIsReady(const R2D_Tilemap *tilemap)
{
    return tilemap != 0 && tilemap->is_ready && IsTextureValid(tilemap->texture);
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

    for (int y = start_y; y <= end_y; ++y) {
        for (int x = start_x; x <= end_x; ++x) {
            const unsigned int raw_gid = layer->tiles[y * layer->width + x];
            const unsigned int gid = raw_gid & R2D_TILED_GID_MASK;
            const int tile = (int)gid - tilemap->first_gid;
            Rectangle source;
            Rectangle destination;

            if (gid == 0 || tile < 0 || tile >= tilemap->tile_count) {
                continue;
            }

            source = (Rectangle) {
                (float)((tile % tilemap->columns) * tilemap->tile_width),
                (float)((tile / tilemap->columns) * tilemap->tile_height),
                (float)tilemap->tile_width,
                (float)tilemap->tile_height
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
                tilemap->texture,
                source,
                destination,
                (Vector2) { 0.0f, 0.0f },
                0.0f,
                WHITE
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
