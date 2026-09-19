#pragma once
#include <array>
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/time/real_time_clock.h"

namespace nosclock {

static constexpr size_t NUM_DIGITS = 6;
static constexpr size_t NUM_LEDS = 12;
static constexpr size_t NUM_DOTS = 4;
static constexpr uint32_t BLINK_INTERVAL_MS = 500;
static constexpr float DOTS_BRIGHTNESS_LIMIT = 0.5f;

class INosEffect {
 public:
  virtual ~INosEffect() = default;

  // Apply effect to the 12 backlight LEDs
  virtual std::array<esphome::Color, NUM_LEDS> apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) = 0;

  // Apply effect to the 4 colon dots
  virtual std::array<esphome::Color, NUM_DOTS> apply_dots(esphome::ESPTime time_now, esphome::Color target_color) = 0;
};

} // namespace nosclock
