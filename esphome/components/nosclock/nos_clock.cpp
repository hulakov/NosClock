#include "nos_clock.h"
#include "nos_clock_impl.h"

namespace nos_clock
{

// Реалізація методів NosClock (Wrapper)
NosClock::NosClock() : m_impl(std::make_unique<NosClockImpl>()) {}
NosClock::~NosClock() = default;

void NosClock::setup() { m_impl->setup(); }
void NosClock::set_brightness(int val) { m_impl->set_brightness(val); }
void NosClock::set_effect_brightness(int val) { m_impl->set_effect_brightness(val); }
void NosClock::set_enabled(bool state) { m_impl->enable(state); }
void NosClock::set_effect(NosClockEffect effect, CRGB color) { m_impl->set_effect(effect, color); }
NosClockEffect NosClock::get_effect() const { return m_impl->get_effect_mode(); }

}