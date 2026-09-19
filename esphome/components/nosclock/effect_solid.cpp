#include "effect_solid.h"

namespace nosclock {

std::array<esphome::Color, NUM_LEDS> SolidEffect::apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) {
  std::array<esphome::Color, NUM_LEDS> res{};
  if (m_backlight_blinks && time_now.second % 2 != 0) {
    return res;
  }

  for (size_t i = 0; i < res.size(); i++) {
    res[i] = target_color;
  }
  return res;
}

std::array<esphome::Color, NUM_DOTS> SolidEffect::apply_dots(esphome::ESPTime time_now, esphome::Color target_color) {
  std::array<esphome::Color, NUM_DOTS> res{};
  if (m_dots_blinks && time_now.second % 2 != 0) {
    return res;
  }

  for (size_t i = 0; i < res.size(); i++) {
    res[i] = target_color;
  }
  return res;
}

} // namespace nosclock
