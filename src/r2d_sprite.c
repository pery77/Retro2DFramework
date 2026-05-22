#include "r2d/r2d.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *R2D_AnimTrim(char *text);

static int R2D_SpriteClampFrame(const R2D_SpriteSheet *sheet, int frame)
{
    const int frame_count = R2D_SpriteSheetFrameCount(sheet);

    if (frame_count <= 0) {
        return 0;
    }

    if (frame < 0) {
        return 0;
    }

    if (frame >= frame_count) {
        return frame_count - 1;
    }

    return frame;
}

R2D_SpriteSheet R2D_LoadSpriteSheet(const char *path, int frame_width, int frame_height)
{
    Texture2D texture = { 0 };

    if (path != 0) {
        texture = R2D_LoadTexture(path);
    }

    if (!IsTextureValid(texture)) {
        TraceLog(LOG_WARNING, "R2D: Failed to load sprite sheet: %s", path != 0 ? path : "");
        return (R2D_SpriteSheet) { 0 };
    }

    TraceLog(LOG_INFO, "R2D: Sprite sheet loaded: %s id=%u", path, texture.id);
    return R2D_SpriteSheetFromTexture(texture, frame_width, frame_height);
}

R2D_SpriteSheet R2D_SpriteSheetFromTexture(Texture2D texture, int frame_width, int frame_height)
{
    R2D_SpriteSheet sheet = { 0 };

    if (!IsTextureValid(texture) || frame_width <= 0 || frame_height <= 0) {
        return sheet;
    }

    SetTextureFilter(texture, TEXTURE_FILTER_POINT);

    sheet.texture = texture;
    sheet.frame_width = frame_width;
    sheet.frame_height = frame_height;
    sheet.columns = texture.width / frame_width;
    sheet.rows = texture.height / frame_height;

    if (sheet.columns <= 0 || sheet.rows <= 0) {
        sheet.columns = 0;
        sheet.rows = 0;
    }

    return sheet;
}

void R2D_UnloadSpriteSheet(R2D_SpriteSheet *sheet)
{
    if (sheet == 0) {
        return;
    }

    if (IsTextureValid(sheet->texture)) {
        UnloadTexture(sheet->texture);
    }

    *sheet = (R2D_SpriteSheet) { 0 };
}

bool R2D_SpriteSheetIsReady(const R2D_SpriteSheet *sheet)
{
    return sheet != 0 &&
        IsTextureValid(sheet->texture) &&
        sheet->frame_width > 0 &&
        sheet->frame_height > 0 &&
        sheet->columns > 0 &&
        sheet->rows > 0;
}

int R2D_SpriteSheetFrameCount(const R2D_SpriteSheet *sheet)
{
    if (!R2D_SpriteSheetIsReady(sheet)) {
        return 0;
    }

    return sheet->columns * sheet->rows;
}

Rectangle R2D_SpriteSheetFrame(const R2D_SpriteSheet *sheet, int frame)
{
    int column;
    int row;

    if (!R2D_SpriteSheetIsReady(sheet)) {
        return (Rectangle) { 0 };
    }

    frame = R2D_SpriteClampFrame(sheet, frame);
    column = frame % sheet->columns;
    row = frame / sheet->columns;

    return (Rectangle) {
        (float)(column * sheet->frame_width),
        (float)(row * sheet->frame_height),
        (float)sheet->frame_width,
        (float)sheet->frame_height
    };
}

static bool R2D_SpriteParseCsvFloat(char **cursor, float *value)
{
    char *start;
    char *comma;

    if (cursor == 0 || *cursor == 0 || value == 0) {
        return false;
    }

    start = *cursor;
    comma = strchr(start, ',');
    if (comma != 0) {
        *comma = '\0';
        *cursor = comma + 1;
    } else {
        *cursor = start + strlen(start);
    }

    start = R2D_AnimTrim(start);
    if (start[0] == '\0') {
        return false;
    }

    *value = (float)atof(start);
    return true;
}

