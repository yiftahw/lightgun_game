#pragma once

#include <cstdint>

static inline constexpr uint32_t dfrobot_max_unit = 1023;
static inline constexpr uint32_t wii_ir_led_width_cm = 20;
static inline constexpr uint32_t screen_physical_size_inch = 27;
static inline constexpr uint32_t inch_in_cm = 2.54;

static inline constexpr float screen_scale = 1.0f; // pixels will be drawn at this scale

static inline constexpr double screen_ratio_width = 16;
static inline constexpr double screen_ratio_height = 9;
