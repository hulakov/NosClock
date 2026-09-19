#pragma once
#include <array>
#include "nos_effect.h"

namespace nosclock {

class SolidEffect : public INosEffect {
 public:
  SolidEffect(bool dots_blinks, bool backlight_blinks = false)
      : m_dots_blinks(dots_blinks), m_backlight_blinks(backlight_blinks) {}
  std::array<esphome::Color, NUM_LEDS> apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) override;
  std::array<esphome::Color, NUM_DOTS> apply_dots(esphome::ESPTime time_now, esphome::Color target_color) override;

 private:
  bool m_dots_blinks;
  bool m_backlight_blinks;
};

} // namespace nosclock
