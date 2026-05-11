#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R2D_PACK_MAGIC "R2DAS01"
#define R2D_PACK_MAGIC_SIZE 8
#define R2D_MAX_ASSETS 512
#define R2D_MAX_PATH 1200

typedef struct AssetList {
    char paths[R2D_MAX_ASSETS][R2D_MAX_PATH];
    int count;
} AssetList;

static void NormalizePath(char *path)
{
    char input[R2D_MAX_PATH];
    char output[R2D_MAX_PATH] = "";
    char *token;
    char *next;

    snprintf(input, sizeof(input), "%s", path);

    for (int i = 0; input[i] != '\0'; ++i) {
        if (input[i] == '\\') {
            input[i] = '/';
        }
    }

    while (strncmp(input, "assets/", 7) == 0) {
        memmove(input, input + 7, strlen(input + 7) + 1);
    }

    token = input;
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
            char *slash = strrchr(output, '/');

            if (slash != 0) {
                *slash = '\0';
            } else if (output[0] != '\0' && strcmp(output, "..") != 0 && strcmp(output, "C:") != 0) {
                output[0] = '\0';
            } else if (output[0] == '\0') {
                snprintf(output, sizeof(output), "%s", "..");
            }
        } else {
            if (output[0] != '\0') {
                strncat(output, "/", sizeof(output) - strlen(output) - 1u);
            }

            strncat(output, token, sizeof(output) - strlen(output) - 1u);
        }

        if (next == 0) {
            break;
        }

        token = next + 1;
    }

    while (strncmp(output, "assets/", 7) == 0) {
        memmove(output, output + 7, strlen(output + 7) + 1);
    }

    snprintf(path, R2D_MAX_PATH, "%s", output);
}

static void DirectoryOf(char *destination, int destination_size, const char *path)
{
    const char *slash = strrchr(path, '/');
    int length = 0;

    if (destination_size <= 0) {
        return;
    }

    destination[0] = '\0';

    if (slash == 0) {
        return;
    }

    length = (int)(slash - path + 1);
    if (length >= destination_size) {
        length = destination_size - 1;
    }

    memcpy(destination, path, (size_t)length);
    destination[length] = '\0';
}

static void ResolveAssetPath(char *destination, int destination_size, const char *base_asset, const char *value)
{
    char directory[R2D_MAX_PATH];
    char value_copy[R2D_MAX_PATH];

    if (destination_size <= 0) {
        return;
    }

    if (value == 0) {
        value = "";
    }

    snprintf(value_copy, sizeof(value_copy), "%s", value);

    if (strncmp(value_copy, "assets/", 7) == 0) {
        snprintf(destination, (size_t)destination_size, "%s", value_copy);
    } else if (strchr(value_copy, '/') != 0 || strchr(value_copy, '\\') != 0) {
        DirectoryOf(directory, sizeof(directory), base_asset);
        snprintf(destination, (size_t)destination_size, "%s%s", directory, value_copy);
    } else {
        DirectoryOf(directory, sizeof(directory), base_asset);
        snprintf(destination, (size_t)destination_size, "%s%s", directory, value_copy);
    }

    NormalizePath(destination);
}

static int StringEndsWith(const char *text, const char *suffix)
{
    const size_t text_length = strlen(text);
    const size_t suffix_length = strlen(suffix);

    return text_length >= suffix_length &&
        strcmp(text + text_length - suffix_length, suffix) == 0;
}

static int LooksLikeAssetPath(const char *text)
{
    return strncmp(text, "assets/", 7) == 0 ||
        strncmp(text, "assets\\", 7) == 0 ||
        strncmp(text, "audio/", 6) == 0 ||
        strncmp(text, "audio\\", 6) == 0 ||
        strncmp(text, "textures/", 9) == 0 ||
        strncmp(text, "textures\\", 9) == 0 ||
        strncmp(text, "tilemaps/", 9) == 0 ||
        strncmp(text, "tilemaps\\", 9) == 0 ||
        strncmp(text, "shaders/", 8) == 0 ||
        strncmp(text, "shaders\\", 8) == 0;
}

