#include "effect_scanner_split.h"
#include <cmath>
#include "esphome/core/hal.h"

namespace nosclock {

struct ElementInfo {
  bool is_dot;
  size_t hardware_index;
};

static const ElementInfo ELEMENTS[16] = {
  {false, 0},    // 0: Digit 0 Top
  {false, 2},    // 1: Digit 1 Top
  {true,  0},    // 2: Dot 0 Top
  {false, 4},    // 3: Digit 2 Top
  {false, 6},    // 4: Digit 3 Top
  {true,  2},    // 5: Dot 1 Top
  {false, 8},    // 6: Digit 4 Top
  {false, 10},   // 7: Digit 5 Top
  {false, 11},   // 8: Digit 5 Bottom
  {false, 9},    // 9: Digit 4 Bottom
  {true,  3},    // 10: Dot 1 Bottom
  {false, 7},    // 11: Digit 3 Bottom
  {false, 5},    // 12: Digit 2 Bottom
  {true,  1},    // 13: Dot 0 Bottom
  {false, 3},    // 14: Digit 1 Bottom
  {false, 1}     // 15: Digit 0 Bottom
};

void ScannerSplitEffect::update_position() {
  uint32_t now = esphome::millis();
  if (m_last_frame_ms == 0) {
    m_last_frame_ms = now;
    return;
  }
  uint32_t dt = now - m_last_frame_ms;
  if (dt == 0) return;
  m_last_frame_ms = now;

  float speed = 1.0f / 333.0f; // 333ms per element (25% slower than 250ms)
  m_pos += speed * dt;
  if (m_pos >= 16.0f) {
    m_pos = std::fmod(m_pos, 16.0f);
  }
}

std::array<esphome::Color, NUM_LEDS> ScannerSplitEffect::apply_backlight(esphome::ESPTime time_now, esphome::Color target_color) {
  update_position();

  std::array<esphome::Color, NUM_LEDS> res{};
  float width = 1.5f;

  for (int i = 0; i < 16; i++) {
    const auto &elem = ELEMENTS[i];
    if (elem.is_dot) continue;

    float distance = std::fabs(m_pos - (float)i);
    if (distance > 8.0f) {
      distance = 16.0f - distance;
    }

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

std::array<esphome::Color, NUM_DOTS> ScannerSplitEffect::apply_dots(esphome::ESPTime time_now, esphome::Color target_color) {
  update_position();

  std::array<esphome::Color, NUM_DOTS> res{};
  float width = 1.5f;

  for (int i = 0; i < 16; i++) {
    const auto &elem = ELEMENTS[i];
    if (!elem.is_dot) continue;

    float distance = std::fabs(m_pos - (float)i);
    if (distance > 8.0f) {
      distance = 16.0f - distance;
    }

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
