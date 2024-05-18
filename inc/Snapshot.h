#pragma once

#include <array>
#include <stdint.h>
#include <string>
#include <sstream>

struct Point
{
    uint16_t x;
    uint16_t y;

    std::string to_string() const;
};

Point point_from_string(const std::string &str);

struct Snapshot
{
    std::array<Point, 4> points;

    std::string to_string() const;
};

Snapshot snapshot_from_string(const std::string& input);