static char *Trim(char *text)
{
    char *end;

    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
        ++text;
    }

    end = text + strlen(text);
    while (end > text && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
        --end;
    }

    *end = '\0';
    return text;
}

static int AddAsset(AssetList *assets, const char *path)
{
    char normalized[R2D_MAX_PATH];

    if (assets == 0 || path == 0 || path[0] == '\0') {
        return 0;
    }

    snprintf(normalized, sizeof(normalized), "%s", path);
    NormalizePath(normalized);

    for (int i = 0; i < assets->count; ++i) {
        if (strcmp(assets->paths[i], normalized) == 0) {
            return 0;
        }
    }

    if (assets->count >= R2D_MAX_ASSETS) {
        fprintf(stderr, "r2d_pack: too many assets\n");
        exit(1);
    }

    snprintf(assets->paths[assets->count], sizeof(assets->paths[assets->count]), "%s", normalized);
    ++assets->count;
    return 1;
}

static char *ReadTextFile(const char *path)
{
    FILE *file = 0;
    long size;
    char *text;

#if defined(_MSC_VER)
    if (fopen_s(&file, path, "rb") != 0) {
        file = 0;
    }
#else
    file = fopen(path, "rb");
#endif

    if (file == 0) {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }

    size = ftell(file);
    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }

    text = (char *)malloc((size_t)size + 1u);
    if (text == 0) {
        fclose(file);
        return 0;
    }

    if (size > 0 && fread(text, 1, (size_t)size, file) != (size_t)size) {
        fclose(file);
        free(text);
        return 0;
    }

    fclose(file);
    text[size] = '\0';
    return text;
}

static void BuildAssetFilePath(char *destination, int destination_size, const char *asset_root, const char *asset_path)
{
    snprintf(destination, (size_t)destination_size, "%s/%s", asset_root, asset_path);
    NormalizePath(destination);
}

static void AddSongDependencies(AssetList *assets, const char *asset_root, const char *asset_path)
{
    char full_path[R2D_MAX_PATH];
    char *text;
    char *line;

    BuildAssetFilePath(full_path, sizeof(full_path), asset_root, asset_path);
    text = ReadTextFile(full_path);
    if (text == 0) {
        return;
    }

    line = text;
    while (*line != '\0') {
        char *next = strchr(line, '\n');
        char *separator;

        if (next != 0) {
            *next = '\0';
        }

        separator = strchr(line, '=');
        if (separator != 0) {
            char dependency[R2D_MAX_PATH];
            char *key;
            char *value;

            *separator = '\0';
            key = Trim(line);
            value = Trim(separator + 1);

            if (strcmp(key, "midi") == 0) {
                ResolveAssetPath(dependency, sizeof(dependency), asset_path, value);
                AddAsset(assets, dependency);
            } else if (strcmp(key, "soundfont") == 0) {
                if (strchr(value, '/') == 0 && strchr(value, '\\') == 0) {
                    snprintf(dependency, sizeof(dependency), "audio/soundfonts/%s", value);
                    NormalizePath(dependency);
                } else {
                    ResolveAssetPath(dependency, sizeof(dependency), asset_path, value);
                }

                AddAsset(assets, dependency);
            }
        }

        if (next == 0) {
            break;
        }

        line = next + 1;
    }

    free(text);
}

