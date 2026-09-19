#include "nos_tubes.h"
#include <driver/gpio.h>
#include <esp_rom_sys.h>
#include <cmath>

namespace nosclock {

void NosTubes::setup() {
  // Initialize shift register pins GPIO6 (Clk), GPIO7 (Data) as outputs in ESP-IDF
  gpio_set_direction(GPIO_NUM_6, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
}

uint32_t NosTubes::get_single_digit_bit(int num, int pos) {
  switch (num) {
    case 0:
      switch (pos) {
        case 0: return 1UL << 9;
        case 1: return 1UL << 19;
        case 2: return 1UL << 31;
        default: return 0;
      }
    default:
      switch (pos) {
        case 0: return 1UL << (num - 1);
        case 1: return 1UL << (num - 1 + 10);
        case 2: return 1UL << (num - 1 + 22);
        default: return 0;
      }
  }
}

uint32_t NosTubes::get_3_digits(int d1, int d2, int d3) {
  return get_single_digit_bit(d1, 0) | get_single_digit_bit(d2, 1) | get_single_digit_bit(d3, 2);
}

void NosTubes::shift_out_32(uint32_t data) {
  for (int i = 31; i >= 0; i--) {
    gpio_set_level(GPIO_NUM_7, (data >> i) & 1);
    gpio_set_level(GPIO_NUM_6, 1);
    esp_rom_delay_us(1);
    gpio_set_level(GPIO_NUM_6, 0);
  }
}

void NosTubes::update(esphome::ESPTime time_now, bool enabled, float brightness, esphome::output::FloatOutput *tubes_en) {
  int hours = time_now.hour;
  int minutes = time_now.minute;
  int seconds = time_now.second;

  bool tubes_need_update = (seconds != m_last_second ||
                            minutes != m_last_minute ||
                            hours != m_last_hour ||
                            enabled != m_last_enabled ||
                            std::abs(brightness - m_last_brightness) >= 0.01f);

  if (tubes_need_update) {
    m_last_second = seconds;
    m_last_minute = minutes;
    m_last_hour = hours;
    m_last_enabled = enabled;
    m_last_brightness = brightness;

    if (enabled) {
      // Turn off EN (GPIO10) before shift to prevent ghosting
      tubes_en->set_level(0.0f);

      // Shift data (64 bits total)
      shift_out_32(get_3_digits(minutes % 10, seconds / 10, seconds % 10)); // Last 3 digits
      shift_out_32(get_3_digits(hours / 10, hours % 10, minutes / 10));     // First 3 digits

      // Restore EN to active brightness
      tubes_en->set_level(brightness);
    } else {
      tubes_en->set_level(0.0f);
    }
  }
}

} // namespace nosclock
