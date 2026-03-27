#include <memory>
#pragma once

#include "nos_led_effect.h"
#include "nos_time_context.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

class NosClockImpl
{
private:
    // Pin Definitions
    static constexpr int DATA_PIN = 7;
    static constexpr int CLOCK_PIN = 6;
    static constexpr int LATCH_PIN = 10;
    static constexpr int COLON_PIN = 5;
    static constexpr int SK6812_PIN = 21;    
    static constexpr int LAMP_PINS[3] = { 0, 1, 3 };

    // Timing Constants
    static constexpr int DIGIT_PERIOD_US = 1400;
    static constexpr int MIN_DIGIT_ON_TIME_US = 70;
    static constexpr int MIN_BLANKING_US = 40;
    static constexpr uint32_t COLON_PWM_FREQ = 500;
    static constexpr uint8_t COLON_PWM_RES = 8;
    static constexpr int NUM_LEDS = INosEffect::NUM_LEDS;

public:
    NosClockImpl()
    {
    }

    void setup()
    {
        pinMode(DATA_PIN, OUTPUT);
        pinMode(CLOCK_PIN, OUTPUT);
        pinMode(LATCH_PIN, OUTPUT);

        for (int i = 0; i < 3; i++)
        {
            pinMode(LAMP_PINS[i], OUTPUT);
            digitalWrite(LAMP_PINS[i], LOW);
        }
        
        pinMode(COLON_PIN, OUTPUT);
        ledcAttach(COLON_PIN, COLON_PWM_FREQ, COLON_PWM_RES);

        FastLED.addLeds<WS2812B, SK6812_PIN, GRB>(m_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

        set_brightness(m_brightness);

        // Task for driving both Nixies and LEDs
        xTaskCreate(NosClockImpl::task_wrapper, "nos_clock_task", 4096, this, 10, nullptr);
    }

    void enable(bool v)
    {
        m_enabled = v;

        if (!m_enabled)
        {
            for (int i = 0; i < 3; i++)
            {
                digitalWrite(LAMP_PINS[i], LOW);
            }
            
            ledcWrite(COLON_PIN, 0);
            FastLED.clear();
            FastLED.show();
            
            ESP_LOGD("NosClock", "Hardware blanking: OFF");
        }
        else
        {
            update_colon_pwm();
            ESP_LOGD("NosClock", "Hardware restored: ON");
        }
    }

    void set_brightness(int v)
    {
        m_brightness = v;
        update_colon_pwm();
        
        // Disable LEDs if brightness is too low, or set fixed overhead
        FastLED.setBrightness(v <= 20 ? 0 : 120);
        FastLED.show();
    }

    void set_effect(NosClockEffect effect, CRGB color = CRGB::Red)
    {
        m_effect = INosEffect::make_effect(effect, color);
    }

    NosClockEffect get_effect_mode() const
    {
        return m_effect->get_effect();
    }

private:
    CRGB m_leds[NUM_LEDS];
    int m_brightness = 100;
    bool m_enabled = true;
    std::unique_ptr<INosEffect> m_effect = INosEffect::make_effect(NosClockEffect::DigitSync);

    static void task_wrapper(void *pv)
    {
        static_cast<NosClockImpl *>(pv)->run_loop();
    }

    void update_colon_pwm()
    {
        ledcWrite(COLON_PIN, (uint32_t)(m_brightness * 2.55f));
    }

    void run_loop()
    {
        int step = 0;
        for (;;)
        {
            if (!m_enabled)
            {
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }

            TickType_t xLastWakeTime = xTaskGetTickCount(); 

            // 1. Capture atomic time snapshot
            NosTimeContext ctx;
            gettimeofday(&ctx.tv, NULL);
            localtime_r(&ctx.tv.tv_sec, &ctx.ti);
            ctx.now_ms = millis();

            // 2. Perform Nixie Multiplexing step
            step = (step + 1) % 3;
            perform_mux_step(step, ctx);

            // 3. Update LED effects (every 3 steps to maintain sync)
            if (step == 0)
            {
                if (m_effect->loop(m_leds, ctx))
                {
                    FastLED.show();
                }
            }
            
            xTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3));
        }
    }

    void IRAM_ATTR perform_mux_step(int step, const NosTimeContext& ctx)
    {
        int on, off;
        calculate_digit_timings(on, off);

        // Pre-blanking to avoid ghosting
        esp_rom_delay_us(off);

        uint8_t dL, dR;
        get_digits_for_step(step, dL, dR, ctx);

        // Prepare shift register data
        digitalWrite(LATCH_PIN, LOW);
        uint8_t out = (dR << 4) | (dL & 0x0F);
        shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, out);
        digitalWrite(LATCH_PIN, HIGH);

        // Enable specific lamp pair for calculated 'on' time
        noInterrupts(); 
        digitalWrite(LAMP_PINS[step], HIGH);
        esp_rom_delay_us(on);
        digitalWrite(LAMP_PINS[step], LOW);
        interrupts();
    }

    void calculate_digit_timings(int &on, int &off)
    {
        on = (DIGIT_PERIOD_US * m_brightness) / 100;
        
        if (on < MIN_DIGIT_ON_TIME_US)
        {
            on = MIN_DIGIT_ON_TIME_US;
        }
        
        off = max(MIN_BLANKING_US, DIGIT_PERIOD_US - on);
        on = DIGIT_PERIOD_US - off;
    }

    void get_digits_for_step(int muxStep, uint8_t &left, uint8_t &right, const NosTimeContext& ctx)
    {
        uint8_t h = ctx.ti.tm_hour;
        uint8_t m = ctx.ti.tm_min;
        uint8_t s = ctx.ti.tm_sec;

        // Glitch (Slot Machine) animation at the end of every 10th minute
        if (s == 59 && m % 10 == 9)
        {
            uint8_t anim = (uint8_t)((millis() % 1000) / 100);
            h = get_glitched_val(h, anim);
            m = get_glitched_val(m, anim);
            s = get_glitched_val(s, anim);
        }

        // Multiplexing logic: map time to specific lamp pairs
        if (muxStep == 2)
        {
            left = h / 10;
            right = m % 10;
        }
        else if (muxStep == 1)
        {
            left = h % 10;
            right = s / 10;
        }
        else
        {
            left = m / 10;
            right = s % 10;
        }
    }

    uint8_t get_glitched_val(uint8_t val, uint8_t step)
    {
        uint8_t t = (val / 10 + step) % 10;
        uint8_t u = (val % 10 + step) % 10;
        return (t * 10) + u;
    }
};