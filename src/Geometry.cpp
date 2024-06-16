#include "Geometry.h"

LineSegment::LineSegment(const PointF &p1, const PointF &p2)
    : p1(p1),
      p2(p2)
{
}

Line::Line()
{
}

std::optional<Line> Line::from_points(const PointF &p1, const PointF &p2)
{
    if (p1.x == p2.x && p1.y == p2.y)
    {
        return std::nullopt;
    }

    Line result;

    if (p1.x == p2.x)
    {
        result._x_const = p1.x;
        return result;
    }

    if (p1.y == p2.y)
    {
        result._m = 0;
        result._n = p1.y;
        return result;
    }

    result._m = (p1.y - p2.y) / (p1.x - p2.x);
    result._n = p1.y - result._m.value() * p1.x;
    return result;
}

Line::Line(float m, float n)
    : _m(m),
      _n(n)
{
}

Line::Line(const PointF &point, float m)
    : _m(m),
      _n(point.y - m * point.x)
{
}

Line Line::vertical(float x_const)
{
    Line result;
    result._x_const = x_const;
    return result;
}

std::optional<float> Line::y(float x) const
{
    if (is_vertical())
    {
        // vertical line
        // we can't map x to y
        return std::nullopt;
    }

    if (is_horizontal())
    {
        // horizontal line
        // for any x, y is constant and equal to _n
        return _n;
    }

    return _m.value() * x + _n.value();
}

std::optional<float> Line::x(float y) const
{
    if (is_vertical())
    {
        // vertical line
        // for any y, x is constant and equal to _x_const
        return _x_const;
    }

    if (is_horizontal())
    {
        // horizontal line
        // we can't map y to x
        return std::nullopt;
    }

    if (_m.value() == 0)
    {
        // horizontal line
        // we can't map y to x
        return std::nullopt;
    }

    return (y - _n.value()) / _m.value();
}

std::optional<PointF> Line::intersection(const Line &other) const
{
    if (is_vertical() && other.is_vertical())
    {
        // parallel vertical lines
        return std::nullopt;
    }

    if (!is_vertical() && !other.is_vertical() && (_m.value() == other._m.value()))
    {
        // parallel non-vertical lines
        return std::nullopt;
    }

    if (is_vertical())
    {
        float x = _x_const.value();
        auto opt_y = other.y(x);
        if (!opt_y.has_value())
        {
            return std::nullopt;
        }

        return PointF{x, opt_y.value()};
    }

    if (other.is_vertical())
    {
        float x = other._x_const.value();
        auto opt_y = y(x);
        if (!opt_y.has_value())
        {
            return std::nullopt;
        }

        return PointF{x, opt_y.value()};
    }

    float x = (other._n.value() - _n.value()) / (_m.value() - other._m.value());
    auto opt_y = y(x);
    if (!opt_y.has_value())
    {
        return std::nullopt;
    }

    return PointF{x, opt_y.value()};
}

Line Line::perpendicular(const PointF &point) const
{
    if (is_vertical())
    {
        return Line(point, 0);
    }

    if (is_horizontal())
    {
        return Line::vertical(point.x);
    }

    // multiplication of perpendicular line slopes is -1
    float m = -1 / _m.value();
    return Line(point, m);
}

std::optional<PointF> Line::perpendicular_foot(const PointF &point) const
{
    return intersection(perpendicular(point));
}