static bool R2D_SpriteAtlasAddGridFrame(
    R2D_SpriteAtlas *atlas,
    const char *name,
    int index,
    int frame_width,
    int frame_height,
    Vector2 pivot,
    Rectangle hitbox,
    Rectangle hurtbox,
    bool has_hitbox,
    bool has_hurtbox
)
{
    R2D_SpriteAtlasFrame *frame;
    int columns;

    if (atlas == 0 ||
        name == 0 ||
        name[0] == '\0' ||
        atlas->frame_count >= R2D_SPRITE_ATLAS_MAX_FRAMES ||
        frame_width <= 0 ||
        frame_height <= 0 ||
        !IsTextureValid(atlas->texture)) {
        return false;
    }

    columns = atlas->texture.width / frame_width;
    if (columns <= 0) {
        return false;
    }

    frame = &atlas->frames[atlas->frame_count++];
    *frame = (R2D_SpriteAtlasFrame) { 0 };
    snprintf(frame->name, sizeof(frame->name), "%s", name);
    frame->source = R2D_Rect(
        (float)((index % columns) * frame_width),
        (float)((index / columns) * frame_height),
        (float)frame_width,
        (float)frame_height
    );
    frame->pivot = pivot;
    frame->hitbox = hitbox;
    frame->hurtbox = hurtbox;
    frame->has_hitbox = has_hitbox;
    frame->has_hurtbox = has_hurtbox;
    return true;
}

static bool R2D_SpriteAtlasParseFrame(
    R2D_SpriteAtlas *atlas,
    char *value,
    int frame_width,
    int frame_height
)
{
    char *cursor;
    char *comma;
    char *name;
    float values[11] = { 0 };
    int count = 0;
    Rectangle hitbox = { 0 };
    Rectangle hurtbox = { 0 };
    bool has_hitbox = false;
    bool has_hurtbox = false;

    if (value == 0) {
        return false;
    }

    name = R2D_AnimTrim(value);
    comma = strchr(name, ',');
    if (comma == 0) {
        return false;
    }

    *comma = '\0';
    cursor = comma + 1;
    while (count < 11 && cursor != 0 && cursor[0] != '\0') {
        if (!R2D_SpriteParseCsvFloat(&cursor, &values[count])) {
            return false;
        }
        ++count;
    }

    if (count < 3) {
        return false;
    }

    if (count >= 7) {
        hitbox = R2D_Rect(values[3], values[4], values[5], values[6]);
        has_hitbox = hitbox.width > 0.0f && hitbox.height > 0.0f;
    }

    if (count >= 11) {
        hurtbox = R2D_Rect(values[7], values[8], values[9], values[10]);
        has_hurtbox = hurtbox.width > 0.0f && hurtbox.height > 0.0f;
    }

    return R2D_SpriteAtlasAddGridFrame(
        atlas,
        name,
        (int)values[0],
        frame_width,
        frame_height,
        (Vector2) { values[1], values[2] },
        hitbox,
        hurtbox,
        has_hitbox,
        has_hurtbox
    );
}

bool R2D_LoadSpriteAtlas(R2D_SpriteAtlas *atlas, const char *path)
{
    char *text;
    char *cursor;
    char texture_path[1024] = "";
    int frame_width = 0;
    int frame_height = 0;

    if (atlas == 0 || path == 0) {
        return false;
    }

    *atlas = (R2D_SpriteAtlas) { 0 };
    text = R2D_LoadAssetText(path);
    if (text == 0) {
        return false;
    }

    cursor = text;
    while (*cursor != '\0') {
        char line[512];
        char *equals;
        char *key;
        char *value;
        int length = 0;

        while (cursor[length] != '\0' && cursor[length] != '\n' && length < (int)sizeof(line) - 1) {
            line[length] = cursor[length];
            ++length;
        }
        line[length] = '\0';

        while (cursor[length] != '\0' && cursor[length] != '\n') {
            ++length;
        }
        cursor += length;
        if (*cursor == '\n') {
            ++cursor;
        }

        key = R2D_AnimTrim(line);
        if (key[0] == '\0' || key[0] == '#' || key[0] == ';') {
            continue;
        }

        equals = strchr(key, '=');
        if (equals == 0) {
            continue;
        }

        *equals = '\0';
        value = R2D_AnimTrim(equals + 1);
        key = R2D_AnimTrim(key);

        if (strcmp(key, "texture") == 0) {
            snprintf(texture_path, sizeof(texture_path), "%s", value);
            atlas->texture = R2D_LoadTexture(value);
        } else if (strcmp(key, "frame_width") == 0) {
            frame_width = atoi(value);
        } else if (strcmp(key, "frame_height") == 0) {
            frame_height = atoi(value);
        } else if (strcmp(key, "frame") == 0 && IsTextureValid(atlas->texture)) {
            R2D_SpriteAtlasParseFrame(atlas, value, frame_width, frame_height);
        }
    }

    R2D_UnloadAssetText(text);
    if (!IsTextureValid(atlas->texture) || atlas->frame_count <= 0) {
        TraceLog(LOG_WARNING, "R2D: Failed to load sprite atlas: %s", path);
        R2D_UnloadSpriteAtlas(atlas);
        return false;
    }

    SetTextureFilter(atlas->texture, TEXTURE_FILTER_POINT);
    TraceLog(LOG_INFO, "R2D: Sprite atlas loaded: %s texture=%s frames=%d", path, texture_path, atlas->frame_count);
    return true;
}

