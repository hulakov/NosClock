#pragma once
#include <array>
#include "esphome/core/component.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/output/float_output.h"

namespace nosclock {

class NosTubes {
 public:
  void setup();
  void update(esphome::ESPTime time_now, bool enabled, float brightness, esphome::output::FloatOutput *tubes_en);

 private:
  uint32_t get_single_digit_bit(int num, int pos);
  uint32_t get_3_digits(int d1, int d2, int d3);
  void shift_out_32(uint32_t data);

  std::array<int, 6> m_last_digits{-1, -1, -1, -1, -1, -1};
  bool m_last_enabled = false;
  float m_last_brightness = -1.0f;
  uint32_t m_second_59_start_ms = 0;
  bool m_was_rolling = false;
};

} // namespace nosclock
