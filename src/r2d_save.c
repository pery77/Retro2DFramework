#include "r2d/r2d.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool R2D_SaveBoolValue(const char *value)
{
    if (value == 0) {
        return false;
    }

    return strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0;
}

static char *R2D_SaveTrim(char *text)
{
    char *end;

    if (text == 0) {
        return text;
    }

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

static bool R2D_SaveGetEnv(const char *name, char *destination, int destination_size)
{
    if (destination == 0 || destination_size <= 0) {
        return false;
    }

    destination[0] = '\0';

#if defined(_MSC_VER)
    {
        size_t required = 0;

        if (getenv_s(&required, 0, 0, name) != 0 || required == 0 || required > (size_t)destination_size) {
            return false;
        }

        if (getenv_s(&required, destination, (size_t)destination_size, name) != 0) {
            destination[0] = '\0';
            return false;
        }
    }
#else
    {
        const char *value = getenv(name);

        if (value == 0 || value[0] == '\0') {
            return false;
        }

        snprintf(destination, (size_t)destination_size, "%s", value);
    }
#endif

    return destination[0] != '\0';
}

const char *R2D_UserDataPath(const char *app_name, const char *file_name)
{
    static char path[R2D_SAVE_PATH_SIZE];
    char directory[R2D_SAVE_PATH_SIZE];
    char base[R2D_SAVE_PATH_SIZE];

    if (app_name == 0 || app_name[0] == '\0') {
        app_name = "Retro2DFramework";
    }

    if (file_name == 0 || file_name[0] == '\0') {
        file_name = "save.ini";
    }

#if defined(_WIN32)
    if (R2D_SaveGetEnv("APPDATA", base, sizeof(base))) {
        snprintf(directory, sizeof(directory), "%s/%s", base, app_name);
    } else {
        snprintf(directory, sizeof(directory), "%s%s", GetApplicationDirectory(), app_name);
    }
#else
    if (R2D_SaveGetEnv("XDG_DATA_HOME", base, sizeof(base))) {
        snprintf(directory, sizeof(directory), "%s/%s", base, app_name);
    } else {
        if (R2D_SaveGetEnv("HOME", base, sizeof(base))) {
            snprintf(directory, sizeof(directory), "%s/.local/share/%s", base, app_name);
        } else {
            snprintf(directory, sizeof(directory), "%s%s", GetApplicationDirectory(), app_name);
        }
    }
#endif

    if (!DirectoryExists(directory)) {
        MakeDirectory(directory);
    }

    snprintf(path, sizeof(path), "%s/%s", directory, file_name);
    return path;
}

R2D_SaveData R2D_SaveDataDefault(void)
{
    return (R2D_SaveData) {
        1,
        R2D_DEFAULT_WINDOW_SCALE,
        false,
        1.0f,
        0.8f,
        1.0f,
        0,
        0
    };
}

bool R2D_SaveDataLoad(const char *path, R2D_SaveData *save)
{
    FILE *file = 0;
    char line[256];

    if (save == 0 || path == 0) {
        return false;
    }

    *save = R2D_SaveDataDefault();

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

    while (fgets(line, sizeof(line), file) != 0) {
        char *equals;
        char *key;
        char *value;

        if (line[0] == '#' || line[0] == ';') {
            continue;
        }

        equals = strchr(line, '=');
        if (equals == 0) {
            continue;
        }

        *equals = '\0';
        key = R2D_SaveTrim(line);
        value = R2D_SaveTrim(equals + 1);

        if (strcmp(key, "version") == 0) {
            save->version = atoi(value);
        } else if (strcmp(key, "window_scale") == 0) {
            save->window_scale = atoi(value);
        } else if (strcmp(key, "fullscreen") == 0) {
            save->fullscreen = R2D_SaveBoolValue(value);
        } else if (strcmp(key, "master_volume") == 0) {
            save->master_volume = (float)atof(value);
        } else if (strcmp(key, "music_volume") == 0) {
            save->music_volume = (float)atof(value);
        } else if (strcmp(key, "sfx_volume") == 0) {
            save->sfx_volume = (float)atof(value);
        } else if (strcmp(key, "progress") == 0) {
            save->progress = atoi(value);
        } else if (strcmp(key, "high_score") == 0) {
            save->high_score = atoi(value);
        }
    }

    fclose(file);

    if (save->version <= 0) {
        save->version = 1;
    }

    if (save->window_scale <= 0) {
        save->window_scale = R2D_DEFAULT_WINDOW_SCALE;
    }

    save->master_volume = R2D_Clamp01(save->master_volume);
    save->music_volume = R2D_Clamp01(save->music_volume);
    save->sfx_volume = R2D_Clamp01(save->sfx_volume);
    return true;
}

bool R2D_SaveDataSave(const char *path, R2D_SaveData save)
{
    FILE *file = 0;

    if (path == 0) {
        return false;
    }

#if defined(_MSC_VER)
    if (fopen_s(&file, path, "wb") != 0) {
        file = 0;
    }
#else
    file = fopen(path, "wb");
#endif

    if (file == 0) {
        return false;
    }

    if (save.version <= 0) {
        save.version = 1;
    }

    if (save.window_scale <= 0) {
        save.window_scale = R2D_DEFAULT_WINDOW_SCALE;
    }

    save.master_volume = R2D_Clamp01(save.master_volume);
    save.music_volume = R2D_Clamp01(save.music_volume);
    save.sfx_volume = R2D_Clamp01(save.sfx_volume);

    fprintf(file, "version=%d\n", save.version);
    fprintf(file, "window_scale=%d\n", save.window_scale);
    fprintf(file, "fullscreen=%s\n", save.fullscreen ? "true" : "false");
    fprintf(file, "master_volume=%.3f\n", save.master_volume);
    fprintf(file, "music_volume=%.3f\n", save.music_volume);
    fprintf(file, "sfx_volume=%.3f\n", save.sfx_volume);
    fprintf(file, "progress=%d\n", save.progress);
    fprintf(file, "high_score=%d\n", save.high_score);

    fclose(file);
    return true;
}
