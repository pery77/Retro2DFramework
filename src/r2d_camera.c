#include "r2d/r2d.h"

#include <math.h>

R2D_Camera R2D_CameraCreate(int viewport_width, int viewport_height)
{
    if (viewport_width < 1) {
        viewport_width = R2D_DEFAULT_VIRTUAL_WIDTH;
    }

    if (viewport_height < 1) {
        viewport_height = R2D_DEFAULT_VIRTUAL_HEIGHT;
    }

    return (R2D_Camera) {
        { 0.0f, 0.0f },
        viewport_width,
        viewport_height
    };
}

void R2D_CameraFollow(R2D_Camera *camera, Vector2 target)
{
    if (camera == 0) {
        return;
    }

    camera->position = (Vector2) {
        target.x - (float)camera->viewport_width * 0.5f,
        target.y - (float)camera->viewport_height * 0.5f
    };
}

void R2D_CameraClampToRect(R2D_Camera *camera, Rectangle bounds)
{
    float max_x;
    float max_y;

    if (camera == 0) {
        return;
    }

    max_x = bounds.x + bounds.width - (float)camera->viewport_width;
    max_y = bounds.y + bounds.height - (float)camera->viewport_height;

    if (bounds.width <= (float)camera->viewport_width) {
        camera->position.x = bounds.x + (bounds.width - (float)camera->viewport_width) * 0.5f;
    } else if (camera->position.x < bounds.x) {
        camera->position.x = bounds.x;
    } else if (camera->position.x > max_x) {
        camera->position.x = max_x;
    }

    if (bounds.height <= (float)camera->viewport_height) {
        camera->position.y = bounds.y + (bounds.height - (float)camera->viewport_height) * 0.5f;
    } else if (camera->position.y < bounds.y) {
        camera->position.y = bounds.y;
    } else if (camera->position.y > max_y) {
        camera->position.y = max_y;
    }
}

Vector2 R2D_CameraPixelPosition(const R2D_Camera *camera)
{
    if (camera == 0) {
        return (Vector2) { 0.0f, 0.0f };
    }

    return (Vector2) {
        floorf(camera->position.x),
        floorf(camera->position.y)
    };
}

Vector2 R2D_CameraWorldToScreen(const R2D_Camera *camera, Vector2 world)
{
    if (camera == 0) {
        return world;
    }

    return (Vector2) {
        world.x - camera->position.x,
        world.y - camera->position.y
    };
}

Vector2 R2D_CameraScreenToWorld(const R2D_Camera *camera, Vector2 screen)
{
    if (camera == 0) {
        return screen;
    }

    return (Vector2) {
        screen.x + camera->position.x,
        screen.y + camera->position.y
    };
}

Rectangle R2D_CameraView(const R2D_Camera *camera)
{
    if (camera == 0) {
        return (Rectangle) { 0 };
    }

    return (Rectangle) {
        camera->position.x,
        camera->position.y,
        (float)camera->viewport_width,
        (float)camera->viewport_height
    };
}
