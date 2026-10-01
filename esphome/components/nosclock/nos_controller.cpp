#include "nos_controller.h"
#include "esphome/core/log.h"
#include <cmath>

namespace nosclock {

static const char *const TAG = "nosclock";

void NosController::setup() {
  // Empty, hardware is initialized dynamically via setup_hardware
}

void NosController::dump_config() {
  ESP_LOGCONFIG(TAG, "NosClock Controller initialized");
}

void NosController::setup_hardware(esphome::i2c::I2CBus *bus) {
  m_nos_tubes.setup();
  m_aw9523.setup(bus);
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
        esphome::Color final_color = raw_colors[i].to_color(user_brightness);
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
  float dots_combined_factor = user_brightness * DOTS_BRIGHTNESS_LIMIT;

  ColorFloat pure_user_color(
    dots_color_values.get_red(),
    dots_color_values.get_green(),
    dots_color_values.get_blue()
  );

  dots = active_effect.apply_dots(time_now, pure_user_color);

  for (size_t i = 0; i < dots.size(); i++) {
    esphome::Color final_dot_color = dots[i].to_color(dots_combined_factor);
    m_aw9523.set_dot_color(i, final_dot_color);
  }
}

void NosController::update_clock(
    esphome::ESPTime time_now,
    esphome::light::LightState *tubes_light,
    esphome::light::LightState *backlight_strip,
    esphome::output::FloatOutput *tubes_en
) {
  bool enabled = false;
  float tubes_brightness = 0.0f;
  if (tubes_light != nullptr) {
    enabled = tubes_light->current_values.is_on();
    tubes_brightness = tubes_light->current_values.get_brightness();
  }

  // 1. Update Nixie tubes
  m_nos_tubes.update(time_now, enabled, tubes_brightness, tubes_en);

  // 2. Resolve pointers and values
  auto *addressable = (backlight_strip != nullptr) ? 
    (esphome::light::AddressableLight *) backlight_strip->get_output() : nullptr;

  esphome::light::LightColorValues color_values;
  std::string effect_name = "Solid";
  if (backlight_strip != nullptr) {
    color_values = backlight_strip->current_values;
    std::string active_effect_name = backlight_strip->get_effect_name();
    if (!active_effect_name.empty() && active_effect_name != "None") {
      effect_name = active_effect_name;
    }
  }

  INosEffect &active_effect = resolve_effect(effect_name);

  // 3. Update Backlight
  update_backlight(addressable, color_values, time_now, active_effect);

  // 4. Update Dots (using backlight color and brightness values)
  update_dots(color_values, time_now, active_effect);
}

} // namespace nosclock
