#include "r2d/r2d.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R2D_ASSET_PACK_MAGIC "R2DAS01"
#define R2D_ASSET_PACK_MAGIC_SIZE 8
#define R2D_ASSET_PACK_NAME "assets.assets"

typedef struct R2D_AssetPackEntry {
    char *path;
    uint64_t offset;
    uint64_t size;
} R2D_AssetPackEntry;

typedef struct R2D_AssetPack {
    FILE *file;
    R2D_AssetPackEntry *entries;
    uint32_t count;
    bool mounted;
} R2D_AssetPack;

static R2D_AssetPack r2d_asset_pack = { 0 };

static bool R2D_AssetPathIsAbsolute(const char *path)
{
    if (path == 0 || path[0] == '\0') {
        return false;
    }

    if (path[0] == '/' || path[0] == '\\') {
        return true;
    }

    return ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) &&
        path[1] == ':';
}

static bool R2D_AssetStartsWithInsensitive(const char *text, const char *prefix)
{
    while (*prefix != '\0') {
        char a = *text;
        char b = *prefix;

        if (a >= 'A' && a <= 'Z') {
            a = (char)(a + 32);
        }

        if (b >= 'A' && b <= 'Z') {
            b = (char)(b + 32);
        }

        if (a != b) {
            return false;
        }

        ++text;
        ++prefix;
    }

    return true;
}

static void R2D_AssetNormalize(char *destination, int destination_size, const char *path)
{
    char temp[1200];
    char collapsed[1200] = "";
    const char *relative = path;
    int length;
    char *token;
    char *next;

    if (destination_size <= 0) {
        return;
    }

    destination[0] = '\0';

    if (path == 0) {
        return;
    }

#ifdef R2D_DEVELOPMENT_ASSET_DIR
    if (R2D_AssetStartsWithInsensitive(path, R2D_DEVELOPMENT_ASSET_DIR)) {
        relative = path + strlen(R2D_DEVELOPMENT_ASSET_DIR);
    }
#endif

    if (relative == path && R2D_AssetPathIsAbsolute(path)) {
        char app_assets[1024];
        snprintf(app_assets, sizeof(app_assets), "%sassets/", GetApplicationDirectory());

        if (R2D_AssetStartsWithInsensitive(path, app_assets)) {
            relative = path + strlen(app_assets);
        }
    }

    if (strncmp(relative, "assets/", 7) == 0 || strncmp(relative, "assets\\", 7) == 0) {
        relative += 7;
    }

    snprintf(temp, sizeof(temp), "%s", relative);
    length = (int)strlen(temp);

    while (length > 0 && (temp[0] == '/' || temp[0] == '\\')) {
        memmove(temp, temp + 1, (size_t)length);
        --length;
    }

    for (int i = 0; temp[i] != '\0'; ++i) {
        if (temp[i] == '\\') {
            temp[i] = '/';
        }
    }

    token = temp;
    while (*token == '/') {
        ++token;
    }

    while (*token != '\0') {
        next = strchr(token, '/');
        if (next != 0) {
            *next = '\0';
        }

        if (strcmp(token, ".") == 0 || token[0] == '\0') {
        } else if (strcmp(token, "..") == 0) {
            char *slash = strrchr(collapsed, '/');

            if (slash != 0) {
                *slash = '\0';
            } else if (collapsed[0] != '\0') {
                collapsed[0] = '\0';
            }
        } else {
            const size_t used = strlen(collapsed);

            if (collapsed[0] != '\0') {
                snprintf(collapsed + used, sizeof(collapsed) - used, "/");
            }

            {
                const size_t next_used = strlen(collapsed);
                snprintf(collapsed + next_used, sizeof(collapsed) - next_used, "%s", token);
            }
        }

        if (next == 0) {
            break;
        }

        token = next + 1;
    }

    snprintf(destination, (size_t)destination_size, "%s", collapsed);
}

static const char *R2D_AssetExtension(const char *path)
{
    const char *dot = strrchr(path, '.');

    return dot != 0 ? dot : "";
}

