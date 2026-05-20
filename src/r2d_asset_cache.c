#include "r2d/r2d.h"

#include <stdio.h>
#include <string.h>

static void R2D_AssetCacheKey(char *destination, int destination_size, const char *path)
{
    if (destination_size <= 0) {
        return;
    }

    if (path == 0) {
        path = "";
    }

    snprintf(destination, (size_t)destination_size, "%s", path);

    for (int i = 0; destination[i] != '\0'; ++i) {
        if (destination[i] == '\\') {
            destination[i] = '/';
        }
    }
}

static int R2D_AssetCacheFindTexture(const R2D_AssetCache *cache, const char *key)
{
    if (cache == 0 || key == 0) {
        return -1;
    }

    for (int i = 0; i < R2D_ASSET_CACHE_MAX_TEXTURES; ++i) {
        if (cache->textures[i].loaded && strcmp(cache->textures[i].path, key) == 0) {
            return i;
        }
    }

    return -1;
}

static int R2D_AssetCacheFindShader(const R2D_AssetCache *cache, const char *key)
{
    if (cache == 0 || key == 0) {
        return -1;
    }

    for (int i = 0; i < R2D_ASSET_CACHE_MAX_SHADERS; ++i) {
        if (cache->shaders[i].loaded && strcmp(cache->shaders[i].path, key) == 0) {
            return i;
        }
    }

    return -1;
}

static int R2D_AssetCacheFreeTextureSlot(const R2D_AssetCache *cache)
{
    if (cache == 0) {
        return -1;
    }

    for (int i = 0; i < R2D_ASSET_CACHE_MAX_TEXTURES; ++i) {
        if (!cache->textures[i].loaded) {
            return i;
        }
    }

    return -1;
}

static int R2D_AssetCacheFreeShaderSlot(const R2D_AssetCache *cache)
{
    if (cache == 0) {
        return -1;
    }

    for (int i = 0; i < R2D_ASSET_CACHE_MAX_SHADERS; ++i) {
        if (!cache->shaders[i].loaded) {
            return i;
        }
    }

    return -1;
}

void R2D_AssetCacheInit(R2D_AssetCache *cache)
{
    if (cache != 0) {
        memset(cache, 0, sizeof(*cache));
    }
}

void R2D_AssetCacheClear(R2D_AssetCache *cache)
{
    if (cache == 0) {
        return;
    }

    for (int i = 0; i < R2D_ASSET_CACHE_MAX_TEXTURES; ++i) {
        if (cache->textures[i].loaded && IsTextureValid(cache->textures[i].texture)) {
            UnloadTexture(cache->textures[i].texture);
        }
    }

    for (int i = 0; i < R2D_ASSET_CACHE_MAX_SHADERS; ++i) {
        if (cache->shaders[i].loaded && IsShaderValid(cache->shaders[i].shader)) {
            UnloadShader(cache->shaders[i].shader);
        }
    }

    R2D_AssetCacheInit(cache);
}

void R2D_AssetCacheReleaseGroup(R2D_AssetCache *cache, int group)
{
    if (cache == 0) {
        return;
    }

    for (int i = 0; i < R2D_ASSET_CACHE_MAX_TEXTURES; ++i) {
        if (!cache->textures[i].loaded || cache->textures[i].group != group) {
            continue;
        }

        if (IsTextureValid(cache->textures[i].texture)) {
            UnloadTexture(cache->textures[i].texture);
        }

        memset(&cache->textures[i], 0, sizeof(cache->textures[i]));
        cache->texture_count--;
    }

    for (int i = 0; i < R2D_ASSET_CACHE_MAX_SHADERS; ++i) {
        if (!cache->shaders[i].loaded || cache->shaders[i].group != group) {
            continue;
        }

        if (IsShaderValid(cache->shaders[i].shader)) {
            UnloadShader(cache->shaders[i].shader);
        }

        memset(&cache->shaders[i], 0, sizeof(cache->shaders[i]));
        cache->shader_count--;
    }
}

Texture2D R2D_AssetCacheLoadTexture(R2D_AssetCache *cache, const char *path, int group)
{
    char key[R2D_ASSET_CACHE_PATH_SIZE];
    int index;
    Texture2D texture = { 0 };

    if (cache == 0 || path == 0) {
        return texture;
    }

    R2D_AssetCacheKey(key, sizeof(key), path);
    index = R2D_AssetCacheFindTexture(cache, key);
    if (index >= 0) {
        return cache->textures[index].texture;
    }

    index = R2D_AssetCacheFreeTextureSlot(cache);
    if (index < 0) {
        TraceLog(LOG_WARNING, "R2D: Texture cache full, cannot load: %s", path);
        return texture;
    }

    texture = R2D_LoadTexture(path);
    if (!IsTextureValid(texture)) {
        return texture;
    }

    snprintf(cache->textures[index].path, sizeof(cache->textures[index].path), "%s", key);
    cache->textures[index].texture = texture;
    cache->textures[index].group = group;
    cache->textures[index].loaded = true;
    cache->texture_count++;
    return texture;
}

Shader R2D_AssetCacheLoadFragmentShader(R2D_AssetCache *cache, const char *path, int group)
{
    char key[R2D_ASSET_CACHE_PATH_SIZE];
    int index;
    Shader shader = { 0 };

    if (cache == 0 || path == 0) {
        return shader;
    }

    R2D_AssetCacheKey(key, sizeof(key), path);
    index = R2D_AssetCacheFindShader(cache, key);
    if (index >= 0) {
        return cache->shaders[index].shader;
    }

    index = R2D_AssetCacheFreeShaderSlot(cache);
    if (index < 0) {
        TraceLog(LOG_WARNING, "R2D: Shader cache full, cannot load: %s", path);
        return shader;
    }

    shader = R2D_LoadFragmentShader(path);
    if (!IsShaderValid(shader)) {
        return shader;
    }

    snprintf(cache->shaders[index].path, sizeof(cache->shaders[index].path), "%s", key);
    cache->shaders[index].shader = shader;
    cache->shaders[index].group = group;
    cache->shaders[index].loaded = true;
    cache->shader_count++;
    return shader;
}

int R2D_AssetCacheTextureCount(const R2D_AssetCache *cache)
{
    return cache != 0 ? cache->texture_count : 0;
}

int R2D_AssetCacheShaderCount(const R2D_AssetCache *cache)
{
    return cache != 0 ? cache->shader_count : 0;
}
