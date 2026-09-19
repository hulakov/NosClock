#pragma once
#include <array>
#include <cstdint>
#include "nos_effect.h"

namespace nosclock {

class ScannerEffect : public INosEffect {
 public:
  std::array<esphome::Color, NUM_LEDS> apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) override;
  std::array<esphome::Color, NUM_DOTS> apply_dots(esphome::ESPTime time_now, esphome::Color target_color) override;

 private:
  void update_position();

  float m_pos = 0.0f;
  float m_direction = 1.0f;
  uint32_t m_last_frame_ms = 0;
};

} // namespace nosclock
