#include <cstdint>
#include "DataAcqSim.h"
#include "consts.h"
#include "Snapshot.h"

namespace
{
    static inline constexpr uint16_t square_size = 50;
    static inline constexpr uint16_t point_max_value = dfrobot_max_unit - square_size;
    static inline constexpr uint16_t square_mid_point = square_size / 2;
    static inline constexpr uint16_t num_frames_per_side = square_size * 3;
    static inline constexpr uint16_t step_width = point_max_value / num_frames_per_side;

    // Create a snapshot with 4 points
    // The snapshot is a simple square that moves around the screen
    // the DFRobot max unit is 1023
    // the square is of size 50x50
    // the square will move in a square path around the screen
    static Snapshot create_snapshot(uint16_t t)
    {

        uint16_t side = t % (num_frames_per_side * 4) / num_frames_per_side;
        uint16_t step = (t % num_frames_per_side) * step_width;
        Point top_left = {0, 0};
        switch (side)
        {
        case 0:
            top_left.x = step;
            top_left.y = 0;
            break;
        case 1:
            top_left.x = point_max_value;
            top_left.y = step;
            break;
        case 2:
            top_left.x = point_max_value - step;
            top_left.y = point_max_value;
            break;
        case 3:
        default:
            top_left.x = 0;
            top_left.y = point_max_value - step;
            break;
        }
        Point top_right = {uint16_t(top_left.x + square_size), top_left.y};
        Point bottom_right = {uint16_t(top_left.x + square_size), uint16_t(top_left.y + square_size)};
        Point bottom_left = {top_left.x, uint16_t(top_left.y + square_size)};

        return {top_left, top_right, bottom_right, bottom_left};
    }
} // anonymous namespace

DataAcqSim::DataAcqSim()
    : counter(0)
{
}

DataAcqSim::~DataAcqSim()
{
}

Snapshot DataAcqSim::get()
{
    return create_snapshot(counter++);
}
