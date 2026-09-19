#include "aw9523.h"

namespace nosclock {

void Aw9523::setup(esphome::i2c::I2CBus *bus) {
  m_bus = bus;
  // Init AW9523
  write_reg(0x12, 0x00); // REG_LED_P0
  write_reg(0x13, 0x00); // REG_LED_P1
  write_reg(0x11, 0x03); // REG_GCR

  // Initialize cache to a value that forces the first write
  for (int i = 0; i < 4; i++) {
    m_last_colors[i] = esphome::Color(99, 99, 99);
  }
}

void Aw9523::write_reg(uint8_t reg, uint8_t value) {
  if (m_bus != nullptr) {
    uint8_t data[2] = {reg, value};
    m_bus->write(0x58, data, 2);
  }
}

void Aw9523::set_dot_color(int dot_index, esphome::Color color) {
  static const uint8_t REG_R[] = {0x26, 0x29, 0x2C, 0x2D};
  static const uint8_t REG_G[] = {0x27, 0x2A, 0x20, 0x2E};
  static const uint8_t REG_B[] = {0x28, 0x2B, 0x21, 0x2F};

  if (dot_index >= 0 && dot_index < 4) {
    if (color.r != m_last_colors[dot_index].r ||
        color.g != m_last_colors[dot_index].g ||
        color.b != m_last_colors[dot_index].b) {
      write_reg(REG_R[dot_index], color.r);
      write_reg(REG_G[dot_index], color.g);
      write_reg(REG_B[dot_index], color.b);
      m_last_colors[dot_index] = color;
    }
  }
}

} // namespace nosclock
