#pragma once
#include <array>
#include "nos_effect.h"

namespace nosclock {

class DigitSyncEffect : public INosEffect {
 public:
  std::array<esphome::Color, NUM_LEDS> apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) override;
  std::array<esphome::Color, NUM_DOTS> apply_dots(esphome::ESPTime time_now, esphome::Color target_color) override;
};

} // namespace nosclock
