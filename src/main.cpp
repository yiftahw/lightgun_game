#include <thread>
#include <stdio.h>
#include <iostream>
#include <SDL2/SDL.h>
#include "http_client.h"
#include "screen.h"
#include "Snapshot.h"

static inline constexpr int screen_width = 800;
static inline constexpr int screen_height = 800;
static inline constexpr float screen_scale = 2.0f; // pixels will be drawn at this scale
static inline constexpr float effective_scale = 0.9f; // pixels will only be drawn within this range

static inline constexpr int32_t dfrobot_max_unit = 1023;

static_assert(effective_scale <= 1.0f, "Effective scale must be less than or equal to 1.0f");
static_assert(screen_scale >= 1.0f, "Screen scale must be greater than or equal to 1.0f");


point2d map_dfrobot(uint16_t x, uint16_t y)
{
    constexpr float scaled_width = (screen_width / screen_scale);
    constexpr float scaled_height = (screen_height / screen_scale);

    constexpr float width_border = (1.0f - effective_scale) * scaled_width;
    constexpr float height_border = (1.0f - effective_scale) * scaled_height;

    constexpr float effective_width = scaled_width - (2.0f * width_border);
    constexpr float effective_height = scaled_height - (2.0f * height_border);

    constexpr float x_min = width_border;
    constexpr float y_min = height_border;

    float x_mapped = x_min + ((static_cast<float>(x) / static_cast<float>(dfrobot_max_unit)) * effective_width);
    float y_mapped = y_min + ((static_cast<float>(y) / static_cast<float>(dfrobot_max_unit)) * effective_height);

    // to make the y axis go from top to bottom, we need to invert it
    y_mapped = effective_height - y_mapped;

    return {x_mapped, y_mapped};
}

void test()
{
    auto screen = Screen::create("Hello, World!", screen_width, screen_height, screen_scale);
    if (screen == nullptr)
    {
        printf("Failed to create screen!\n");
        return;
    }

    HTTPClient client("10.100.102.34:80");

    while (true)
    {
        screen->clear_pixels();

        std::string data = client.get_data();
        auto snapshot = parsePoints(data);
        for (const auto &point : snapshot.points)
        {
            screen->add_pixel(map_dfrobot(point.x, point.y));
        }

        screen->render_screen();
        screen->input();
    }
}

int main(int argc, char** argv)
{
    // SDL library uses macros to bypass our main function and it calls our main function
    // this causes our main function to must have argc and argv parameters
    // to bypass the "unused parameter" warning from "Werror" flag, we can use this line
    if (argc == 0 && argv == nullptr) {}

    test();

    return 0;
}
