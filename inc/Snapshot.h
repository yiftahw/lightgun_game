#pragma once

#include <array>
#include <stdint.h>
#include <string>
#include <sstream>
#include "consts.h"

struct Point
{
    uint16_t x;
    uint16_t y;

    std::string to_string() const;
};

struct PointF
{
    float x;
    float y;

    std::string to_string() const;
};

Point point_from_string(const std::string &str);

struct Snapshot
{
    std::array<Point, dfrobot_snapshot_size> points;

    std::string to_string() const;

    static Snapshot invalid();
};

Snapshot snapshot_from_string(const std::string& input);
