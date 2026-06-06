#pragma once

#include <time.h>
#include <sys/time.h>

struct NosTimeContext
{
    timeval tv;
    tm ti;
    uint32_t now_ms;

    // Returns precise seconds in current minute (0.000 - 59.999)
    float get_seconds_precise() const
    {
        return (float)(ti.tm_sec) + (float)tv.tv_usec / 1000000.0f;
    }

    // Helper to get specific digit for Nixie/LED sync (0-5: H1, H2, M1, M2, S1, S2)
    uint8_t get_clock_digit(int index) const
    {
        switch (index)
        {
            case 0: return ti.tm_hour / 10;
            case 1: return ti.tm_hour % 10;
            case 2: return ti.tm_min / 10;
            case 3: return ti.tm_min % 10;
            case 4: return ti.tm_sec / 10;
            case 5: return ti.tm_sec % 10;
            default: return 0;
        }
    }
};
