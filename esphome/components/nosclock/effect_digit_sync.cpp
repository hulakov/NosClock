#include "effect_digit_sync.h"
#include <algorithm>
#include <cmath>

namespace nosclock {

static const esphome::Color PALETTE[10] = {
  esphome::Color(255, 255, 255), // White
  esphome::Color(0, 255, 0),     // Green
  esphome::Color(0, 0, 255),     // Blue
  esphome::Color(150, 75, 0),    // Brown
  esphome::Color(255, 127, 0),   // Orange
  esphome::Color(255, 255, 0),   // Yellow
  esphome::Color(0, 255, 255),   // Cyan
  esphome::Color(128, 0, 128),   // Purple
  esphome::Color(255, 0, 255),   // Magenta
  esphome::Color(255, 20, 147)   // DeepPink
};

static uint8_t get_clock_digit(esphome::ESPTime time_now, int index) {
  switch (index) {
    case 0: return time_now.hour / 10;
    case 1: return time_now.hour % 10;
    case 2: return time_now.minute / 10;
    case 3: return time_now.minute % 10;
    case 4: return time_now.second / 10;
    case 5: return time_now.second % 10;
    default: return 0;
  }
}

std::array<ColorFloat, NUM_LEDS> DigitSyncEffect::apply_backlight(esphome::ESPTime time_now, ColorFloat target_color) {
  std::array<ColorFloat, NUM_LEDS> res;

  for (size_t i = 0; i < 6; i++) {
    uint8_t digit = get_clock_digit(time_now, i);
    res[i * 2] = ColorFloat(PALETTE[digit]);
    res[i * 2 + 1] = ColorFloat(PALETTE[(digit + 1) % 10]);
  }
  return res;
}

std::array<ColorFloat, NUM_DOTS> DigitSyncEffect::apply_dots(esphome::ESPTime time_now, ColorFloat target_color) {
  std::array<ColorFloat, NUM_DOTS> res{};
  if (time_now.second % 2 != 0) return res;

  float r_sum = 0.0f, g_sum = 0.0f, b_sum = 0.0f;
  for (int i = 0; i < 6; i++) {
    uint8_t digit = get_clock_digit(time_now, i);
    ColorFloat c_top(PALETTE[digit]);
    ColorFloat c_bot(PALETTE[(digit + 1) % 10]);

    r_sum += c_top.r + c_bot.r;
    g_sum += c_top.g + c_bot.g;
    b_sum += c_top.b + c_bot.b;
  }

  ColorFloat unified_color(
    r_sum / 12.0f,
    g_sum / 12.0f,
    b_sum / 12.0f
  );

  for (size_t i = 0; i < NUM_DOTS; i++) {
    res[i] = unified_color;
  }

  return res;
}

} // namespace nosclock
