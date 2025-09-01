#pragma once

#include "Snapshot.h"
#include "Geometry.h"
#include "mapping_common.h"

struct borders
{
    ScreenCorners corners;

    Line screen_top;
    Line screen_bot;
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
std::optional<PointF> map_snapshot_to_cursor(const Snapshot &snapshot, const screen_constants &screen_consts);

/** @brief map a point from the dfrobot coordinate system to the screen coordinate system
 * @note this function is used for debugging purposes (Showing the 4 dfrobot points on the screen) */
PointF map_snapshot_debug(const Point &dfrobot, const screen_constants &screen_consts);


std::optional<borders> map_snapshot_to_borders(const Snapshot &snapshot);
