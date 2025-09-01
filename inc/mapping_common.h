#pragma once

#include <optional>
#include "Geometry.h"

struct screen_constants
{
    float effective_width;
    float effective_height;

    screen_constants(int screen_width, int screen_height, float screen_scale)
    {
        effective_width = (screen_width / screen_scale);
        effective_height = (screen_height / screen_scale);
    }
};

struct ScreenCorners
{
    ScreenCorners(float width, float height)
        : top_left(0, 0), top_right(width, 0), bot_left(0, height), bot_right(width, height) {}

    ScreenCorners(PointF tl, PointF tr, PointF bl, PointF br)
        : top_left(tl), top_right(tr), bot_left(bl), bot_right(br) {}

    PointF top_left;
    PointF top_right;
    PointF bot_left;
    PointF bot_right;
};

std::optional<ScreenCorners> calculate_screen_corners(const Snapshot &snapshot);
