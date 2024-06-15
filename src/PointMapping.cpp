#include <optional>
#include <exception>
#include "PointMapping.h"
#include "consts.h"
#include "SafeDivide.h"
#include "Geometry.h"

namespace
{
    PointF map(const Snapshot &snapshot, const screen_constants &screen_consts)
    {
        PointF result;

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
        float ratio_top = SafeDivide((cam_top_right.x - top_avg.x), (wii_ir_led_width_cm / 2));
        float ratio_bot = SafeDivide((cam_bot_right.x - bot_avg.x), (wii_ir_led_width_cm / 2));

        constexpr float screen_half_width_cm = screen_width_cm / 2;
        float x_diff_top = std::abs(top_avg.x - screen_half_width_cm * ratio_top);
        float x_diff_bot = std::abs(bot_avg.x - screen_half_width_cm * ratio_bot);

        // now that we have the step width, we can calculate the screen end points
        float x_top_left = top_avg.x - x_diff_top;
        float x_top_right = top_avg.x + x_diff_top;
        float x_bot_left = bot_avg.x - x_diff_bot;
        float x_bot_right = bot_avg.x + x_diff_bot;

        auto opt_y_top_left = top_line.y(x_top_left);
        auto opt_y_top_right = top_line.y(x_top_right);
        auto opt_y_bot_left = bot_line.y(x_bot_left);
        auto opt_y_bot_right = bot_line.y(x_bot_right);

        if (!opt_y_top_left.has_value() || !opt_y_top_right.has_value() || !opt_y_bot_left.has_value() || !opt_y_bot_right.has_value())
        {
            throw std::runtime_error("Failed to calculate the screen end points");
        }

        PointF screen_top_left = {x_top_left, opt_y_top_left.value()};
        PointF screen_top_right = {x_top_right, opt_y_top_right.value()};
        PointF screen_bot_left = {x_bot_left, opt_y_bot_left.value()};
        PointF screen_bot_right = {x_bot_right, opt_y_bot_right.value()};

        // calculate the screen vertical line equations
        auto opt_left_line = Line::from_points(screen_top_left, screen_bot_left);
        auto opt_right_line = Line::from_points(screen_top_right, screen_bot_right);
        if (!opt_left_line.has_value() || !opt_right_line.has_value())
        {
            throw std::runtime_error("Failed to calculate the screen vertical borders");
        }

        const Line &left_line = opt_left_line.value();
        // const Line &right_line = opt_right_line.value();

        // to compensate for the camera tilt, we need to calculate a slope for the camera mid point which is the "cursor"
        constexpr PointF ir_camera_mid = {float(dfrobot_max_unit_x) / 2, float(dfrobot_max_unit_y) / 2};

        if (top_line.is_vertical() || bot_line.is_vertical())
        {
            throw std::runtime_error("Top or bottom line is vertical");
        }
        PointF horizontal_slope_start = {screen_bot_left.y, bot_line._m.value()};
        PointF horizontal_slope_end = {screen_top_left.y, top_line._m.value()};
        auto opt_horizontal_slope_line = Line::from_points(horizontal_slope_start, horizontal_slope_end);
        if (!opt_horizontal_slope_line.has_value())
        {
            throw std::runtime_error("Failed to calculate the horizontal slope line");
        }

        const Line &horizontal_slope_line = opt_horizontal_slope_line.value();
        auto opt_horizontal_slope = horizontal_slope_line.y(ir_camera_mid.y);
        if (!opt_horizontal_slope.has_value())
        {
            throw std::runtime_error("Failed to calculate the horizontal slope");
        }

        float horizontal_slope = opt_horizontal_slope.value();
        Line camera_horizontal_line = Line(ir_camera_mid, horizontal_slope); 
        auto opt_intersect_left = left_line.intersection(camera_horizontal_line);
        if (!opt_intersect_left.has_value())
        {
            throw std::runtime_error("Failed to calculate the left intersection point");
        }
        const PointF &intersect_left = opt_intersect_left.value();

        // for now, only the vertical position is optimized
        // the horizontal position is calculated as a simple perpendicular foot
        // TODO: calculate the optimized vertical slope to get the accuate horizontal position
        auto opt_intersect_bot = bot_line.perpendicular_foot(ir_camera_mid);
        if (!opt_intersect_bot.has_value())
        {
            throw std::runtime_error("Failed to calculate the bottom intersection point");
        }
        const PointF &intersect_bot = opt_intersect_bot.value();

        // // calculate the percentage of the intersection points relative to the screen pixels
        float x_percentage = SafeDivide((intersect_bot.x - screen_top_left.x), (screen_top_right.x - screen_top_left.x));
        float y_percentage = SafeDivide((intersect_left.y - screen_top_left.y), (screen_bot_left.y - screen_top_left.y));
        
        // // calculate the cursor
        result.x = x_percentage * screen_consts.effective_width;
        result.y = y_percentage * screen_consts.effective_height;

        // // invert the y axis
        result.y = screen_consts.effective_height - result.y;

        return result;
    }
}

PointF map_snapshot_to_screen(const Snapshot &snapshot, const screen_constants &screen_consts)
{
    PointF point = {0, 0};
    for (const auto &p : snapshot.points)
    {
        if (p.x == 1023 && p.y == 1023) // invalid point
        {
            return point;
        }
    }

    try
    {
        point = map(snapshot, screen_consts);
    }
    catch (const std::exception &e)
    {
        printf("Error: %s\n", e.what());
    }
    return point;
}

PointF map_dfrobot_to_screen(const Point &point, const screen_constants &constants)
{
    float x_mapped = ((static_cast<float>(point.x) * constants.effective_width) / dfrobot_max_unit_x);
    float y_mapped = ((static_cast<float>(point.y) * constants.effective_height) / dfrobot_max_unit_y);

    // to make the y axis go from top to bot, we need to invert it
    y_mapped = constants.effective_height - y_mapped;

    return {x_mapped, y_mapped};
}