static void AddQuotedPathDependencies(AssetList *assets, const char *asset_path, const char *text, const char *key)
{
    char pattern[64];
    const char *cursor = text;

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    while ((cursor = strstr(cursor, pattern)) != 0) {
        const char *value = cursor + strlen(pattern);
        char dependency[R2D_MAX_PATH];
        int length = 0;

        while (*value == ' ' || *value == '\t' || *value == '\r' || *value == '\n') {
            ++value;
        }

        if (*value != ':') {
            ++cursor;
            continue;
        }

        ++value;
        while (*value == ' ' || *value == '\t' || *value == '\r' || *value == '\n') {
            ++value;
        }

        if (*value != '"') {
            ++cursor;
            continue;
        }

        ++value;
        while (value[length] != '\0' && value[length] != '"' && length < R2D_MAX_PATH - 1) {
            dependency[length] = value[length];
            ++length;
        }

        dependency[length] = '\0';

        if (StringEndsWith(dependency, ".png") ||
            StringEndsWith(dependency, ".tsx") ||
            StringEndsWith(dependency, ".tmx") ||
            StringEndsWith(dependency, ".json")) {
            ResolveAssetPath(dependency, sizeof(dependency), asset_path, dependency);
            AddAsset(assets, dependency);
        }

        cursor = value + length;
    }
}

static void AddTsxDependencies(AssetList *assets, const char *asset_root, const char *asset_path)
{
    char full_path[R2D_MAX_PATH];
    char *text;
    char *image;

    BuildAssetFilePath(full_path, sizeof(full_path), asset_root, asset_path);
    text = ReadTextFile(full_path);
    if (text == 0) {
        return;
    }

    image = strstr(text, "<image");
    if (image != 0) {
        char *source = strstr(image, "source=\"");

        if (source != 0) {
            char dependency[R2D_MAX_PATH];
            int length = 0;

            source += strlen("source=\"");
            while (source[length] != '\0' && source[length] != '"' && length < R2D_MAX_PATH - 1) {
                dependency[length] = source[length];
                ++length;
            }

            dependency[length] = '\0';
            ResolveAssetPath(dependency, sizeof(dependency), asset_path, dependency);
            AddAsset(assets, dependency);
        }
    }

    free(text);
}

static void AddTilemapDependencies(AssetList *assets, const char *asset_root, const char *asset_path)
{
    char full_path[R2D_MAX_PATH];
    char *text;

    BuildAssetFilePath(full_path, sizeof(full_path), asset_root, asset_path);
    text = ReadTextFile(full_path);
    if (text == 0) {
        return;
    }

    AddQuotedPathDependencies(assets, asset_path, text, "image");
    AddQuotedPathDependencies(assets, asset_path, text, "source");
    free(text);
}

static void ScanSourceForAssets(AssetList *assets, const char *source_path)
{
    char *text = ReadTextFile(source_path);
    const char *cursor;

    if (text == 0) {
        fprintf(stderr, "r2d_pack: could not read source: %s\n", source_path);
        exit(1);
    }

    if (strstr(text, "R2D_CrtInit") != 0) {
        AddAsset(assets, "shaders/crt.fs");
        AddAsset(assets, "textures/noise.png");
    }

    cursor = text;
    while ((cursor = strstr(cursor, "R2D_AssetPath(\"")) != 0) {
        char asset_path[R2D_MAX_PATH];
        int length = 0;

        cursor += strlen("R2D_AssetPath(\"");
        while (cursor[length] != '\0' && cursor[length] != '"' && length < R2D_MAX_PATH - 1) {
            asset_path[length] = cursor[length];
            ++length;
        }

        asset_path[length] = '\0';
        AddAsset(assets, asset_path);
        cursor += length;
    }

    cursor = text;
    while ((cursor = strchr(cursor, '"')) != 0) {
        char asset_path[R2D_MAX_PATH];
        int length = 0;

        ++cursor;
        while (cursor[length] != '\0' && cursor[length] != '"' && length < R2D_MAX_PATH - 1) {
            asset_path[length] = cursor[length];
            ++length;
        }

        asset_path[length] = '\0';
        if (LooksLikeAssetPath(asset_path) &&
            (StringEndsWith(asset_path, ".png") ||
            StringEndsWith(asset_path, ".json") ||
            StringEndsWith(asset_path, ".tmx") ||
            StringEndsWith(asset_path, ".tsx") ||
            StringEndsWith(asset_path, ".fs") ||
            StringEndsWith(asset_path, ".r2sfx") ||
            StringEndsWith(asset_path, ".r2song") ||
            StringEndsWith(asset_path, ".mid") ||
            StringEndsWith(asset_path, ".midi") ||
            StringEndsWith(asset_path, ".sf2"))) {
            AddAsset(assets, asset_path);
        }

        cursor += length;
        if (*cursor == '"') {
            ++cursor;
        }
    }

    free(text);
}

