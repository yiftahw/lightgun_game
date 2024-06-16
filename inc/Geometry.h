#pragma once

#include <optional>
#include "Snapshot.h"

class LineSegment
{
public:
    explicit LineSegment(const PointF &p1, const PointF &p2);
    PointF p1;
    PointF p2;
private:
    LineSegment() = delete;
};

class Line
{
public:
    Line(float m, float n);
    Line(const PointF& point, float m);

    static Line vertical(float x_const);
    static std::optional<Line> from_points(const PointF &p1, const PointF &p2);

    std::optional<float> y(float x) const;
    std::optional<float> x(float y) const;
    std::optional<PointF> intersection(const Line &other) const;
    std::optional<PointF> perpendicular_foot(const PointF &point) const;
    Line perpendicular(const PointF &point) const;

    std::optional<float> _m;
    std::optional<float> _n;
    std::optional<float> _x_const;
    inline constexpr bool is_horizontal() const { return _m.has_value() && _m.value() == 0; }
    inline constexpr bool is_vertical() const { return _x_const.has_value(); }

private:
    Line();
};