void R2D_UnloadSpriteAtlas(R2D_SpriteAtlas *atlas)
{
    if (atlas == 0) {
        return;
    }

    if (IsTextureValid(atlas->texture)) {
        UnloadTexture(atlas->texture);
    }

    *atlas = (R2D_SpriteAtlas) { 0 };
}

const R2D_SpriteAtlasFrame *R2D_SpriteAtlasFind(const R2D_SpriteAtlas *atlas, const char *name)
{
    if (atlas == 0 || name == 0) {
        return 0;
    }

    for (int i = 0; i < atlas->frame_count; ++i) {
        if (strcmp(atlas->frames[i].name, name) == 0) {
            return &atlas->frames[i];
        }
    }

    return 0;
}

Rectangle R2D_SpriteAtlasHitbox(const R2D_SpriteAtlasFrame *frame, Vector2 position)
{
    if (frame == 0 || !frame->has_hitbox) {
        return (Rectangle) { 0 };
    }

    return R2D_Rect(
        position.x - frame->pivot.x + frame->hitbox.x,
        position.y - frame->pivot.y + frame->hitbox.y,
        frame->hitbox.width,
        frame->hitbox.height
    );
}

Rectangle R2D_SpriteAtlasHurtbox(const R2D_SpriteAtlasFrame *frame, Vector2 position)
{
    if (frame == 0 || !frame->has_hurtbox) {
        return (Rectangle) { 0 };
    }

    return R2D_Rect(
        position.x - frame->pivot.x + frame->hurtbox.x,
        position.y - frame->pivot.y + frame->hurtbox.y,
        frame->hurtbox.width,
        frame->hurtbox.height
    );
}

R2D_Anim R2D_AnimFrames(int first_frame, int frame_count, float fps, bool loop)
{
    if (first_frame < 0) {
        first_frame = 0;
    }

    if (frame_count < 1) {
        frame_count = 1;
    }

    if (fps <= 0.0f) {
        fps = 1.0f;
    }

    return (R2D_Anim) {
        first_frame,
        frame_count,
        fps,
        loop
    };
}

void R2D_AnimSetInit(R2D_AnimSet *set)
{
    if (set != 0) {
        *set = (R2D_AnimSet) { 0 };
    }
}

bool R2D_AnimSetAdd(R2D_AnimSet *set, const char *name, R2D_Anim anim)
{
    R2D_AnimClip *clip;

    if (set == 0 || name == 0 || name[0] == '\0') {
        return false;
    }

    clip = 0;
    for (int i = 0; i < set->count; ++i) {
        if (strcmp(set->clips[i].name, name) == 0) {
            clip = &set->clips[i];
            break;
        }
    }

    if (clip == 0) {
        if (set->count >= R2D_ANIM_SET_MAX_CLIPS) {
            return false;
        }

        clip = &set->clips[set->count++];
    }

    snprintf(clip->name, sizeof(clip->name), "%s", name);
    clip->anim = anim;
    return true;
}

const R2D_Anim *R2D_AnimSetFind(const R2D_AnimSet *set, const char *name)
{
    if (set == 0 || name == 0) {
        return 0;
    }

    for (int i = 0; i < set->count; ++i) {
        if (strcmp(set->clips[i].name, name) == 0) {
            return &set->clips[i].anim;
        }
    }

    return 0;
}

