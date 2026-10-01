#include "nos_controller.h"
#include "esphome/core/log.h"
#include <cmath>

namespace nosclock {

static const char *const TAG = "nosclock";

void NosController::setup() {
  m_nos_tubes.setup();
  if (m_i2c_bus != nullptr) {
    m_aw9523.setup(m_i2c_bus);
  }
}

void NosController::dump_config() {
  ESP_LOGCONFIG(TAG, "NosClock Controller initialized");
}

INosEffect &NosController::resolve_effect(const std::string &effect_name) {
  if (effect_name == "Static") {
    return m_static_effect;
  } else if (effect_name == "DigitSync") {
    return m_digit_sync_effect;
  } else if (effect_name == "Scanner") {
    return m_scanner_effect;
  } else if (effect_name == "ScannerDual") {
    return m_scanner_dual_effect;
  } else if (effect_name == "ScannerSplit") {
    return m_scanner_split_effect;
  } else if (effect_name == "Cycle") {
    return m_cycle_effect;
  }
  return m_solid_effect;
}

void NosController::update_backlight(esphome::light::AddressableLight *addressable, esphome::light::LightColorValues color_values, esphome::ESPTime time_now, INosEffect &active_effect) {
  if (addressable == nullptr) return;

  bool changed = false;
  if (color_values.is_on()) {
    float user_brightness = color_values.get_brightness();
    ColorFloat pure_user_color(
      color_values.get_red(),
      color_values.get_green(),
      color_values.get_blue()
    );

    std::array<ColorFloat, NUM_LEDS> raw_colors = active_effect.apply_backlight(time_now, pure_user_color);
    for (size_t i = 0; i < raw_colors.size(); i++) {
      if (i < addressable->size()) {
        float r_norm = raw_colors[i].r * user_brightness;
        float g_norm = raw_colors[i].g * user_brightness;
        float b_norm = raw_colors[i].b * user_brightness;

        uint8_t r_out = (r_norm > 0.001f) ? (uint8_t) std::round(BACKLIGHT_MIN_RAW + r_norm * (BACKLIGHT_MAX_RAW - BACKLIGHT_MIN_RAW)) : 0;
        uint8_t g_out = (g_norm > 0.001f) ? (uint8_t) std::round(BACKLIGHT_MIN_RAW + g_norm * (BACKLIGHT_MAX_RAW - BACKLIGHT_MIN_RAW)) : 0;
        uint8_t b_out = (b_norm > 0.001f) ? (uint8_t) std::round(BACKLIGHT_MIN_RAW + b_norm * (BACKLIGHT_MAX_RAW - BACKLIGHT_MIN_RAW)) : 0;

        esphome::Color final_color(r_out, g_out, b_out);
        if ((*addressable)[i].get() != final_color) {
          (*addressable)[i] = final_color;
          changed = true;
        }
      }
    }
  } else {
    for (int i = 0; i < addressable->size(); i++) {
      if ((*addressable)[i].get() != esphome::Color(0, 0, 0)) {
        (*addressable)[i] = esphome::Color(0, 0, 0);
        changed = true;
      }
    }
  }

  if (changed) {
    addressable->schedule_show();
  }
}

void NosController::update_dots(esphome::light::LightColorValues dots_color_values, esphome::ESPTime time_now, INosEffect &active_effect) {
  std::array<ColorFloat, NUM_DOTS> dots{};

  if (!dots_color_values.is_on()) {
    for (size_t i = 0; i < dots.size(); i++) {
      m_aw9523.set_dot_color(i, esphome::Color(0, 0, 0));
    }
    return;
  }

  float user_brightness = dots_color_values.get_brightness();

  ColorFloat pure_user_color(
    dots_color_values.get_red(),
    dots_color_values.get_green(),
    dots_color_values.get_blue()
  );

  dots = active_effect.apply_dots(time_now, pure_user_color);

  for (size_t i = 0; i < dots.size(); i++) {
    float r_norm = dots[i].r * user_brightness;
    float g_norm = dots[i].g * user_brightness;
    float b_norm = dots[i].b * user_brightness;

    uint8_t r_out = (r_norm > 0.001f) ? (uint8_t) std::round(DOTS_R_MIN_RAW + r_norm * (DOTS_R_MAX_RAW - DOTS_R_MIN_RAW)) : 0;
    uint8_t g_out = (g_norm > 0.001f) ? (uint8_t) std::round(DOTS_G_MIN_RAW + g_norm * (DOTS_G_MAX_RAW - DOTS_G_MIN_RAW)) : 0;
    uint8_t b_out = (b_norm > 0.001f) ? (uint8_t) std::round(DOTS_B_MIN_RAW + b_norm * (DOTS_B_MAX_RAW - DOTS_B_MIN_RAW)) : 0;

    esphome::Color final_dot_color(r_out, g_out, b_out);
    m_aw9523.set_dot_color(i, final_dot_color);
  }
}

void NosController::update() {
  if (m_time == nullptr) return;

  auto time_now = m_time->now();
  if (!time_now.is_valid()) return;

  bool enabled = m_clock_enabled;
  float tubes_brightness = 0.0f;
  if (m_tubes_light != nullptr) {
    enabled = enabled && m_tubes_light->current_values.is_on();
    tubes_brightness = m_tubes_light->current_values.get_brightness();
  }

  // 1. Update Nixie tubes
  m_nos_tubes.update(time_now, enabled, tubes_brightness, m_tubes_en);

  // 2. Resolve pointers and values
  auto *addressable = (m_backlight_strip != nullptr) ? 
    (esphome::light::AddressableLight *) m_backlight_strip->get_output() : nullptr;

  esphome::light::LightColorValues color_values;
  std::string effect_name = "Solid";
  if (m_backlight_strip != nullptr) {
    color_values = m_backlight_strip->current_values;
    std::string active_effect_name = m_backlight_strip->get_effect_name();
    if (!active_effect_name.empty() && active_effect_name != "None") {
      effect_name = active_effect_name;
    }
  }

  if (!m_clock_enabled) {
    color_values.set_state(false);
  }

  INosEffect &active_effect = resolve_effect(effect_name);

  // 3. Update Backlight
  update_backlight(addressable, color_values, time_now, active_effect);

  // 4. Update Dots (using backlight color and brightness values)
  update_dots(color_values, time_now, active_effect);
}

} // namespace nosclock
