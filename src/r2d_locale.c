#include "r2d/r2d.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *R2D_LocaleTrim(char *text)
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

static char *R2D_LocaleReadDiskText(const char *path)
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

static void R2D_LocaleUnescape(char *text)
{
    char *read;
    char *write;

    if (text == 0) {
        return;
    }

    read = text;
    write = text;
    while (*read != '\0') {
        if (*read == '\\') {
            ++read;
            if (*read == 'n') {
                *write++ = '\n';
                ++read;
            } else if (*read == 't') {
                *write++ = '\t';
                ++read;
            } else if (*read == '\\') {
                *write++ = '\\';
                ++read;
            } else if (*read != '\0') {
                *write++ = *read++;
            }
        } else {
            *write++ = *read++;
        }
    }
    *write = '\0';
}

static bool R2D_LocalizationParse(R2D_Localization *localization, char *text, const char *language)
{
    char *cursor;

    if (localization == 0 || text == 0) {
        return false;
    }

    R2D_LocalizationClear(localization);
    snprintf(localization->language, sizeof(localization->language), "%s", language != 0 ? language : "");

    cursor = text;
    while (*cursor != '\0') {
        char *line = cursor;
        char *next = strchr(cursor, '\n');
        char *equals;
        char *key;
        char *value;

        if (next != 0) {
            *next = '\0';
            cursor = next + 1;
        } else {
            cursor += strlen(cursor);
        }

        key = R2D_LocaleTrim(line);
        if (key[0] == '\0' || key[0] == '#' || key[0] == ';') {
            continue;
        }

        equals = strchr(key, '=');
        if (equals == 0) {
            continue;
        }

        *equals = '\0';
        value = R2D_LocaleTrim(equals + 1);
        key = R2D_LocaleTrim(key);
        R2D_LocaleUnescape(value);
        R2D_LocalizationSet(localization, key, value);
    }

    return localization->count > 0;
}

void R2D_LocalizationInit(R2D_Localization *localization)
{
    if (localization != 0) {
        *localization = (R2D_Localization) { 0 };
    }
}

bool R2D_LocalizationLoad(R2D_Localization *localization, const char *language)
{
    char path[1024];

    if (localization == 0) {
        return false;
    }

    if (language == 0 || language[0] == '\0') {
        language = "en";
    }

    snprintf(path, sizeof(path), "%slocale/%s.r2loc", GetApplicationDirectory(), language);
    if (R2D_LocalizationLoadFile(localization, path, language)) {
        return true;
    }

    snprintf(path, sizeof(path), "locale/%s.r2loc", language);
    return R2D_LocalizationLoadFile(localization, path, language);
}

bool R2D_LocalizationLoadFile(R2D_Localization *localization, const char *path, const char *language)
{
    char *text;
    bool ok;

    if (localization == 0 || path == 0) {
        return false;
    }

    text = R2D_LocaleReadDiskText(path);
    if (text != 0) {
        ok = R2D_LocalizationParse(localization, text, language);
        free(text);
        return ok;
    }

    text = R2D_LoadAssetText(path);
    if (text == 0) {
        return false;
    }

    ok = R2D_LocalizationParse(localization, text, language);
    R2D_UnloadAssetText(text);
    return ok;
}

bool R2D_LocalizationSet(R2D_Localization *localization, const char *key, const char *text)
{
    R2D_LocalizedText *entry = 0;

    if (localization == 0 || key == 0 || key[0] == '\0' || text == 0) {
        return false;
    }

    for (int i = 0; i < localization->count; ++i) {
        if (strcmp(localization->entries[i].key, key) == 0) {
            entry = &localization->entries[i];
            break;
        }
    }

    if (entry == 0) {
        if (localization->count >= R2D_LOCALE_MAX_ENTRIES) {
            return false;
        }
        entry = &localization->entries[localization->count++];
    }

    snprintf(entry->key, sizeof(entry->key), "%s", key);
    snprintf(entry->text, sizeof(entry->text), "%s", text);
    return true;
}

bool R2D_LocalizationHas(const R2D_Localization *localization, const char *key)
{
    if (localization == 0 || key == 0) {
        return false;
    }

    for (int i = 0; i < localization->count; ++i) {
        if (strcmp(localization->entries[i].key, key) == 0) {
            return true;
        }
    }

    return false;
}

const char *R2D_LocalizationGet(const R2D_Localization *localization, const char *key, const char *fallback)
{
    if (fallback == 0) {
        fallback = key != 0 ? key : "";
    }

    if (localization == 0 || key == 0) {
        return fallback;
    }

    for (int i = 0; i < localization->count; ++i) {
        if (strcmp(localization->entries[i].key, key) == 0) {
            return localization->entries[i].text;
        }
    }

    return fallback;
}

void R2D_LocalizationClear(R2D_Localization *localization)
{
    if (localization != 0) {
        *localization = (R2D_Localization) { 0 };
    }
}