R2D_Anim R2D_AnimSetGet(const R2D_AnimSet *set, const char *name, R2D_Anim fallback)
{
    const R2D_Anim *anim = R2D_AnimSetFind(set, name);

    return anim != 0 ? *anim : fallback;
}

static char *R2D_AnimTrim(char *text)
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

static bool R2D_AnimParseBool(const char *text)
{
    return text != 0 &&
        (strcmp(text, "true") == 0 ||
            strcmp(text, "yes") == 0 ||
            strcmp(text, "on") == 0 ||
            strcmp(text, "1") == 0);
}

static bool R2D_AnimParseClip(char *value, R2D_Anim *anim)
{
    char *parts[4] = { 0 };
    char *cursor = value;

    if (value == 0 || anim == 0) {
        return false;
    }

    for (int i = 0; i < 4; ++i) {
        char *comma;

        parts[i] = R2D_AnimTrim(cursor);
        comma = strchr(cursor, ',');
        if (comma == 0) {
            if (i < 3) {
                return false;
            }
        } else {
            *comma = '\0';
            cursor = comma + 1;
        }
    }

    *anim = R2D_AnimFrames(
        atoi(parts[0]),
        atoi(parts[1]),
        (float)atof(parts[2]),
        R2D_AnimParseBool(parts[3])
    );
    return true;
}

bool R2D_AnimSetLoad(R2D_AnimSet *set, const char *path)
{
    char *text;
    char *cursor;

    if (set == 0 || path == 0) {
        return false;
    }

    text = R2D_LoadAssetText(path);
    if (text == 0) {
        return false;
    }

    R2D_AnimSetInit(set);
    cursor = text;
    while (*cursor != '\0') {
        char line[256];
        char *equals;
        char *key;
        char *value;
        int length = 0;
        R2D_Anim anim;

        while (cursor[length] != '\0' && cursor[length] != '\n' && length < (int)sizeof(line) - 1) {
            line[length] = cursor[length];
            ++length;
        }
        line[length] = '\0';

        while (cursor[length] != '\0' && cursor[length] != '\n') {
            ++length;
        }

        cursor += length;
        if (*cursor == '\n') {
            ++cursor;
        }

        key = R2D_AnimTrim(line);
        if (key[0] == '\0' || key[0] == '#' || key[0] == ';') {
            continue;
        }

        equals = strchr(key, '=');
        if (equals == 0) {
            continue;
        }

        *equals = '\0';
        value = R2D_AnimTrim(equals + 1);
        key = R2D_AnimTrim(key);

        if (R2D_AnimParseClip(value, &anim)) {
            R2D_AnimSetAdd(set, key, anim);
        }
    }

    R2D_UnloadAssetText(text);
    return set->count > 0;
}

void R2D_AnimPlay(R2D_AnimPlayer *player, R2D_Anim anim)
{
    if (player == 0) {
        return;
    }

    player->anim = anim;
    player->time = 0.0f;
    player->frame = 0;
    player->playing = true;
}

bool R2D_AnimPlayNamed(R2D_AnimPlayer *player, const R2D_AnimSet *set, const char *name, R2D_Anim fallback)
{
    const R2D_Anim *anim = R2D_AnimSetFind(set, name);

    R2D_AnimPlay(player, anim != 0 ? *anim : fallback);
    return anim != 0;
}

void R2D_AnimStop(R2D_AnimPlayer *player)
{
    if (player != 0) {
        player->playing = false;
    }
}

void R2D_AnimUpdate(R2D_AnimPlayer *player, float dt)
{
    int frame;

    if (player == 0 || !player->playing || player->anim.frame_count <= 1) {
        return;
    }

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    player->time += dt;
    frame = (int)floorf(player->time * player->anim.fps);

    if (player->anim.loop) {
        player->frame = frame % player->anim.frame_count;
    } else if (frame >= player->anim.frame_count) {
        player->frame = player->anim.frame_count - 1;
        player->playing = false;
    } else {
        player->frame = frame;
    }
}

int R2D_AnimFrame(const R2D_AnimPlayer *player)
{
    if (player == 0) {
        return 0;
    }

    return player->anim.first_frame + player->frame;
}

