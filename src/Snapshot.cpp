#include "Snapshot.h"

std::string Point::to_string() const
{
    return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
}

std::string PointF::to_string() const
{
    return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
}

Point point_from_string(const std::string &str)
{
    Point p;
    size_t comma = str.find(',');
    p.x = std::stoi(str.substr(1, comma - 1));
    p.y = std::stoi(str.substr(comma + 1, str.size() - 2));
    return p;
}

std::string Snapshot::to_string() const
{
    std::string str = "[";
    for (const auto &point : points)
    {
        str += point.to_string() + ",";
    }
    str.pop_back(); // remove the last comma
    str += "]";
    return str;
}

Snapshot snapshot_from_string(const std::string &input)
{
    Snapshot result;

    // Remove leading and trailing '[' and ']'
    std::string data = input.substr(1, input.size() - 2);

    std::stringstream ss(data);
    char discard;
    for (int i = 0; i < 4; ++i)
    {
        ss >> discard; // discard '('
        ss >> result.points[i].x;
        ss >> discard; // discard ','
        ss >> result.points[i].y;
        ss >> discard; // discard ')'
        ss >> discard; // discard ','
    }

    return result;
}
