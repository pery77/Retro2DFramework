#include "r2d/r2d.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *R2D_RuntimeTrim(char *text)
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

static bool R2D_RuntimeBool(const char *value, bool fallback)
{
    if (value == 0) {
        return fallback;
    }

    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0 || strcmp(value, "on") == 0) {
        return true;
    }

    if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0 || strcmp(value, "no") == 0 || strcmp(value, "off") == 0) {
        return false;
    }

    return fallback;
}

static void R2D_RuntimeSetAssetPack(R2D_RuntimeConfig *runtime, const char *path)
{
    if (runtime == 0) {
        return;
    }

    if (path == 0) {
        path = "";
    }

    snprintf(runtime->asset_pack_path, sizeof(runtime->asset_pack_path), "%s", path);
    runtime->config.asset_pack_path = runtime->asset_pack_path[0] != '\0' ? runtime->asset_pack_path : 0;
}

static void R2D_RuntimeApplyPair(R2D_RuntimeConfig *runtime, const char *key, const char *value)
{
    if (runtime == 0 || key == 0 || value == 0) {
        return;
    }

    if (strcmp(key, "virtual_width") == 0 || strcmp(key, "width") == 0) {
        runtime->config.virtual_width = atoi(value);
    } else if (strcmp(key, "virtual_height") == 0 || strcmp(key, "height") == 0) {
        runtime->config.virtual_height = atoi(value);
    } else if (strcmp(key, "resolution") == 0) {
        int width = 0;
        int height = 0;
        char *separator = strchr(value, 'x');

        if (separator == 0) {
            separator = strchr(value, 'X');
        }

        if (separator != 0) {
            width = atoi(value);
            height = atoi(separator + 1);
        }

        if (width > 0 && height > 0) {
            runtime->config.virtual_width = width;
            runtime->config.virtual_height = height;
        }
    } else if (strcmp(key, "window_scale") == 0 || strcmp(key, "scale") == 0) {
        runtime->config.window_scale = atoi(value);
    } else if (strcmp(key, "fullscreen") == 0) {
        runtime->config.fullscreen = R2D_RuntimeBool(value, runtime->config.fullscreen);
    } else if (strcmp(key, "crt") == 0 || strcmp(key, "crt_enabled") == 0) {
        runtime->crt_enabled = R2D_RuntimeBool(value, runtime->crt_enabled);
    } else if (strcmp(key, "master_volume") == 0 || strcmp(key, "volume") == 0) {
        runtime->master_volume = R2D_Clamp01((float)atof(value));
    } else if (strcmp(key, "music_volume") == 0) {
        runtime->music_volume = R2D_Clamp01((float)atof(value));
    } else if (strcmp(key, "sfx_volume") == 0) {
        runtime->sfx_volume = R2D_Clamp01((float)atof(value));
    } else if (strcmp(key, "ui_volume") == 0) {
        runtime->ui_volume = R2D_Clamp01((float)atof(value));
    } else if (strcmp(key, "ambient_volume") == 0) {
        runtime->ambient_volume = R2D_Clamp01((float)atof(value));
    } else if (strcmp(key, "asset_pack") == 0 || strcmp(key, "asset_pack_path") == 0) {
        R2D_RuntimeSetAssetPack(runtime, value);
    }
}

R2D_RuntimeConfig R2D_RuntimeConfigDefault(void)
{
    R2D_RuntimeConfig runtime = { 0 };

    runtime.config = R2D_DefaultConfig();
    runtime.crt_enabled = true;
    runtime.master_volume = 1.0f;
    runtime.music_volume = 1.0f;
    runtime.sfx_volume = 1.0f;
    runtime.ui_volume = 1.0f;
    runtime.ambient_volume = 1.0f;

    return runtime;
}

bool R2D_RuntimeConfigLoad(R2D_RuntimeConfig *runtime, const char *path)
{
    FILE *file = 0;
    char line[512];

    if (runtime == 0 || path == 0) {
        return false;
    }

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
        key = R2D_RuntimeTrim(line);
        value = R2D_RuntimeTrim(equals + 1);
        R2D_RuntimeApplyPair(runtime, key, value);
    }

    fclose(file);
    return true;
}

void R2D_RuntimeConfigApplyArgs(R2D_RuntimeConfig *runtime, int argc, char **argv)
{
    for (int i = 1; runtime != 0 && i < argc; ++i) {
        char key[64];
        const char *arg = argv[i];
        const char *value = 0;
        const char *equals;

        if (arg == 0 || strncmp(arg, "--", 2) != 0) {
            continue;
        }

        arg += 2;
        equals = strchr(arg, '=');
        if (equals != 0) {
            const int key_length = (int)(equals - arg);

            if (key_length <= 0 || key_length >= (int)sizeof(key)) {
                continue;
            }

            memcpy(key, arg, (size_t)key_length);
            key[key_length] = '\0';
            value = equals + 1;
        } else {
            snprintf(key, sizeof(key), "%s", arg);

            if (strcmp(key, "fullscreen") == 0) {
                value = "true";
            } else if (strcmp(key, "windowed") == 0 || strcmp(key, "no-crt") == 0) {
                value = "false";
            } else if (i + 1 < argc) {
                value = argv[++i];
            }
        }

        for (int key_index = 0; key[key_index] != '\0'; ++key_index) {
            if (key[key_index] == '-') {
                key[key_index] = '_';
            }
        }

        if (strcmp(key, "windowed") == 0) {
            R2D_RuntimeApplyPair(runtime, "fullscreen", "false");
        } else if (strcmp(key, "no-crt") == 0) {
            R2D_RuntimeApplyPair(runtime, "crt", "false");
        } else if (value != 0) {
            R2D_RuntimeApplyPair(runtime, key, value);
        }
    }
}

void R2D_RuntimeConfigApplyAudio(const R2D_RuntimeConfig *runtime)
{
    if (runtime == 0) {
        return;
    }

    R2D_AudioSetMasterVolume(runtime->master_volume);
    R2D_AudioSetGroupVolume(R2D_AUDIO_GROUP_MUSIC, runtime->music_volume);
    R2D_AudioSetGroupVolume(R2D_AUDIO_GROUP_SFX, runtime->sfx_volume);
    R2D_AudioSetGroupVolume(R2D_AUDIO_GROUP_UI, runtime->ui_volume);
    R2D_AudioSetGroupVolume(R2D_AUDIO_GROUP_AMBIENT, runtime->ambient_volume);
}

void R2D_RuntimeConfigApplyCrt(const R2D_RuntimeConfig *runtime, R2D_Crt *crt)
{
    if (runtime == 0 || crt == 0) {
        return;
    }

    R2D_CrtSetEnabled(crt, runtime->crt_enabled);
}
