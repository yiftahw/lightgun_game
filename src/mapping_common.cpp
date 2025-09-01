#include "mapping_common.h"

ScreenCorners::ScreenCorners(const PointF &top_left, const PointF &top_right, const PointF &bot_left, const PointF &bot_right)
    : top_left(top_left)
    , top_right(top_right)
    , bot_left(bot_left)
    , bot_right(bot_right)
    , top(Line::from_points(top_left, top_right))
    , bot(Line::from_points(bot_left, bot_right))
    , left(Line::from_points(top_left, bot_left))
    , right(Line::from_points(top_right, bot_right))
{
}

namespace 
{
    ScreenCorners calculate_corners(const Snapshot &snapshot)
    {
        PointF avg = {0, 0};
        for (const auto &tmp : snapshot.points)
        {
            avg.x += tmp.x;
            avg.y += tmp.y;
        }
        avg.x /= snapshot.points.size();
        avg.y /= snapshot.points.size();

        // map the snapshot to corners relative to the average point
        std::optional<PointF> opt_cam_top_left;
        std::optional<PointF> opt_cam_top_right;
        std::optional<PointF> opt_cam_bot_left;
        std::optional<PointF> opt_cam_bot_right;

        for (const auto &tmp : snapshot.points)
        {
            PointF point = {static_cast<float>(tmp.x), static_cast<float>(tmp.y)};
            if (point.x < avg.x && point.y < avg.y)
            {
                opt_cam_top_left = point;
            }
            if (point.x > avg.x && point.y < avg.y)
            {
                opt_cam_top_right = point;
            }
            if (point.x < avg.x && point.y > avg.y)
            {
                opt_cam_bot_left = point;
            }
            if (point.x > avg.x && point.y > avg.y)
            {
                opt_cam_bot_right = point;
            }
        }

        if (!opt_cam_top_left.has_value() || !opt_cam_top_right.has_value() || !opt_cam_bot_left.has_value() || !opt_cam_bot_right.has_value())
        {
            throw std::runtime_error("Failed to map the snapshot to 4 corners");
        }

        const PointF &cam_top_left = opt_cam_top_left.value();
        const PointF &cam_top_right = opt_cam_top_right.value();
        const PointF &cam_bot_left = opt_cam_bot_left.value();
        const PointF &cam_bot_right = opt_cam_bot_right.value();

        // now that we have the 4 points mapped, we can create 2 horizontal line equations
        // we know that the length between 2 horizontal pairs is constant (the WII IR Sensor Bar size)
        auto opt_top_line = Line::from_points(cam_top_left, cam_top_right);
        auto opt_bot_line = Line::from_points(cam_bot_left, cam_bot_right);
        if (!opt_top_line.has_value() || !opt_bot_line.has_value())
        {
            throw std::runtime_error("Failed to create the 2 horizontal lines");
        }

        const Line &top_line = opt_top_line.value();
        const Line &bot_line = opt_bot_line.value();

        // get the average point of the 2 horizontal pairs
        // this is the horizontal center of the screen relative to the IR camera
        PointF top_avg = {float(cam_top_left.x + cam_top_right.x) / 2, float(cam_top_left.y + cam_top_right.y) / 2};
        PointF bot_avg = {float(cam_bot_left.x + cam_bot_right.x) / 2, float(cam_bot_left.y + cam_bot_right.y) / 2};

        // the distance between any average point and a point on the same horizontal line is half the width of the WII IR Sensor Bar
        // we cross multiply to get the step width and calculate the screen corner points
        float ratio_top = (cam_top_right.x - top_avg.x) / (wii_ir_led_width_cm / 2);
        float ratio_bot = (cam_bot_right.x - bot_avg.x) / (wii_ir_led_width_cm / 2);

        constexpr float screen_half_width_cm = screen_width_cm / 2;
        float x_diff_top = std::abs(screen_half_width_cm * ratio_top);
        float x_diff_bot = std::abs(screen_half_width_cm * ratio_bot);

        // now that we have the step width, we can calculate the screen end points
        float x_top_left = top_avg.x - x_diff_top;
        float x_top_right = top_avg.x + x_diff_top;
        float x_bot_left = bot_avg.x - x_diff_bot;
        float x_bot_right = bot_avg.x + x_diff_bot;

        std::optional<float> opt_y_top_left = top_line.y(x_top_left);
        std::optional<float> opt_y_top_right = top_line.y(x_top_right);
        std::optional<float> opt_y_bot_left = bot_line.y(x_bot_left);
        std::optional<float> opt_y_bot_right = bot_line.y(x_bot_right);

        if (!opt_y_top_left.has_value() || !opt_y_top_right.has_value() || !opt_y_bot_left.has_value() || !opt_y_bot_right.has_value())
        {
            throw std::runtime_error("Failed to calculate the screen end points");
        }

        PointF screen_top_left = {x_top_left, opt_y_top_left.value()};
        PointF screen_top_right = {x_top_right, opt_y_top_right.value()};
        PointF screen_bot_left = {x_bot_left, opt_y_bot_left.value()};
        PointF screen_bot_right = {x_bot_right, opt_y_bot_right.value()};

        return ScreenCorners(screen_top_left, screen_top_right, screen_bot_left, screen_bot_right);
    }
}

std::optional<ScreenCorners> calculate_screen_corners(const Snapshot &snapshot)
{
    try
    {
        return calculate_corners(snapshot);
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "Error: %s\n", e.what());
    }
    return std::nullopt;
}
