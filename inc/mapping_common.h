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

class ScreenCorners
{
public:
    explicit ScreenCorners(const PointF &top_left, const PointF &top_right, const PointF &bot_left, const PointF &bot_right, const Line &top, const Line &bottom);
    PointF top_left;
    PointF top_right;
    PointF bot_left;
    PointF bot_right;
    Line top;
    Line bot;
private:
    ScreenCorners() = delete;
};

std::optional<ScreenCorners> calculate_screen_corners(const Snapshot &snapshot);
