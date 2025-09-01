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

void play(IDataAcq *data_acq, Screen *screen, screen_constants constants, const bool debug_mode)
{
    const ScreenCorners screen_corners{
        PointF{0, 0},
        PointF{constants.effective_width, 0},
        PointF{0, constants.effective_height},
        PointF{constants.effective_width, constants.effective_height}
    };

    while (true)
    {
        auto snapshot = data_acq->get();

        if (debug_mode)
        {
            printf("Snapshot: %s\n", snapshot.to_string().c_str());

            auto opt_borders = map_snapshot_to_borders(snapshot);
            if (!opt_borders.has_value())
            {
                continue;
            }
            auto borders = opt_borders.value();
            auto corners = borders.corners;

            screen->clear_pixels();
            for (auto &point : snapshot.points)
            {
                screen->add_pixel(sdl_point(point));
            }
            auto& top_left = corners.top_left;
            auto& top_right = corners.top_right;
            auto& bot_left = corners.bot_left;
            auto& bot_right = corners.bot_right;

            screen->add_pixel(sdl_point(top_left));
            screen->add_pixel(sdl_point(top_right));
            screen->add_pixel(sdl_point(bot_left));
            screen->add_pixel(sdl_point(bot_right));

            screen->clear_segments();

            screen->add_segment(sdl_segment(borders.screen_top_segment));
            screen->add_segment(sdl_segment(borders.screen_bot_segment));
            screen->add_segment(sdl_segment(borders.screen_left_segment));
            screen->add_segment(sdl_segment(borders.screen_right_segment));
            screen->add_segment(sdl_segment(borders.cursor_horizontal_segment));
            screen->add_segment(sdl_segment(borders.cursor_vertical_segment));
        }
        else // playback_mode::CURSOR
        {
            auto pt = map_snapshot_to_cursor(snapshot, screen_corners);
            if (!pt)
            {
                continue;
            }
            const auto &[x, y] = pt.value();

            screen->clear_pixels();
            screen->add_pixel({x, y});
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
    ScreenCorners corners(2560, 1440);

    for (uint32_t i = 0; i < cycles; i++)
    {
        if (i % 100 == 0)
        {
            printf("Cycle: %d\n", i);
        }
        auto snapshot = playback_acq.get(true);
        map_snapshot_to_cursor(snapshot, corners); // we don't care about the return value
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // calculate the average time taken for each cycle
    printf("Average time taken: %f microseconds\n", float(duration.count()) / cycles);
}

int main(int argc, char** argv)
{
    // parse CLI arguments
    CLI::App app{"Lightgun Game"};

    std::string record_directory;
    app.add_option("-r,--record", record_directory, "Directory path to record data to (will not record if not specified)")
        ->check(CLI::ExistingDirectory);
    
    std::string playback_file_path;
    app.add_option("-p,--playback", playback_file_path, "File path for playback")
        ->check(CLI::ExistingFile);

    bool debug_mode = false;
    app.add_flag("-d,--debug", debug_mode, "Debug rendering mode");

    CLI11_PARSE(app, argc, argv);

    // construct instances
    IDataAcq *data_acq = nullptr;
    if (playback_file_path.length() > 0)
    {
        // CLI11 asserts that the file exists
        uint8_t fps = 15;
        auto playback_acq = new DataAcqPlayback(playback_file_path, fps);
        if (!playback_acq->is_open())
        {
            return EXIT_FAILURE;
        }
        data_acq = playback_acq;
    }
    else
    {
        data_acq = new DataAcqHTTP("10.100.102.34:80");
    }

    if (record_directory.length() > 0)
    {
        // CLI11 asserts the directory exists

        // TODO: recording should be unified with the rendering logic,
        // so they can be used at the same time.
        record(data_acq, "record.txt", 1200, 60);
    }
    else
    {
        auto [screen, constants] = init_screen();
        if (screen == nullptr)
        {
            return EXIT_FAILURE;
        }

        play(data_acq, screen, constants, debug_mode);
    }

    return 0;
}
