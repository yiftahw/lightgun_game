#include <thread>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <iostream>
#include <tuple>
#include <optional>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <CLI/CLI.hpp>

#include "DataAcqHTTP.h"
#include "screen.h"
#include "Snapshot.h"
#include "consts.h"
#include "PointMapping.h"
#include "DataAcqPlayback.h"
#include "utils.h"

enum class RenderMode
{
    CURSOR,
    DEBUG,
};
std::optional<RenderMode> render_mode_from_string(const std::string &str)
{
    static const std::map<std::string, RenderMode> to_string_map {
        {"cursor", RenderMode::CURSOR},
        {"debug", RenderMode::DEBUG}
    };
    return optional_map_get(to_string_map, str);
}

enum class AcquisitionMode
{
    HTTP_RECORD,
    HTTP_LIVE,
    PLAYBACK
};
std::optional<AcquisitionMode> data_acq_mode_from_string(const std::string &str)
{
    static const std::map<std::string, AcquisitionMode> to_string_map {
        {"record", AcquisitionMode::HTTP_RECORD},
        {"live", AcquisitionMode::HTTP_LIVE},
        {"playback", AcquisitionMode::PLAYBACK}
    };
    return optional_map_get(to_string_map, str);
}

std::pair<SDL_FPoint, SDL_FPoint> sdl_segment(const LineSegment &segment)
{
    return {{segment.p1.x, segment.p1.y}, {segment.p2.x, segment.p2.y}};
}

std::pair<SDL_FPoint, SDL_FPoint> sdl_segment(const PointF &p1, const PointF &p2)
{
    return {{p1.x, p1.y}, {p2.x, p2.y}};
}

SDL_FPoint sdl_point(const PointF &point)
{
    return {point.x, point.y};
}

SDL_FPoint sdl_point(const Point &point)
{
    return {static_cast<float>(point.x), static_cast<float>(point.y)};
}

// record raw data from the HTTP server
void record(IDataAcq *data_acq, std::string file_name, uint32_t samples, uint8_t fps)
{
    std::ofstream output(file_name, std::ios::out);
    if (!output.is_open())
    {
        printf("Failed to open file %s\n", file_name.c_str());
        return;
    }

    while (samples-- > 0)
    {
        auto snapshot = data_acq->get();
        output << snapshot.to_string() << std::endl;
        std::cout << snapshot.to_string() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
    }

    output.close();
}

void play(IDataAcq *data_acq, Screen *screen, screen_constants constants, RenderMode mode)
{
    while (true)
    {
        auto snapshot = data_acq->get();

        if (mode == RenderMode::DEBUG)
        {
            printf("Snapshot: %s\n", snapshot.to_string().c_str());
            
            // std::vector<PointF> mapped_points(dfrobot_snapshot_size);
            // for (auto &point : snapshot.points)
            // {
            //     auto mapped = map_snapshot_debug(point, constants);
            //     mapped_points.push_back(mapped);
            // }

            auto opt_borders = map_snapshot_to_borders(snapshot);
            if (!opt_borders.has_value())
            {
                continue;
            }
            auto borders = opt_borders.value();

            auto corners = calculate_screen_corners(snapshot);
            if (!corners.has_value())
            {
                continue;
            }

            screen->clear_pixels();
            // for (auto &point : mapped_points)
            for (auto &point : snapshot.points)
            {
                screen->add_pixel(sdl_point(point));
            }
            auto& top_left = corners.value().top_left;
            auto& top_right = corners.value().top_right;
            auto& bot_left = corners.value().bot_left;
            auto& bot_right = corners.value().bot_right;

            screen->add_pixel(sdl_point(top_left));
            screen->add_pixel(sdl_point(top_right));
            screen->add_pixel(sdl_point(bot_left));
            screen->add_pixel(sdl_point(bot_right));

            screen->clear_segments();
            // screen->add_segment(sdl_segment(top_left, top_right));
            // screen->add_segment(sdl_segment(bot_left, bot_right));

            screen->add_segment(sdl_segment(borders.screen_top_segment));
            screen->add_segment(sdl_segment(borders.screen_bot_segment));
            screen->add_segment(sdl_segment(borders.screen_left_segment));
            screen->add_segment(sdl_segment(borders.screen_right_segment));
            screen->add_segment(sdl_segment(borders.cursor_horizontal_segment));
            screen->add_segment(sdl_segment(borders.cursor_vertical_segment));
        }
        else // playback_mode::CURSOR
        {
            auto pt = map_snapshot_to_cursor(snapshot, constants);
            if (pt.x == 0 && pt.y == 0)
            {
                continue;
            }

            screen->clear_pixels();
            screen->add_pixel({pt.x, pt.y});
        }

        screen->render_screen();
        screen->input();
    }
}