void R2D_DrawSprite(Texture2D texture, Rectangle source, Vector2 position, bool flip_x)
{
    R2D_DrawSpriteEx(
        texture,
        source,
        position,
        (Vector2) { 0.0f, 0.0f },
        0.0f,
        1.0f,
        flip_x,
        WHITE
    );
}

void R2D_DrawSpriteEx(
    Texture2D texture,
    Rectangle source,
    Vector2 position,
    Vector2 origin,
    float rotation,
    float scale,
    bool flip_x,
    Color tint
)
{
    Rectangle src = source;
    const Rectangle dest = {
        position.x,
        position.y,
        fabsf(source.width) * scale,
        fabsf(source.height) * scale
    };

    if (!IsTextureValid(texture)) {
        return;
    }

    if (flip_x) {
        src.x += src.width;
        src.width *= -1.0f;
    }

    DrawTexturePro(texture, src, dest, origin, rotation, tint);
}

void R2D_DrawSpriteCamera(const R2D_Camera *camera, Texture2D texture, Rectangle source, Vector2 position, bool flip_x)
{
    R2D_DrawSprite(texture, source, R2D_CameraWorldToPixelScreen(camera, position), flip_x);
}

void R2D_DrawSpriteExCamera(
    const R2D_Camera *camera,
    Texture2D texture,
    Rectangle source,
    Vector2 position,
    Vector2 origin,
    float rotation,
    float scale,
    bool flip_x,
    Color tint
)
{
    R2D_DrawSpriteEx(texture, source, R2D_CameraWorldToPixelScreen(camera, position), origin, rotation, scale, flip_x, tint);
}

void R2D_DrawAtlasFrame(const R2D_SpriteAtlas *atlas, const char *name, Vector2 position, bool flip_x)
{
    R2D_DrawAtlasFrameEx(atlas, R2D_SpriteAtlasFind(atlas, name), position, 0.0f, 1.0f, flip_x, WHITE);
}

void R2D_DrawAtlasFrameEx(
    const R2D_SpriteAtlas *atlas,
    const R2D_SpriteAtlasFrame *frame,
    Vector2 position,
    float rotation,
    float scale,
    bool flip_x,
    Color tint
)
{
    if (atlas == 0 || frame == 0 || !IsTextureValid(atlas->texture)) {
        return;
    }

    R2D_DrawSpriteEx(atlas->texture, frame->source, position, frame->pivot, rotation, scale, flip_x, tint);
}

void R2D_DrawAtlasFrameExCamera(
    const R2D_Camera *camera,
    const R2D_SpriteAtlas *atlas,
    const R2D_SpriteAtlasFrame *frame,
    Vector2 position,
    float rotation,
    float scale,
    bool flip_x,
    Color tint
)
{
    R2D_DrawAtlasFrameEx(atlas, frame, R2D_CameraWorldToPixelScreen(camera, position), rotation, scale, flip_x, tint);
}

void R2D_DrawSheetFrame(const R2D_SpriteSheet *sheet, int frame, Vector2 position, bool flip_x)
{
    if (!R2D_SpriteSheetIsReady(sheet)) {
        return;
    }

    R2D_DrawSprite(sheet->texture, R2D_SpriteSheetFrame(sheet, frame), position, flip_x);
}

void R2D_DrawSheetFrameCamera(const R2D_Camera *camera, const R2D_SpriteSheet *sheet, int frame, Vector2 position, bool flip_x)
{
    R2D_DrawSheetFrame(sheet, frame, R2D_CameraWorldToPixelScreen(camera, position), flip_x);
}

void R2D_DrawAnim(const R2D_SpriteSheet *sheet, const R2D_AnimPlayer *player, Vector2 position, bool flip_x)
{
    if (player == 0) {
        return;
    }

    R2D_DrawSheetFrame(sheet, R2D_AnimFrame(player), position, flip_x);
}

void R2D_DrawAnimCamera(const R2D_Camera *camera, const R2D_SpriteSheet *sheet, const R2D_AnimPlayer *player, Vector2 position, bool flip_x)
{
    if (player == 0) {
        return;
    }

    R2D_DrawSheetFrameCamera(camera, sheet, R2D_AnimFrame(player), position, flip_x);
}
