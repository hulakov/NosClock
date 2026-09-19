#include "effect_digit_sync.h"
#include <algorithm>

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

std::array<esphome::Color, NUM_LEDS> DigitSyncEffect::apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) {
  float brightness = std::max({target_color.r, target_color.g, target_color.b, (uint8_t)1}) / 255.0f;
  std::array<esphome::Color, NUM_LEDS> res;

  for (size_t i = 0; i < 6; i++) {
    uint8_t digit = get_clock_digit(time_now, i);
    res[i * 2] = esphome::Color(
      PALETTE[digit].r * brightness,
      PALETTE[digit].g * brightness,
      PALETTE[digit].b * brightness
    );
    res[i * 2 + 1] = esphome::Color(
      PALETTE[(digit + 1) % 10].r * brightness,
      PALETTE[(digit + 1) % 10].g * brightness,
      PALETTE[(digit + 1) % 10].b * brightness
    );
  }
  return res;
}

std::array<esphome::Color, NUM_DOTS> DigitSyncEffect::apply_dots(esphome::ESPTime time_now, esphome::Color target_color) {
  std::array<esphome::Color, NUM_DOTS> res{};
  if (time_now.second % 2 != 0) return res;

  float brightness = std::max({target_color.r, target_color.g, target_color.b, (uint8_t)1}) / 255.0f;

  // Dot 0 & 1 are between Digit 1 and Digit 2
  uint8_t d1 = get_clock_digit(time_now, 1);
  uint8_t d2 = get_clock_digit(time_now, 2);
  esphome::Color c1_top = PALETTE[d1];
  esphome::Color c1_bot = PALETTE[(d1 + 1) % 10];
  esphome::Color c2_top = PALETTE[d2];
  esphome::Color c2_bot = PALETTE[(d2 + 1) % 10];

  esphome::Color color_dots_12(
    (uint8_t)(((int)c1_top.r + (int)c1_bot.r + (int)c2_top.r + (int)c2_bot.r) / 4 * brightness),
    (uint8_t)(((int)c1_top.g + (int)c1_bot.g + (int)c2_top.g + (int)c2_bot.g) / 4 * brightness),
    (uint8_t)(((int)c1_top.b + (int)c1_bot.b + (int)c2_top.b + (int)c2_bot.b) / 4 * brightness)
  );

  res[0] = color_dots_12;
  res[1] = color_dots_12;

  // Dot 2 & 3 are between Digit 3 and Digit 4
  uint8_t d3 = get_clock_digit(time_now, 3);
  uint8_t d4 = get_clock_digit(time_now, 4);
  esphome::Color c3_top = PALETTE[d3];
  esphome::Color c3_bot = PALETTE[(d3 + 1) % 10];
  esphome::Color c4_top = PALETTE[d4];
  esphome::Color c4_bot = PALETTE[(d4 + 1) % 10];

  esphome::Color color_dots_34(
    (uint8_t)(((int)c3_top.r + (int)c3_bot.r + (int)c4_top.r + (int)c4_bot.r) / 4 * brightness),
    (uint8_t)(((int)c3_top.g + (int)c3_bot.g + (int)c4_top.g + (int)c4_bot.g) / 4 * brightness),
    (uint8_t)(((int)c3_top.b + (int)c3_bot.b + (int)c4_top.b + (int)c4_bot.b) / 4 * brightness)
  );

  res[2] = color_dots_34;
  res[3] = color_dots_34;

  return res;
}

} // namespace nosclock
