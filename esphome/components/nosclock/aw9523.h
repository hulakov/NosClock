#pragma once
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/color.h"

namespace nosclock {

class Aw9523 {
 public:
  void setup(esphome::i2c::I2CBus *bus);
  void set_dot_color(int dot_index, esphome::Color color);

 private:
  void write_reg(uint8_t reg, uint8_t value);

  esphome::i2c::I2CBus *m_bus = nullptr;
  esphome::Color m_last_colors[4];
};

} // namespace nosclock
