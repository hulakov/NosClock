#pragma once
#include <array>
#include <cmath>
#include <algorithm>
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/time/real_time_clock.h"

namespace nosclock {

struct ColorFloat {
  float r{0.0f};
  float g{0.0f};
  float b{0.0f};

  ColorFloat() = default;
  ColorFloat(float r, float g, float b) : r(r), g(g), b(b) {}
  ColorFloat(esphome::Color c) : r(c.r / 255.0f), g(c.g / 255.0f), b(c.b / 255.0f) {}

  esphome::Color to_color(float factor = 1.0f) const {
    return esphome::Color(
      (uint8_t)std::round(std::min(std::max(r * factor * 255.0f, 0.0f), 255.0f)),
      (uint8_t)std::round(std::min(std::max(g * factor * 255.0f, 0.0f), 255.0f)),
      (uint8_t)std::round(std::min(std::max(b * factor * 255.0f, 0.0f), 255.0f))
    );
  }
};

static constexpr size_t NUM_DIGITS = 6;
static constexpr size_t NUM_LEDS = 12;
static constexpr size_t NUM_DOTS = 4;
static constexpr uint32_t BLINK_INTERVAL_MS = 500;
static constexpr float DOTS_BRIGHTNESS_LIMIT = 0.08f;


class INosEffect {
 public:
  virtual ~INosEffect() = default;

  // Apply effect to the 12 backlight LEDs (using normalized float colors 0.0f..1.0f)
  virtual std::array<ColorFloat, NUM_LEDS> apply_backlight(esphome::ESPTime time_now, ColorFloat user_color) = 0;

  // Apply effect to the 4 colon dots (using normalized float colors 0.0f..1.0f)
  virtual std::array<ColorFloat, NUM_DOTS> apply_dots(esphome::ESPTime time_now, ColorFloat user_color) = 0;
};

} // namespace nosclock
