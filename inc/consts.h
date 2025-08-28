#pragma once

#include <cstdint>

static inline constexpr uint32_t dfrobot_snapshot_size = 4;
static inline constexpr uint32_t dfrobot_max_unit_x = 1023; // 10 bit resolution
static inline constexpr uint32_t dfrobot_resolution_x = 128;
static inline constexpr uint32_t dfrobot_resolution_y = 96;
static inline constexpr uint32_t dfrobot_max_unit_y = (dfrobot_max_unit_x * dfrobot_resolution_y) / dfrobot_resolution_x;

static inline constexpr float wii_ir_led_width_cm = 20;

static inline constexpr float screen_scale = 1.0f; // pixels will be drawn at this scale

// screen size constants
static inline constexpr float screen_physical_size_inch = 27;
static inline constexpr double screen_ratio_width = 16;
static inline constexpr double screen_ratio_height = 9;
static inline constexpr float screen_width_cm = 59.8;
static inline constexpr float screen_height_cm = 33.6;
