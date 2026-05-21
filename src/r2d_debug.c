#include "r2d/r2d.h"

#include <stdio.h>

R2D_DebugInfo R2D_DebugInfoDefault(void)
{
    R2D_DebugInfo info = { 0 };

    info.title = "DEBUG";
    info.fps = GetFPS();
    info.frame_ms = GetFrameTime() * 1000.0f;
    info.tile_x = -1;
    info.tile_y = -1;
    info.tile_gid = 0;
    info.asset_count = R2D_AssetPackEntryCount();

    return info;
}

const char *R2D_DebugFormatBytes(size_t bytes, char *buffer, int buffer_size)
{
    const char *units[] = { "B", "KB", "MB", "GB" };
    float value = (float)bytes;
    int unit = 0;

    if (buffer == 0 || buffer_size <= 0) {
        return "";
    }

    while (value >= 1024.0f && unit < 3) {
        value /= 1024.0f;
        ++unit;
    }

    if (unit == 0) {
        snprintf(buffer, (size_t)buffer_size, "%u %s", (unsigned int)bytes, units[unit]);
    } else {
        snprintf(buffer, (size_t)buffer_size, "%.1f %s", value, units[unit]);
    }

    return buffer;
}

void R2D_DebugDrawOverlay(const R2D_DebugInfo *info, int x, int y)
{
    char line[128];
    char bytes[32];
    const int width = 154;
    const int height = info != 0 && info->line != 0 && info->line[0] != '\0' ? 74 : 64;

    if (info == 0) {
        return;
    }

    DrawRectangle(x, y, width, height, R2D_ColorFromHex(0x101820dd));
    DrawRectangleLines(x, y, width, height, R2D_ColorFromHex(0x8ecae6ff));

    DrawText(info->title != 0 ? info->title : "DEBUG", x + 6, y + 5, 8, R2D_ColorFromHex(0xffd166ff));

    snprintf(line, sizeof(line), "fps %d  %.2f ms", info->fps, info->frame_ms);
    DrawText(line, x + 6, y + 17, 8, R2D_ColorFromHex(0xf8f8f2ff));

    snprintf(line, sizeof(line), "entities %d  assets %d", info->entity_count, info->asset_count);
    DrawText(line, x + 6, y + 28, 8, R2D_ColorFromHex(0xf8f8f2ff));

    snprintf(line, sizeof(line), "mem %s", R2D_DebugFormatBytes(info->memory_bytes, bytes, sizeof(bytes)));
    DrawText(line, x + 6, y + 39, 8, R2D_ColorFromHex(0xf8f8f2ff));

    snprintf(line, sizeof(line), "tile %d,%d  gid %u", info->tile_x, info->tile_y, info->tile_gid);
    DrawText(line, x + 6, y + 50, 8, R2D_ColorFromHex(0xf8f8f2ff));

    if (info->line != 0 && info->line[0] != '\0') {
        DrawText(info->line, x + 6, y + 61, 8, R2D_ColorFromHex(0x8ecae6ff));
    }
}
