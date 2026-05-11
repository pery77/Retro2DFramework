#include "r2d/r2d.h"

#include <math.h>

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

void R2D_DrawSheetFrame(const R2D_SpriteSheet *sheet, int frame, Vector2 position, bool flip_x)
{
    if (!R2D_SpriteSheetIsReady(sheet)) {
        return;
    }

    R2D_DrawSprite(sheet->texture, R2D_SpriteSheetFrame(sheet, frame), position, flip_x);
}

void R2D_DrawAnim(const R2D_SpriteSheet *sheet, const R2D_AnimPlayer *player, Vector2 position, bool flip_x)
{
    if (player == 0) {
        return;
    }

    R2D_DrawSheetFrame(sheet, R2D_AnimFrame(player), position, flip_x);
}
