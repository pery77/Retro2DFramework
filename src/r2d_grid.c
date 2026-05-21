#include "r2d/r2d.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct R2D_GridSearchNode {
    int x;
    int y;
    int parent;
    int g;
    int h;
    bool open;
    bool closed;
    bool used;
} R2D_GridSearchNode;

R2D_GridPoint R2D_GridPointMake(int x, int y)
{
    return (R2D_GridPoint) { x, y };
}

int R2D_GridManhattanDistance(R2D_GridPoint a, R2D_GridPoint b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

float R2D_GridEuclideanDistance(R2D_GridPoint a, R2D_GridPoint b)
{
    const float dx = (float)(a.x - b.x);
    const float dy = (float)(a.y - b.y);

    return sqrtf(dx * dx + dy * dy);
}

static bool R2D_GridInside(int x, int y, int width, int height)
{
    return x >= 0 && y >= 0 && x < width && y < height;
}

bool R2D_GridLineOfSight(R2D_GridPoint start, R2D_GridPoint end, R2D_GridBlockedCallback blocked, void *user_data)
{
    int x0 = start.x;
    int y0 = start.y;
    const int x1 = end.x;
    const int y1 = end.y;
    const int dx = abs(x1 - x0);
    const int dy = abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    for (;;) {
        if (blocked != 0 && blocked(x0, y0, user_data)) {
            return false;
        }

        if (x0 == x1 && y0 == y1) {
            return true;
        }

        {
            const int e2 = err * 2;

            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }

            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
}

int R2D_GridFloodFill(
    R2D_GridPoint start,
    int width,
    int height,
    R2D_GridBlockedCallback blocked,
    void *user_data,
    R2D_GridPoint *out_points,
    int max_points
)
{
    R2D_GridPoint queue[R2D_GRID_MAX_SEARCH_NODES];
    unsigned char visited[R2D_GRID_MAX_SEARCH_NODES];
    int head = 0;
    int tail = 0;
    int count = 0;

    if (width <= 0 || height <= 0 || width * height > R2D_GRID_MAX_SEARCH_NODES ||
        out_points == 0 || max_points <= 0 || !R2D_GridInside(start.x, start.y, width, height) ||
        (blocked != 0 && blocked(start.x, start.y, user_data))) {
        return 0;
    }

    memset(visited, 0, sizeof(visited));
    queue[tail++] = start;
    visited[start.y * width + start.x] = 1;

    while (head < tail && count < max_points) {
        static const int offsets[4][2] = {
            { 1, 0 },
            { -1, 0 },
            { 0, 1 },
            { 0, -1 }
        };
        const R2D_GridPoint point = queue[head++];

        out_points[count++] = point;

        for (int i = 0; i < 4; ++i) {
            const int x = point.x + offsets[i][0];
            const int y = point.y + offsets[i][1];
            const int index = y * width + x;

            if (!R2D_GridInside(x, y, width, height) || visited[index] ||
                (blocked != 0 && blocked(x, y, user_data)) ||
                tail >= R2D_GRID_MAX_SEARCH_NODES) {
                continue;
            }

            visited[index] = 1;
            queue[tail++] = R2D_GridPointMake(x, y);
        }
    }

    return count;
}

static int R2D_GridFindNode(const R2D_GridSearchNode *nodes, int count, int x, int y)
{
    for (int i = 0; i < count; ++i) {
        if (nodes[i].used && nodes[i].x == x && nodes[i].y == y) {
            return i;
        }
    }

    return -1;
}

static int R2D_GridBestOpenNode(const R2D_GridSearchNode *nodes, int count)
{
    int best = -1;
    int best_f = 0;

    for (int i = 0; i < count; ++i) {
        const int f = nodes[i].g + nodes[i].h;

        if (!nodes[i].used || !nodes[i].open || nodes[i].closed) {
            continue;
        }

        if (best < 0 || f < best_f || (f == best_f && nodes[i].h < nodes[best].h)) {
            best = i;
            best_f = f;
        }
    }

    return best;
}

int R2D_GridAStar(
    R2D_GridPoint start,
    R2D_GridPoint goal,
    int width,
    int height,
    R2D_GridBlockedCallback blocked,
    void *user_data,
    R2D_GridPoint *out_path,
    int max_path
)
{
    R2D_GridSearchNode nodes[R2D_GRID_MAX_SEARCH_NODES];
    int node_count = 1;

    if (width <= 0 || height <= 0 || width * height > R2D_GRID_MAX_SEARCH_NODES ||
        out_path == 0 || max_path <= 0 ||
        !R2D_GridInside(start.x, start.y, width, height) ||
        !R2D_GridInside(goal.x, goal.y, width, height) ||
        (blocked != 0 && (blocked(start.x, start.y, user_data) || blocked(goal.x, goal.y, user_data)))) {
        return 0;
    }

    memset(nodes, 0, sizeof(nodes));
    nodes[0] = (R2D_GridSearchNode) {
        start.x,
        start.y,
        -1,
        0,
        R2D_GridManhattanDistance(start, goal),
        true,
        false,
        true
    };

    while (node_count < R2D_GRID_MAX_SEARCH_NODES) {
        static const int offsets[4][2] = {
            { 1, 0 },
            { -1, 0 },
            { 0, 1 },
            { 0, -1 }
        };
        const int current = R2D_GridBestOpenNode(nodes, node_count);

        if (current < 0) {
            return 0;
        }

        if (nodes[current].x == goal.x && nodes[current].y == goal.y) {
            R2D_GridPoint reversed[R2D_GRID_MAX_SEARCH_NODES];
            int length = 0;
            int cursor = current;

            while (cursor >= 0 && length < R2D_GRID_MAX_SEARCH_NODES) {
                reversed[length++] = R2D_GridPointMake(nodes[cursor].x, nodes[cursor].y);
                cursor = nodes[cursor].parent;
            }

            if (length > max_path) {
                length = max_path;
            }

            for (int i = 0; i < length; ++i) {
                out_path[i] = reversed[length - i - 1];
            }

            return length;
        }

        nodes[current].open = false;
        nodes[current].closed = true;

        for (int i = 0; i < 4; ++i) {
            const int x = nodes[current].x + offsets[i][0];
            const int y = nodes[current].y + offsets[i][1];
            const int g = nodes[current].g + 1;
            int neighbor;

            if (!R2D_GridInside(x, y, width, height) || (blocked != 0 && blocked(x, y, user_data))) {
                continue;
            }

            neighbor = R2D_GridFindNode(nodes, node_count, x, y);
            if (neighbor >= 0 && nodes[neighbor].closed) {
                continue;
            }

            if (neighbor < 0) {
                if (node_count >= R2D_GRID_MAX_SEARCH_NODES) {
                    break;
                }

                neighbor = node_count++;
                nodes[neighbor] = (R2D_GridSearchNode) {
                    x,
                    y,
                    current,
                    g,
                    R2D_GridManhattanDistance(R2D_GridPointMake(x, y), goal),
                    true,
                    false,
                    true
                };
            } else if (g < nodes[neighbor].g) {
                nodes[neighbor].parent = current;
                nodes[neighbor].g = g;
                nodes[neighbor].open = true;
            }
        }
    }

    return 0;
}

