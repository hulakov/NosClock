#pragma once

#include "config.h"
#include <RTClib.h>
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

class NosTime {
public:
    void setup() {
        if (!rtc.begin()) {
            Serial.println("Error: RTC module not found!");
        } else {
            set_system_time_from_rtc();
        }

        // 1. Initialize the timezone first (Ukraine EET/EEST)
        setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
        tzset();

        WiFi.begin(config::SSID, config::PASSWORD);
        
        // Start background NTP synchronization
        configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org");
    }

    void loop() {
        bool isWiFiConnected = (WiFi.status() == WL_CONNECTED);

        // 1. Sync immediately after connecting to WiFi
        if (isWiFiConnected && !wasWiFiConnected) {
            sync_ntp_to_rtc();
        }
        wasWiFiConnected = isWiFiConnected;

        // 2. Hourly sync at minute 00
        time_t now;
        time(&now);
        struct tm* timeinfo = localtime(&now);

        if (isWiFiConnected && timeinfo->tm_min == 0 && timeinfo->tm_hour != lastSyncedHour) {
            if (sync_ntp_to_rtc()) {
                lastSyncedHour = timeinfo->tm_hour;
            }
        }
    }

private:
    RTC_DS3231 rtc;
    bool wasWiFiConnected = false;
    int lastSyncedHour = -1;

    // Helper to log time consistently
    void log_rtc_event(const char* prefix, const DateTime& dt) {
        char buf[25];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 dt.year(), dt.month(), dt.day(),
                 dt.hour(), dt.minute(), dt.second());
        Serial.print(prefix);
        Serial.println(buf);
    }

    // Read UTC from RTC and set the ESP32 system time
    void set_system_time_from_rtc() {
        DateTime now = rtc.now();

        if (now.isValid()) {
            // unixtime() returns seconds, assuming the RTC stores UTC
            struct timeval tv = { .tv_sec = (time_t)now.unixtime(), .tv_usec = 0 };
            settimeofday(&tv, NULL);

            log_rtc_event("RTC -> System Clock (UTC): ", now);
        } else {
            Serial.println("Error: RTC returned invalid data.");
        }
    }

    // Fetch time via NTP and write it to the RTC in UTC format
    bool sync_ntp_to_rtc() {
        if (WiFi.status() != WL_CONNECTED) return false;

        struct tm ti;
        // Wait up to 5s for a valid NTP sync
        if (!getLocalTime(&ti, 5000)) {
            Serial.println("NTP Sync Failed: Local time not yet set");
            return false;
        }

        // Get the already synchronized system time in UTC format
        time_t now;
        time(&now);
        struct tm timeinfo_utc;
        gmtime_r(&now, &timeinfo_utc); // gmtime_r provides clean UTC

        DateTime ntpTime(timeinfo_utc.tm_year + 1900, timeinfo_utc.tm_mon + 1, timeinfo_utc.tm_mday, 
                         timeinfo_utc.tm_hour, timeinfo_utc.tm_min, timeinfo_utc.tm_sec);
        
        rtc.adjust(ntpTime);
        log_rtc_event("NTP -> RTC (Write UTC): ", ntpTime);
        return true;
    }
};