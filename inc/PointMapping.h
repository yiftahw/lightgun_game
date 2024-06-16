#pragma once

#include "Snapshot.h"
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

struct borders
{
    ScreenCorners corners;

    Line screen_left;
    Line screen_right;
    Line cursor_horizontal;
    Line cursor_vertical;

    LineSegment screen_top_segment;
    LineSegment screen_bot_segment;
    LineSegment screen_left_segment;
    LineSegment screen_right_segment;
    LineSegment cursor_horizontal_segment;
    LineSegment cursor_vertical_segment;
};

/** @brief map a dfrobot camera snapshot to a cursor position */
PointF map_snapshot_to_cursor(const Snapshot &snapshot, const screen_constants &screen_consts);

/** @brief map a point from the dfrobot coordinate system to the screen coordinate system
 * @note this function is used for debugging purposes (Showing the 4 dfrobot points on the screen) */
PointF map_snapshot_debug(const Point &dfrobot, const screen_constants &screen_consts);

std::optional<ScreenCorners> calculate_screen_corners(const Snapshot &snapshot);

std::optional<borders> map_snapshot_to_borders(const Snapshot &snapshot);
