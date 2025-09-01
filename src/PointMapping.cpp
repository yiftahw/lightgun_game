#include <optional>
#include <exception>
#include "PointMapping.h"
#include "consts.h"
#include "SafeDivide.h"
#include "Geometry.h"
#include "mapping_common.h"

namespace
{
    borders calculate_borders(const Snapshot &snapshot)
    {
        auto opt_corners = calculate_screen_corners(snapshot);
        if (!opt_corners.has_value())
        {
            throw std::runtime_error("Failed to calculate the screen corners");
        }
        const auto &corners = opt_corners.value();

        const PointF &screen_top_left = corners.top_left;
        const PointF &screen_top_right = corners.top_right;
        const PointF &screen_bot_left = corners.bot_left;
        const PointF &screen_bot_right = corners.bot_right;

        auto opt_top_line = Line::from_points(screen_top_left, screen_top_right);
        auto opt_bot_line = Line::from_points(screen_bot_left, screen_bot_right);
        auto opt_left_line = Line::from_points(screen_top_left, screen_bot_left);
        auto opt_right_line = Line::from_points(screen_top_right, screen_bot_right);

        if (!opt_top_line.has_value() || !opt_bot_line.has_value() || !opt_left_line.has_value() || !opt_right_line.has_value())
        {
            throw std::runtime_error("Failed to calculate the screen border lines");
        }

        const Line &top_line = opt_top_line.value();
        const Line &bot_line = opt_bot_line.value();
        const Line &left_line = opt_left_line.value();
        const Line &right_line = opt_right_line.value();

        // to compensate for the camera tilt, we need to calculate a slope for the camera mid point which is the "cursor"
        auto compensated_slope = [](
            const Line &line1,
            const PointF &line1_start,
            const Line &Line2,
            const PointF &line2_start,
            const PointF &camera_point) -> float
        {
            if (line1.is_vertical() || Line2.is_vertical())
            {
                throw std::runtime_error("one of the lines is vertical");
            }

            PointF slope_start = {line1_start.y, line1._m.value()};
            PointF slope_end = {line2_start.y, Line2._m.value()};

            auto opt_slope_line = Line::from_points(slope_start, slope_end);
            if (!opt_slope_line.has_value())
            {
                throw std::runtime_error("Failed to calculate the slope line");
            }
            const Line &slope_line = opt_slope_line.value();

            auto opt_slope = slope_line.y(camera_point.y);
            if (!opt_slope.has_value())
            {
                throw std::runtime_error("Failed to calculate the slope");
            }
            return opt_slope.value();
        };

        // vertical position compensation
        constexpr PointF ir_camera_mid = {static_cast<float>(ir_camera_centers[0]), static_cast<float>(ir_camera_centers[1])};
        float horizontal_intersection_slope = compensated_slope(
            top_line, screen_top_left,
            bot_line, screen_bot_left,
            ir_camera_mid);

        Line horizontal_camera_line = Line(ir_camera_mid, horizontal_intersection_slope);

        // horizontal position compensation
        // to avoid handling vertical lines (with undefined slope), we calculate with inverted x and y axis
        PointF screen_top_left_inverted{screen_top_left.y, screen_top_left.x};
        PointF screen_top_right_inverted{screen_top_right.y, screen_top_right.x};
        PointF screen_bot_left_inverted{screen_bot_left.y, screen_bot_left.x};
        PointF screen_bot_right_inverted{screen_bot_right.y, screen_bot_right.x};

        auto opt_left_line_inverted = Line::from_points(screen_top_left_inverted, screen_bot_left_inverted);
        auto opt_right_line_inverted = Line::from_points(screen_top_right_inverted, screen_bot_right_inverted);
        auto opt_bot_line_inverted = Line::from_points(screen_bot_left_inverted, screen_bot_right_inverted);
        if (!opt_left_line_inverted.has_value() || !opt_right_line_inverted.has_value() || !opt_bot_line_inverted.has_value())
        {
            throw std::runtime_error("Failed to calculate the inverted screen lines");
        }
        const Line &left_line_inverted = opt_left_line_inverted.value();
        const Line &right_line_inverted = opt_right_line_inverted.value();

        constexpr PointF ir_camera_mid_inverted = {ir_camera_mid.y, ir_camera_mid.x};

        float inverted_vertical_intersection_slope = compensated_slope(
            left_line_inverted, screen_top_left_inverted,
            right_line_inverted, screen_top_right_inverted,
            ir_camera_mid_inverted);

        float vertical_intersection_slope = SafeDivide(1.0F, inverted_vertical_intersection_slope);

        Line vertical_camera_line = Line(ir_camera_mid, vertical_intersection_slope);

        auto opt_intersect_top = vertical_camera_line.intersection(top_line);
        auto opt_intersect_bot = vertical_camera_line.intersection(bot_line);
        auto opt_intersect_left = horizontal_camera_line.intersection(left_line);
        auto opt_intersect_right = horizontal_camera_line.intersection(right_line);
        if (!opt_intersect_top.has_value() || !opt_intersect_bot.has_value() || !opt_intersect_left.has_value() || !opt_intersect_right.has_value())
        {
            throw std::runtime_error("Failed to calculate the intersection points");
        }

        LineSegment top_segment(screen_top_left, screen_top_right);
        LineSegment bot_segment(screen_bot_left, screen_bot_right);
        LineSegment left_segment(screen_top_left, screen_bot_left);
        LineSegment right_segment(screen_top_right, screen_bot_right);
        LineSegment cursor_horizontal(opt_intersect_left.value(), opt_intersect_right.value());
        LineSegment cursor_vertical(opt_intersect_top.value(), opt_intersect_bot.value());

        return {corners,
                top_line,
                bot_line,
                left_line,
                right_line,
                horizontal_camera_line,
                vertical_camera_line,
                top_segment, 
                bot_segment,
                left_segment,
                right_segment,
                cursor_horizontal,
                cursor_vertical};
    }

