#pragma once

#include <FastLED.h>
#include <memory>
#include "nos_time_context.h"

enum class NosClockEffect
{
    Empty,
    Solid,
    Aurora,
    DigitSync,
    Scanner,
    ProgressBar,
    Fire,
    Count
};

class INosEffect
{
public:
    static constexpr int NUM_LEDS = 12;

    static std::unique_ptr<INosEffect> make_effect(NosClockEffect effect, CRGB color = CRGB::Red);

    virtual ~INosEffect() = default;

    virtual bool loop(CRGB* leds, const NosTimeContext& ctx) = 0;

    NosClockEffect get_effect() const { return m_mode; }
    CRGB get_color() const { return m_color; }

protected:
    INosEffect(NosClockEffect mode, CRGB color)
        : m_mode(mode)
        , m_color(color)
    {
    }

    NosClockEffect m_mode;
    CRGB m_color;
};
