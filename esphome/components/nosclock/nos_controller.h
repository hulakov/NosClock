#pragma once
#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_state.h"

#include "nos_effect.h"
#include "effect_solid.h"
#include "effect_digit_sync.h"
#include "effect_scanner.h"
#include "effect_scanner_dual.h"
#include "effect_scanner_split.h"
#include "aw9523.h"
#include "nos_tubes.h"

namespace nosclock {

class NosController : public esphome::Component {
 public:
   void setup() override;
   void dump_config() override;

   void setup_hardware(esphome::i2c::I2CBus *bus);
   void update_clock(
       esphome::ESPTime time_now,
       esphome::light::LightState *tubes_light,
       esphome::light::LightState *backlight_strip,
       esphome::light::LightState *dots_strip,
       esphome::output::FloatOutput *tubes_en,
       const std::string &effect_name
   );

 private:
   INosEffect &resolve_effect(const std::string &effect_name);
   void update_backlight(esphome::light::AddressableLight *addressable, esphome::light::LightColorValues color_values, esphome::ESPTime time_now, INosEffect &active_effect);
   void update_dots(esphome::light::LightColorValues dots_color_values, esphome::ESPTime time_now, INosEffect &active_effect);

   NosTubes m_nos_tubes;
   Aw9523 m_aw9523;
   SolidEffect m_solid_effect{true, false};
   SolidEffect m_static_effect{false, false};
   DigitSyncEffect m_digit_sync_effect;
   ScannerEffect m_scanner_effect;
   ScannerDualEffect m_scanner_dual_effect;
   ScannerSplitEffect m_scanner_split_effect;
   SolidEffect m_cycle_effect{true, true};
};

} // namespace nosclock
