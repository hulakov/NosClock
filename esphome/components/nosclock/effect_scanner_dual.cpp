#include "effect_scanner_dual.h"
#include <cmath>
#include "esphome/core/hal.h"

namespace nosclock {

struct GroupInfo {
  bool is_dot;
  size_t hardware_index_1;
  size_t hardware_index_2;
};

static const GroupInfo GROUPS[8] = {
  {false, 0, 1},    // Digit 0 (backlight index 0, 1)
  {false, 2, 3},    // Digit 1 (backlight index 2, 3)
  {true,  0, 1},    // Dot group 0 (dots index 0, 1)
  {false, 4, 5},    // Digit 2 (backlight index 4, 5)
  {false, 6, 7},    // Digit 3 (backlight index 6, 7)
  {true,  2, 3},    // Dot group 1 (dots index 2, 3)
  {false, 8, 9},    // Digit 4 (backlight index 8, 9)
  {false, 10, 11}   // Digit 5 (backlight index 10, 11)
};

void ScannerDualEffect::update_position() {
  uint32_t now = esphome::millis();
  if (m_last_frame_ms == 0) {
    m_last_frame_ms = now;
    return;
  }
  uint32_t dt = now - m_last_frame_ms;
  if (dt == 0) return;
  m_last_frame_ms = now;

  float speed = 1.0f / 400.0f; // 250ms per group
  m_pos += speed * dt * m_direction;

  if (m_pos <= 0.0f) {
    m_pos = 0.0f;
    m_direction = 1.0f;
  } else if (m_pos >= 7.0f) {
    m_pos = 7.0f;
    m_direction = -1.0f;
  }
}

std::array<esphome::Color, NUM_LEDS> ScannerDualEffect::apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) {
  update_position();

  std::array<esphome::Color, NUM_LEDS> res{};
  float width = 1.2f;

  for (int i = 0; i < 8; i++) {
    const auto &group = GROUPS[i];
    if (group.is_dot) continue;

    float distance = std::fabs(m_pos - (float)i);
    if (distance < width) {
      float intensity = 1.0f - (distance / width);
      size_t idx1 = group.hardware_index_1;
      size_t idx2 = group.hardware_index_2;
      if (idx1 < NUM_LEDS) {
        res[idx1] = esphome::Color(
          target_color.r * intensity,
          target_color.g * intensity,
          target_color.b * intensity
        );
      }
      if (idx2 < NUM_LEDS) {
        res[idx2] = esphome::Color(
          target_color.r * intensity,
          target_color.g * intensity,
          target_color.b * intensity
        );
      }
    }
  }
  return res;
}

std::array<esphome::Color, NUM_DOTS> ScannerDualEffect::apply_dots(esphome::ESPTime time_now, esphome::Color target_color) {
  update_position();

  std::array<esphome::Color, NUM_DOTS> res{};
  float width = 1.2f;

  for (int i = 0; i < 8; i++) {
    const auto &group = GROUPS[i];
    if (!group.is_dot) continue;

    float distance = std::fabs(m_pos - (float)i);
    if (distance < width) {
      float intensity = 1.0f - (distance / width);
      size_t idx1 = group.hardware_index_1;
      size_t idx2 = group.hardware_index_2;
      if (idx1 < NUM_DOTS) {
        res[idx1] = esphome::Color(
          target_color.r * intensity,
          target_color.g * intensity,
          target_color.b * intensity
        );
      }
      if (idx2 < NUM_DOTS) {
        res[idx2] = esphome::Color(
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
