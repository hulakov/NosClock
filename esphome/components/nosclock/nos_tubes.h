#pragma once
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

  int m_last_hour = -1;
  int m_last_minute = -1;
  int m_last_second = -1;
  bool m_last_enabled = false;
  float m_last_brightness = -1.0f;
};

} // namespace nosclock
