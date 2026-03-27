#include "nos_led_effect.h"
#include <math.h>

// --- Solid Color Effect ---
class SolidEffect : public INosEffect
{
public:
    SolidEffect(CRGB color)
        : INosEffect(NosClockEffect::Solid, color)
    {
    }

    bool loop(CRGB* leds, const NosTimeContext& ctx) override
    {
        if (m_initialized)
        {
            return false;
        }
        m_initialized = true;

        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = m_color;
        }

        return true;
    }

private:
    bool m_initialized = false;
};

// --- Aurora Effect (Rainbow Cycle) ---
class AuroraEffect : public INosEffect
{
public:
    AuroraEffect()
        : INosEffect(NosClockEffect::Aurora, CRGB::Black)
    {
    }

    bool loop(CRGB* leds, const NosTimeContext& ctx) override
    {
        if (ctx.now_ms - m_last_update < 40)
        {
            return false;
        }
        m_last_update = ctx.now_ms;

        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CHSV(m_hue + (i * 255 / NUM_LEDS), 200, 255);
        }
        m_hue += 2;

        return true;
    }

private:
    uint8_t m_hue = 0;
    uint32_t m_last_update = 0;
};

// --- Digit Sync Effect (Color tied to Nixie digits) ---
class DigitSyncEffect : public INosEffect
{
public:
    DigitSyncEffect()
        : INosEffect(NosClockEffect::DigitSync, CRGB::Black)
    {
    }

    bool loop(CRGB* leds, const NosTimeContext& ctx) override
    {
        if (m_last_sec == ctx.ti.tm_sec)
        {
            return false;
        }
        m_last_sec = ctx.ti.tm_sec;

        static constexpr CRGB palette[10] = {
            CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Cyan,
            CRGB::Blue, CRGB::Purple, CRGB::Magenta, CRGB::DeepPink, CRGB::White
        };

        for (int i = 0; i < 6; i++)
        {
            CRGB digit_color = palette[ctx.get_clock_digit(i)];
            // Apply to both LEDs in the column (bottom and top)
            leds[i * 2] = digit_color;
            leds[i * 2 + 1] = digit_color;
        }

        return true;
    }

private:
    int m_last_sec = -1;
};

// --- Scanner Effect (Cylon / KITT style) ---
class ScannerEffect : public INosEffect
{
public:
    ScannerEffect(CRGB color, float speed = 0.04f)
        : INosEffect(NosClockEffect::Scanner, color)
        , m_speed(speed)
    {
    }

    bool loop(CRGB* leds, const NosTimeContext& ctx) override
    {
        if (ctx.now_ms - m_last_ms < 16)
        {
            return false;
        }
        m_last_ms = ctx.now_ms;

        // Reset frame
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB::Black;
        }

        // Update movement
        m_pos += (m_speed * m_direction);

        // Bounce logic
        if (m_pos <= 0.0f || m_pos >= 5.0f)
        {
            m_direction *= -1.0f;
            m_pos = (m_pos <= 0.0f) ? 0.0f : 5.0f;
        }

        // Draw with linear falloff across 6 columns
        for (int col = 0; col < 6; col++)
        {
            float distance = fabsf(m_pos - (float)col);
            if (distance < 1.0f)
            {
                uint8_t bri = (uint8_t)(255.0f * (1.0f - distance));
                CRGB col_color = m_color;
                col_color.nscale8(bri);

                leds[col * 2] = col_color;
                leds[col * 2 + 1] = col_color;
            }
        }

        return true;
    }

private:
    float m_pos = 0.0f;
    float m_direction = 1.0f;
    float m_speed;
    uint32_t m_last_ms = 0;
};

// --- Progress Bar Effect (Atomic Minute Visualization) ---
class ProgressBarEffect : public INosEffect
{
public:
    ProgressBarEffect(CRGB color)
        : INosEffect(NosClockEffect::ProgressBar, color)
    {
    }

    bool loop(CRGB* leds, const NosTimeContext& ctx) override
    {
        if (ctx.now_ms - m_last_ms < 20)
        {
            return false;
        }
        m_last_ms = ctx.now_ms;

        // Use precise atomic seconds from context (0.0 - 59.999)
        float active_cols = (ctx.get_seconds_precise() / 60.0f) * 6.0f;

        for (int col = 0; col < 6; col++)
        {
            float diff = active_cols - (float)col;
            uint8_t intensity = 0;

            if (diff >= 1.0f)
            {
                intensity = 255;
            }
            else if (diff > 0.0f)
            {
                intensity = (uint8_t)(diff * 255.0f);
            }

            leds[col * 2] = m_color;
            leds[col * 2].nscale8(intensity);
            leds[col * 2 + 1] = m_color;
            leds[col * 2 + 1].nscale8(intensity);
        }

        return true;
    }

private:
    uint32_t m_last_ms = 0;
};

// --- Fire Effect (Perlin Noise) ---
class FireEffect : public INosEffect
{
public:
    FireEffect()
        : INosEffect(NosClockEffect::Fire, CRGB::Orange)
    {
    }

    bool loop(CRGB* leds, const NosTimeContext& ctx) override
    {
        if (ctx.now_ms - m_last_ms < 30)
        {
            return false;
        }
        m_last_ms = ctx.now_ms;

        for (int col = 0; col < 6; col++)
        {
            // Generate flicker using coordinates and system uptime
            uint8_t heat = inoise8(col * 60, ctx.now_ms / 4);
            
            // Bottom LED is "hotter"
            leds[col * 2] = ColorFromPalette(HeatColors_p, qadd8(heat, 60));
            // Top LED is "cooler" flame tip
            leds[col * 2 + 1] = ColorFromPalette(HeatColors_p, qsub8(heat, 50));
        }

        return true;
    }

private:
    uint32_t m_last_ms = 0;
};

// --- Factory Method Implementation ---
std::unique_ptr<INosEffect> INosEffect::make_effect(NosClockEffect effect, CRGB color)
{
    switch (effect)
    {
        case NosClockEffect::Solid:
            return std::make_unique<SolidEffect>(color);
        case NosClockEffect::Aurora:
            return std::make_unique<AuroraEffect>();
        case NosClockEffect::DigitSync:
            return std::make_unique<DigitSyncEffect>();
        case NosClockEffect::Scanner:
            return std::make_unique<ScannerEffect>(color);
        case NosClockEffect::ProgressBar:
            return std::make_unique<ProgressBarEffect>(color);
        case NosClockEffect::Fire:
            return std::make_unique<FireEffect>();
        default:
            return std::make_unique<SolidEffect>(color);
    }
}