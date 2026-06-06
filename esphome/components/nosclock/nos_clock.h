#pragma once

#include "nos_led_effect.h"
#include "esphome/core/component.h"
#include <memory>

class NosClockImpl;

namespace nos_clock {

class NosClock : public esphome::Component {
 public:
  NosClock();
  ~NosClock();

  void setup() override;
  void set_brightness(int val);
  void set_effect_brightness(int val);
  void set_enabled(bool state);
  void set_effect(NosClockEffect effect, CRGB color = CRGB::Red);
  NosClockEffect get_effect() const;

 protected:
  std::unique_ptr<NosClockImpl> m_impl;
};

}  // namespace nos_clock