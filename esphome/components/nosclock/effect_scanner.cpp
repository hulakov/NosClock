#include "effect_scanner.h"
#include <cmath>
#include "esphome/core/hal.h"

namespace nosclock {

struct ElementInfo {
  bool is_dot;
  size_t hardware_index;
};

static const ElementInfo ELEMENTS[16] = {
  {false, 0},   // Digit 0 Top (backlight index 0)
  {false, 1},   // Digit 0 Bottom (backlight index 1)
  {false, 2},   // Digit 1 Top (backlight index 2)
  {false, 3},   // Digit 1 Bottom (backlight index 3)
  {true,  0},   // Dot 0 Top (dots index 0)
  {true,  1},   // Dot 0 Bottom (dots index 1)
  {false, 4},   // Digit 2 Top (backlight index 4)
  {false, 5},   // Digit 2 Bottom (backlight index 5)
  {false, 6},   // Digit 3 Top (backlight index 6)
  {false, 7},   // Digit 3 Bottom (backlight index 7)
  {true,  2},   // Dot 1 Top (dots index 2)
  {true,  3},   // Dot 1 Bottom (dots index 3)
  {false, 8},   // Digit 4 Top (backlight index 8)
  {false, 9},   // Digit 4 Bottom (backlight index 9)
  {false, 10},  // Digit 5 Top (backlight index 10)
  {false, 11}   // Digit 5 Bottom (backlight index 11)
};

void ScannerEffect::update_position() {
  uint32_t now = esphome::millis();
  if (m_last_frame_ms == 0) {
    m_last_frame_ms = now;
    return;
  }
  uint32_t dt = now - m_last_frame_ms;
  if (dt == 0) return;
  m_last_frame_ms = now;

  float speed = 1.0f / 250.0f; // 250ms per element
  m_pos += speed * dt * m_direction;

  if (m_pos <= 0.0f) {
    m_pos = 0.0f;
    m_direction = 1.0f;
  } else if (m_pos >= 15.0f) {
    m_pos = 15.0f;
    m_direction = -1.0f;
  }
}

std::array<esphome::Color, NUM_LEDS> ScannerEffect::apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) {
  update_position();

  std::array<esphome::Color, NUM_LEDS> res{};
  float width = 1.5f;

  for (int i = 0; i < 16; i++) {
    const auto &elem = ELEMENTS[i];
    if (elem.is_dot) continue;

    float distance = std::fabs(m_pos - (float)i);
    if (distance < width) {
      float intensity = 1.0f - (distance / width);
      size_t idx = elem.hardware_index;
      if (idx < NUM_LEDS) {
        res[idx] = esphome::Color(
          target_color.r * intensity,
          target_color.g * intensity,
          target_color.b * intensity
        );
      }
    }
  }
  return res;
}

std::array<esphome::Color, NUM_DOTS> ScannerEffect::apply_dots(esphome::ESPTime time_now, esphome::Color target_color) {
  update_position();

  std::array<esphome::Color, NUM_DOTS> res{};
  float width = 1.5f;

  for (int i = 0; i < 16; i++) {
    const auto &elem = ELEMENTS[i];
    if (!elem.is_dot) continue;

    float distance = std::fabs(m_pos - (float)i);
    if (distance < width) {
      float intensity = 1.0f - (distance / width);
      size_t idx = elem.hardware_index;
      if (idx < NUM_DOTS) {
        res[idx] = esphome::Color(
          target_color.r * intensity,
          target_color.g * intensity,
          target_color.b * intensity
        );
      }
    }
  }
  return res;
}

} // namespace nosclock