std::tuple<Screen*, screen_constants> init_screen()
{
    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return {nullptr, screen_constants(0, 0, 1.0f)};
    }

    SDL_DisplayMode dm;
    if (0 != SDL_GetDesktopDisplayMode(0, &dm))
    {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        return {nullptr, screen_constants(0, 0, 1.0f)};
    }
    printf("Display mode: %dx%d\n", dm.w, dm.h);

    auto width = 0.95 * dm.w;
    auto height = dm.h - (dm.w - width);
    printf("Width: %.2f, Height: %.2f\n", width, height);

    auto screen = Screen::create("Hello, World!", width, height);
    if (screen == nullptr)
    {
        printf("Failed to create screen!\n");
        return {nullptr, screen_constants(0, 0, 1.0f)};
    }

    screen_constants constants(width, height, 1.0f);
    return {screen, constants};
}

void profile_run_time(uint32_t cycles)
{
    auto start = std::chrono::high_resolution_clock::now();

    DataAcqPlayback playback_acq("raw_data.txt", 30);
    if (!playback_acq.is_open())
    {
        printf("Failed to open file\n");
        return;
    }
    screen_constants constants(2560, 1440, 1.0f);

    for (uint32_t i = 0; i < cycles; i++)
    {
        if (i % 100 == 0)
        {
            printf("Cycle: %d\n", i);
        }
        auto snapshot = playback_acq.get(true);
        map_snapshot_to_cursor(snapshot, constants); // we don't care about the return value
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // calculate the average time taken for each cycle
    printf("Average time taken: %f microseconds\n", float(duration.count()) / cycles);
}

int main(int argc, char** argv)
{
    CLI::App app{"Lightgun Game"};

    std::string mode;
    app.add_option("-m,--mode", mode, "Data Acquisition Mode")
        ->check(CLI::IsMember({"live", "record", "playback"}))
        ->default_val("live");

    std::string render_mode;
    app.add_option("-p,--playback", render_mode, "Render mode")
        ->check(CLI::IsMember({"cursor", "debug"}))
        ->default_val("cursor");

    CLI11_PARSE(app, argc, argv);


    auto opt_acq_mode = data_acq_mode_from_string(mode);
    auto opt_render_mode = render_mode_from_string(render_mode);

    if (!opt_acq_mode.has_value() || !opt_render_mode.has_value())
    {
        // CLI11 handles validation
        // this should not happen
        std::cerr << "Invalid arguments" << std::endl;
        return EXIT_FAILURE;
    }

    IDataAcq *data_acq = nullptr;
    if (opt_acq_mode.value() == AcquisitionMode::PLAYBACK)
    {
        auto playback_acq = new DataAcqPlayback("raw_data.txt", 10);
        if (!playback_acq->is_open())
        {
            return -1;
        }
        data_acq = playback_acq;
    }
    else
    {
        data_acq = new DataAcqHTTP("10.100.102.34:80");
    }

    if (opt_acq_mode.value() == AcquisitionMode::HTTP_RECORD)
    {
        record(data_acq, "record.txt", 1200, 60);
    }
    else
    {
        auto [screen, constants] = init_screen();
        if (screen == nullptr)
        {
            return -1;
        }

        play(data_acq, screen, constants, opt_render_mode.value());
    }

    return 0;
}