static uint64_t FileSize(FILE *file)
{
    const long current = ftell(file);
    long size;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, current, SEEK_SET);

    return size < 0 ? 0u : (uint64_t)size;
}

static void WritePack(const AssetList *assets, const char *asset_root, const char *output_path)
{
    FILE *output = 0;
    char magic[R2D_PACK_MAGIC_SIZE] = R2D_PACK_MAGIC;
    uint32_t count = (uint32_t)assets->count;

#if defined(_MSC_VER)
    if (fopen_s(&output, output_path, "wb") != 0) {
        output = 0;
    }
#else
    output = fopen(output_path, "wb");
#endif

    if (output == 0) {
        fprintf(stderr, "r2d_pack: could not open output: %s\n", output_path);
        exit(1);
    }

    fwrite(magic, 1, sizeof(magic), output);
    fwrite(&count, sizeof(count), 1, output);

    for (int i = 0; i < assets->count; ++i) {
        char full_path[R2D_MAX_PATH];
        FILE *input = 0;
        uint32_t path_length = (uint32_t)strlen(assets->paths[i]);
        uint64_t size;

        BuildAssetFilePath(full_path, sizeof(full_path), asset_root, assets->paths[i]);

#if defined(_MSC_VER)
        if (fopen_s(&input, full_path, "rb") != 0) {
            input = 0;
        }
#else
        input = fopen(full_path, "rb");
#endif

        if (input == 0) {
            fprintf(stderr, "r2d_pack: missing asset: %s\n", assets->paths[i]);
            fclose(output);
            exit(1);
        }

        size = FileSize(input);
        fwrite(&path_length, sizeof(path_length), 1, output);
        fwrite(&size, sizeof(size), 1, output);
        fwrite(assets->paths[i], 1, path_length, output);

        for (;;) {
            unsigned char buffer[16384];
            const size_t read_count = fread(buffer, 1, sizeof(buffer), input);

            if (read_count > 0) {
                fwrite(buffer, 1, read_count, output);
            }

            if (read_count < sizeof(buffer)) {
                break;
            }
        }

        fclose(input);
    }

    fclose(output);
    printf("r2d_pack: wrote %s with %d assets\n", output_path, assets->count);
}

int main(int argc, char **argv)
{
    AssetList assets = { 0 };
    const char *asset_root;
    const char *output_path;

    if (argc < 5) {
        fprintf(stderr, "usage: r2d_asset_pack <config> <asset-root> <output.assets> <source> [source...]\n");
        return 1;
    }

    if (strcmp(argv[1], "Release") != 0) {
        printf("r2d_pack: skipping asset pack for %s config\n", argv[1]);
        return 0;
    }

    asset_root = argv[2];
    output_path = argv[3];

    for (int i = 4; i < argc; ++i) {
        ScanSourceForAssets(&assets, argv[i]);
    }

    for (int i = 0; i < assets.count; ++i) {
        if (StringEndsWith(assets.paths[i], ".r2song")) {
            AddSongDependencies(&assets, asset_root, assets.paths[i]);
        } else if (StringEndsWith(assets.paths[i], ".json") || StringEndsWith(assets.paths[i], ".tmx")) {
            AddTilemapDependencies(&assets, asset_root, assets.paths[i]);
        } else if (StringEndsWith(assets.paths[i], ".tsx")) {
            AddTsxDependencies(&assets, asset_root, assets.paths[i]);
        }
    }

    WritePack(&assets, asset_root, output_path);
    return 0;
}