    PointF map(const Snapshot &snapshot, const ScreenCorners &screen_corners)
    {
        PointF result;

        borders screen_borders = calculate_borders(snapshot); 

        auto &vertical_camera_line = screen_borders.cursor_vertical;
        auto &horizontal_camera_line = screen_borders.cursor_horizontal;
        auto &bot_line = screen_borders.screen_bot;
        auto &left_line = screen_borders.screen_left;
        
        auto &screen_top_left = screen_borders.corners.top_left;
        auto &screen_top_right = screen_borders.corners.top_right;
        auto &screen_bot_left = screen_borders.corners.bot_left;

        auto opt_intersect_vertical = vertical_camera_line.intersection(bot_line);
        if (!opt_intersect_vertical.has_value())
        {
            throw std::runtime_error("Failed to calculate the horizontal intersection point");
        }
        const PointF &intersect_bot = opt_intersect_vertical.value();

        auto opt_intersect_horizontal = horizontal_camera_line.intersection(left_line);
        if (!opt_intersect_horizontal.has_value())
        {
            throw std::runtime_error("Failed to calculate the vertical intersection point");
        }
        const PointF &intersect_left = opt_intersect_horizontal.value();

        // // calculate the percentage of the intersection points relative to the screen pixels
        float x_percentage = SafeDivide((intersect_bot.x - screen_top_left.x), (screen_top_right.x - screen_top_left.x));
        float y_percentage = SafeDivide((intersect_left.y - screen_top_left.y), (screen_bot_left.y - screen_top_left.y));

        // // calculate the cursor
        const auto screen_width = screen_corners.top_right.x - screen_corners.top_left.x;
        const auto screen_height = screen_corners.bot_left.y - screen_corners.top_left.y;
        result.x = x_percentage * screen_width;
        result.y = y_percentage * screen_height;

        // // invert the y axis
        result.y = screen_height - result.y;

        return result;
    }
}

std::optional<PointF> map_snapshot_to_cursor(const Snapshot &snapshot, const ScreenCorners &screen_corners)
{
    try
    {
        return map(snapshot, screen_corners);
    }
    catch (const std::exception &e)
    {
        printf("Error: %s\n", e.what());
    }
    return std::nullopt;
}

PointF map_snapshot_debug(const Point &point, const screen_constants &constants)
{
    float x_mapped = ((static_cast<float>(point.x) * constants.effective_width) / dfrobot_max_unit_x);
    float y_mapped = ((static_cast<float>(point.y) * constants.effective_height) / dfrobot_max_unit_y);

    // to make the y axis go from top to bot, we need to invert it
    y_mapped = constants.effective_height - y_mapped;

    return {x_mapped, y_mapped};
}

std::optional<borders> map_snapshot_to_borders(const Snapshot &snapshot)
{
    try
    {
        return calculate_borders(snapshot);
    }
    catch (const std::exception &e)
    {
        printf("Error: %s\n", e.what());
    }
    return std::nullopt;
}