static bool R2D_LoadDiskFileData(const char *path, unsigned char **data, int *size)
{
    FILE *file = 0;
    long length;

#if defined(_MSC_VER)
    if (fopen_s(&file, path, "rb") != 0) {
        file = 0;
    }
#else
    file = fopen(path, "rb");
#endif

    if (file == 0) {
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    length = ftell(file);
    if (length < 0 || length > 2147483647L || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    *data = (unsigned char *)malloc((size_t)length);
    if (*data == 0 && length > 0) {
        fclose(file);
        return false;
    }

    if (length > 0 && fread(*data, 1, (size_t)length, file) != (size_t)length) {
        free(*data);
        *data = 0;
        fclose(file);
        return false;
    }

    fclose(file);
    *size = (int)length;
    return true;
}

static int R2D_AssetFindPackEntry(const char *path)
{
    char normalized[1200];

    R2D_AssetNormalize(normalized, sizeof(normalized), path);

    for (uint32_t i = 0; i < r2d_asset_pack.count; ++i) {
        if (strcmp(r2d_asset_pack.entries[i].path, normalized) == 0) {
            return (int)i;
        }
    }

    return -1;
}

bool R2D_MountAssetPack(const char *path)
{
    FILE *file = 0;
    char magic[R2D_ASSET_PACK_MAGIC_SIZE];
    uint32_t count = 0;
    R2D_AssetPackEntry *entries = 0;

    if (path == 0) {
        return false;
    }

    R2D_UnmountAssetPack();

#if defined(_MSC_VER)
    if (fopen_s(&file, path, "rb") != 0) {
        file = 0;
    }
#else
    file = fopen(path, "rb");
#endif

    if (file == 0) {
        return false;
    }

    if (fread(magic, 1, sizeof(magic), file) != sizeof(magic) ||
        memcmp(magic, R2D_ASSET_PACK_MAGIC, strlen(R2D_ASSET_PACK_MAGIC)) != 0 ||
        fread(&count, sizeof(count), 1, file) != 1) {
        fclose(file);
        return false;
    }

    entries = (R2D_AssetPackEntry *)calloc((size_t)count, sizeof(*entries));
    if (entries == 0 && count > 0) {
        fclose(file);
        return false;
    }

    for (uint32_t i = 0; i < count; ++i) {
        uint32_t path_length = 0;
        uint64_t size = 0;

        if (fread(&path_length, sizeof(path_length), 1, file) != 1 ||
            fread(&size, sizeof(size), 1, file) != 1 ||
            path_length == 0) {
            fclose(file);
            free(entries);
            return false;
        }

        entries[i].path = (char *)calloc((size_t)path_length + 1u, 1u);
        if (entries[i].path == 0 ||
            fread(entries[i].path, 1, path_length, file) != path_length) {
            fclose(file);
            for (uint32_t j = 0; j <= i; ++j) {
                free(entries[j].path);
            }
            free(entries);
            return false;
        }

        entries[i].offset = (uint64_t)ftell(file);
        entries[i].size = size;

        if (fseek(file, (long)size, SEEK_CUR) != 0) {
            fclose(file);
            for (uint32_t j = 0; j <= i; ++j) {
                free(entries[j].path);
            }
            free(entries);
            return false;
        }
    }

    r2d_asset_pack.file = file;
    r2d_asset_pack.entries = entries;
    r2d_asset_pack.count = count;
    r2d_asset_pack.mounted = true;

    TraceLog(LOG_INFO, "R2D: Asset pack mounted: %s (%u files)", path, count);
    return true;
}

void R2D_UnmountAssetPack(void)
{
    if (r2d_asset_pack.file != 0) {
        fclose(r2d_asset_pack.file);
    }

    if (r2d_asset_pack.entries != 0) {
        for (uint32_t i = 0; i < r2d_asset_pack.count; ++i) {
            free(r2d_asset_pack.entries[i].path);
        }

        free(r2d_asset_pack.entries);
    }

    r2d_asset_pack = (R2D_AssetPack) { 0 };
}

bool R2D_AssetPackMounted(void)
{
    return r2d_asset_pack.mounted;
}

bool R2D_AssetExists(const char *path)
{
    if (path == 0) {
        return false;
    }

    if (R2D_AssetPathIsAbsolute(path) && FileExists(path)) {
        return true;
    }

#ifdef R2D_DEVELOPMENT_ASSET_DIR
    if (FileExists(R2D_AssetPath(path))) {
        return true;
    }
#endif

    if (r2d_asset_pack.mounted && R2D_AssetFindPackEntry(path) >= 0) {
        return true;
    }

    return FileExists(R2D_AssetPath(path));
}

bool R2D_LoadAssetData(const char *path, unsigned char **data, int *size)
{
    int entry_index;

    if (data == 0 || size == 0 || path == 0) {
        return false;
    }

    *data = 0;
    *size = 0;

    if (R2D_AssetPathIsAbsolute(path) && R2D_LoadDiskFileData(path, data, size)) {
        return true;
    }

#ifdef R2D_DEVELOPMENT_ASSET_DIR
    if (R2D_LoadDiskFileData(R2D_AssetPath(path), data, size)) {
        return true;
    }
#endif

    entry_index = r2d_asset_pack.mounted ? R2D_AssetFindPackEntry(path) : -1;
    if (entry_index >= 0) {
        const R2D_AssetPackEntry *entry = &r2d_asset_pack.entries[entry_index];

        if (entry->size > (uint64_t)2147483647) {
            return false;
        }

        *data = (unsigned char *)malloc((size_t)entry->size);
        if (*data == 0) {
            return false;
        }

        if (fseek(r2d_asset_pack.file, (long)entry->offset, SEEK_SET) != 0 ||
            fread(*data, 1, (size_t)entry->size, r2d_asset_pack.file) != (size_t)entry->size) {
            free(*data);
            *data = 0;
            return false;
        }

        *size = (int)entry->size;
        return true;
    }

    return R2D_LoadDiskFileData(R2D_AssetPath(path), data, size);
}

void R2D_UnloadAssetData(unsigned char *data)
{
    if (data != 0) {
        free(data);
    }
}

char *R2D_LoadAssetText(const char *path)
{
    unsigned char *data = 0;
    int size = 0;
    char *text;

    if (!R2D_LoadAssetData(path, &data, &size)) {
        return 0;
    }

    text = (char *)malloc((size_t)size + 1u);
    if (text == 0) {
        R2D_UnloadAssetData(data);
        return 0;
    }

    memcpy(text, data, (size_t)size);
    text[size] = '\0';
    R2D_UnloadAssetData(data);
    return text;
}

void R2D_UnloadAssetText(char *text)
{
    free(text);
}

Texture2D R2D_LoadTexture(const char *path)
{
    unsigned char *data = 0;
    int size = 0;
    Image image;
    Texture2D texture = { 0 };

    if (!R2D_LoadAssetData(path, &data, &size)) {
        TraceLog(LOG_WARNING, "R2D: Failed to load texture data: %s", path != 0 ? path : "");
        return texture;
    }

    image = LoadImageFromMemory(R2D_AssetExtension(path), data, size);
    R2D_UnloadAssetData(data);

    if (!IsImageValid(image)) {
        TraceLog(LOG_WARNING, "R2D: Failed to decode texture: %s", path != 0 ? path : "");
        return texture;
    }

    texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

Shader R2D_LoadFragmentShader(const char *path)
{
    char *fragment = R2D_LoadAssetText(path);
    Shader shader = { 0 };

    if (fragment == 0) {
        TraceLog(LOG_WARNING, "R2D: Failed to load fragment shader text: %s", path != 0 ? path : "");
        return shader;
    }

    shader = LoadShaderFromMemory(0, fragment);
    R2D_UnloadAssetText(fragment);
    return shader;
}
