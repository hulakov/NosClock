#include "nos_controller.h"
#include "esphome/core/log.h"

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
    float brightness = color_values.get_brightness();
    esphome::Color target_color(
      color_values.get_red() * 255.0f * brightness,
      color_values.get_green() * 255.0f * brightness,
      color_values.get_blue() * 255.0f * brightness
    );
    std::array<esphome::Color, NUM_LEDS> backlight_colors = active_effect.apply_backlight(time_now, target_color);
    for (size_t i = 0; i < backlight_colors.size(); i++) {
      if (i < addressable->size()) {
        if ((*addressable)[i].get() != backlight_colors[i]) {
          (*addressable)[i] = backlight_colors[i];
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
  std::array<esphome::Color, NUM_DOTS> dots{};
  if (dots_color_values.is_on()) {
    float brightness = dots_color_values.get_brightness();
    esphome::Color target_color(
      dots_color_values.get_red() * 255.0f * brightness,
      dots_color_values.get_green() * 255.0f * brightness,
      dots_color_values.get_blue() * 255.0f * brightness
    );

    dots = active_effect.apply_dots(time_now, target_color);
  } else {
    for (size_t i = 0; i < dots.size(); i++) {
      dots[i] = esphome::Color(0, 0, 0);
    }
  }

  for (size_t i = 0; i < dots.size(); i++) {
    esphome::Color scaled_color(
      dots[i].r * DOTS_BRIGHTNESS_LIMIT,
      dots[i].g * DOTS_BRIGHTNESS_LIMIT,
      dots[i].b * DOTS_BRIGHTNESS_LIMIT
    );
    m_aw9523.set_dot_color(i, scaled_color);
  }
}

void NosController::update_clock(
    esphome::ESPTime time_now,
    esphome::light::LightState *tubes_light,
    esphome::light::LightState *backlight_strip,
    esphome::light::LightState *dots_strip,
    esphome::output::FloatOutput *tubes_en,
    const std::string &effect_name
) {
  bool enabled = false;
  float tubes_brightness = 0.0f;
  if (tubes_light != nullptr) {
    enabled = tubes_light->remote_values.is_on();
    tubes_brightness = tubes_light->remote_values.get_brightness();
  }

  // 1. Update Nixie tubes
  m_nos_tubes.update(time_now, enabled, tubes_brightness, tubes_en);

  // 2. Resolve pointers and values
  auto *addressable = (backlight_strip != nullptr) ? 
    (esphome::light::AddressableLight *) backlight_strip->get_output() : nullptr;

  esphome::light::LightColorValues color_values;
  if (backlight_strip != nullptr) {
    color_values = backlight_strip->remote_values;
  }

  esphome::light::LightColorValues dots_color_values;
  if (dots_strip != nullptr) {
    dots_color_values = dots_strip->remote_values;
  }

  INosEffect &active_effect = resolve_effect(effect_name);

  if (backlight_strip != nullptr && backlight_strip->get_effect_name() != "NosClock") {
    auto call = backlight_strip->make_call();
    call.set_effect("NosClock");
    call.perform();
  }

  // 3. Update Backlight
  update_backlight(addressable, color_values, time_now, active_effect);

  // 4. Update Dots
  update_dots(dots_color_values, time_now, active_effect);
}

} // namespace nosclock
