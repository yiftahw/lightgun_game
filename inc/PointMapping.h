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

PointF map_snapshot_to_screen(const Snapshot& snapshot, const screen_constants &screen_consts);
