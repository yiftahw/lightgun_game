#pragma once

#include "Snapshot.h"

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

/** @brief map a dfrobot camera snapshot to a cursor position */
PointF map_snapshot_to_cursor(const Snapshot& snapshot, const screen_constants &screen_consts);

/** @brief map a point from the dfrobot coordinate system to the screen coordinate system
  * @note this function is used for debugging purposes (Showing the 4 dfrobot points on the screen) */ 
PointF map_snapshot_debug(const Point &dfrobot, const screen_constants &screen_consts);
