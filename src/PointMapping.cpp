#include "PointMapping.h"
#include "consts.h"
#include "SafeDivide.h"

namespace
{
    // PointF map_dfrobot_to_screen(uint16_t x, uint16_t y, const screen_constants &constants)
    // {
    //     float x_mapped = ((static_cast<float>(x) * constants.effective_width) / dfrobot_max_unit);
    //     float y_mapped = ((static_cast<float>(y) * constants.effective_height) / dfrobot_max_unit);

    //     // to make the y axis go from top to bottom, we need to invert it
    //     y_mapped = constants.effective_height - y_mapped;

    //     return {x_mapped, y_mapped};
    // }

    PointF map(const Snapshot &snapshot, const screen_constants &screen_consts)
    {
        PointF result;

        // calculate the 2 axis of the screen relative to the IR camera
        // init the 4 points to their minimum values
        PointF top_left = {dfrobot_max_unit, dfrobot_max_unit};
        PointF top_right = {0, dfrobot_max_unit};
        PointF bottom_left = {dfrobot_max_unit, 0};
        PointF bottom_right = {0, 0};

        for (const auto &tmp : snapshot.points)
        {
            PointF point = {static_cast<float>(tmp.x), static_cast<float>(tmp.y)};
            if (point.x < top_left.x && point.y < top_left.y)
            {
                top_left = point;
            }
            if (point.x > top_right.x && point.y < top_right.y)
            {
                top_right = point;
            }
            if (point.x < bottom_left.x && point.y > bottom_left.y)
            {
                bottom_left = point;
            }
            if (point.x > bottom_right.x && point.y > bottom_right.y)
            {
                bottom_right = point;
            }
        }

        // now that we have the 4 points mapped, we can create 2 horizontal line equations
        // we know that the length between 2 horizontal pairs is constant (the WII IR Sensor Bar size)
        float m_top = SafeDivide((top_right.y - top_left.y), (top_right.x - top_left.x));
        float n_top = top_left.y - m_top * top_left.x;
        float m_bot = SafeDivide((bottom_right.y - bottom_left.y), (bottom_right.x - bottom_left.x));
        float n_bot = bottom_left.y - m_bot * bottom_left.x;

        // get the average point of the 2 horizontal pairs
        // this is the horizontal center of the screen relative to the IR camera
        PointF top_avg = {float(top_left.x + top_right.x) / 2, float(top_left.y + top_right.y) / 2};
        PointF bot_avg = {float(bottom_left.x + bottom_right.x) / 2, float(bottom_left.y + bottom_right.y) / 2};

        // the distance from the average point to any of the 2 points on the same horizontal line is half the width of the WII IR Sensor Bar
        // we can use this to calculate the location of the screen end points
        float top_step_width_cm = SafeDivide(static_cast<float>(wii_ir_led_width_cm / 2), (top_right.x - top_avg.x));
        float bot_step_width_cm = SafeDivide(static_cast<float>(wii_ir_led_width_cm / 2), (bottom_right.x - bot_avg.x));

        constexpr float screen_half_width_cm = screen_width_cm / 2;
        float num_steps_top = SafeDivide(screen_half_width_cm, top_step_width_cm);
        float num_steps_bot = SafeDivide(screen_half_width_cm, bot_step_width_cm);

        // now that we have the step width, we can calculate the screen end points
        PointF screen_top_left = {top_avg.x - num_steps_top, m_top * (top_avg.x - num_steps_top) + n_top};
        PointF screen_top_right = {top_avg.x + num_steps_top, m_top * (top_avg.x + num_steps_top) + n_top};
        PointF screen_bot_left = {bot_avg.x - num_steps_bot, m_bot * (bot_avg.x - num_steps_bot) + n_bot};

        // calculate the left vertical line equation
        float m_left = SafeDivide((top_left.y - bottom_left.y), (top_left.x - bottom_left.x));
        float n_left = top_left.y - m_left * top_left.x;

        // we can create 2 axis of the screen relative to the IR camera
        // we need to calculate where the IR Camera mid point {(511.5, 511.5)} is relative to the 2 axis
        constexpr PointF ir_camera_mid = {float(dfrobot_max_unit) / 2, float(dfrobot_max_unit) / 2};

        // get the intersection point of the camera mid point with the screen axis
        float n_left_mid = ir_camera_mid.y - (m_left * ir_camera_mid.x);
        float n_top_mid = ir_camera_mid.y - (m_top * ir_camera_mid.x);
        const float &m_left_mid = m_left;
        const float &m_top_mid = m_top;

        float x_inter_top_w_left_mid = SafeDivide((n_top - n_left_mid), (m_left_mid - m_top));
        float x_inter_left_w_top_mid = SafeDivide((n_left - n_top_mid), (m_top_mid - m_left));

        // calculate the percentage of the intersection points relative to the screen pixels
        float x_percentage = SafeDivide((x_inter_top_w_left_mid - screen_top_left.x), (screen_top_right.x - screen_top_left.x));
        float y_percentage = SafeDivide((x_inter_left_w_top_mid - screen_top_left.y), (screen_bot_left.y - screen_top_left.y));

        // calculate the screen point
        result.x = x_percentage * screen_consts.effective_width;
        result.y = y_percentage * screen_consts.effective_height;

        // invert the y axis
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
