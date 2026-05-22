#include "r2d/r2d.h"

#include <stdio.h>
#include <string.h>

static long long R2D_FileWatchModTime(const char *path)
{
    if (path == 0 || path[0] == '\0' || !FileExists(path)) {
        return 0;
    }

    return (long long)GetFileModTime(path);
}

static bool R2D_FileWatchPathIsAbsolute(const char *path)
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

void R2D_FileWatchInit(R2D_FileWatch *watch)
{
    if (watch != 0) {
        memset(watch, 0, sizeof(*watch));
    }
}

bool R2D_FileWatchSet(R2D_FileWatch *watch, const char *path)
{
    if (watch == 0 || path == 0 || path[0] == '\0') {
        return false;
    }

    snprintf(watch->path, sizeof(watch->path), "%s", R2D_FileWatchPathIsAbsolute(path) ? path : R2D_AssetPath(path));
    watch->last_write_time = R2D_FileWatchModTime(watch->path);
    watch->exists = watch->last_write_time > 0;
    watch->initialized = true;
    return watch->exists;
}

bool R2D_FileWatchCheck(R2D_FileWatch *watch)
{
    const long long write_time = watch != 0 ? R2D_FileWatchModTime(watch->path) : 0;
    const bool exists = write_time > 0;
    bool changed = false;

    if (watch == 0 || !watch->initialized) {
        return false;
    }

    changed = exists != watch->exists || (exists && write_time != watch->last_write_time);
    watch->exists = exists;
    watch->last_write_time = write_time;
    return changed;
}

bool R2D_FileWatchExists(const R2D_FileWatch *watch)
{
    return watch != 0 && watch->exists;
}

const char *R2D_FileWatchPath(const R2D_FileWatch *watch)
{
    return watch != 0 ? watch->path : "";
}
