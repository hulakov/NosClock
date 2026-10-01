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
    float r = 0.0f, g = 0.0f, b = 0.0f;
    color_values.as_rgb(&r, &g, &b);
    esphome::Color target_color(
      (uint8_t)(r * 255.0f),
      (uint8_t)(g * 255.0f),
      (uint8_t)(b * 255.0f)
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

static DotsMode parse_dots_mode(const std::string &mode_str) {
  if (mode_str == "On") {
    return DotsMode::ON;
  }
  if (mode_str == "Off") {
    return DotsMode::OFF;
  }
  return DotsMode::BLINK;
}

void NosController::update_dots(esphome::light::LightColorValues dots_color_values, esphome::ESPTime time_now, INosEffect &active_effect, DotsMode dots_mode) {
  std::array<esphome::Color, NUM_DOTS> dots{};

  if (dots_mode == DotsMode::OFF || !dots_color_values.is_on()) {
    for (size_t i = 0; i < dots.size(); i++) {
      m_aw9523.set_dot_color(i, esphome::Color(0, 0, 0));
    }
    return;
  }

  if (dots_mode == DotsMode::BLINK && time_now.second % 2 != 0) {
    for (size_t i = 0; i < dots.size(); i++) {
      m_aw9523.set_dot_color(i, esphome::Color(0, 0, 0));
    }
    return;
  }

  float r = 0.0f, g = 0.0f, b = 0.0f;
  dots_color_values.as_rgb(&r, &g, &b);
  esphome::Color target_color(
    (uint8_t)(r * 255.0f),
    (uint8_t)(g * 255.0f),
    (uint8_t)(b * 255.0f)
  );

  esphome::ESPTime effect_time = time_now;
  if (dots_mode != DotsMode::BLINK && effect_time.second % 2 != 0) {
    effect_time.second = 0;
  }

  dots = active_effect.apply_dots(effect_time, target_color);

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
    const std::string &effect_name,
    DotsMode dots_mode
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
  if (backlight_strip != nullptr) {
    color_values = backlight_strip->current_values;
  }

  esphome::light::LightColorValues dots_color_values;
  if (dots_strip != nullptr) {
    dots_color_values = dots_strip->current_values;
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
  update_dots(dots_color_values, time_now, active_effect, dots_mode);
}

void NosController::update_clock(
    esphome::ESPTime time_now,
    esphome::light::LightState *tubes_light,
    esphome::light::LightState *backlight_strip,
    esphome::light::LightState *dots_strip,
    esphome::output::FloatOutput *tubes_en,
    const std::string &effect_name,
    const std::string &dots_mode_str
) {
  update_clock(
    time_now,
    tubes_light,
    backlight_strip,
    dots_strip,
    tubes_en,
    effect_name,
    parse_dots_mode(dots_mode_str)
  );
}

} // namespace nosclock
