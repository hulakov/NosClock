
#include "nos_time.h"
#include "nos_clock_impl.h"

#include <time.h>
#include <sys/time.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h" 


NosClockImpl clock_logic;
NosTime time_logic;
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);


void setup() {
    Serial.begin(115200);
    
    Wire.begin(8, 9);    
    sensors.begin();
    
    time_logic.setup();
    clock_logic.setup();
    

}

void serial_loop() 
{
    if (Serial.available() == 0) {
        return;
    }

    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() == 0) 
    {
        return;
    }

    // List of primary colors for cycling
    static const CRGB colors[] = {
        CRGB::Red, CRGB::Green, CRGB::Blue, 
        CRGB::Yellow, CRGB::Cyan, CRGB::Magenta, CRGB::White
    };
    static const int num_colors = sizeof(colors) / sizeof(colors[0]);
    static int current_color_idx = 2; // Default to Blue

    char cmd = input[0];

    // Command 'e': Toggle Enabled
    if (cmd == 'e') {
        static bool current_enabled = true;
        current_enabled = !current_enabled;
        clock_logic.enable(current_enabled);
        Serial.printf("Clock state toggled: %s\n", current_enabled ? "ENABLED" : "DISABLED");
    }
    // Command 'm': Cycle Effects Mode
    else if (cmd == 'm') {
        int mode = (static_cast<int>(clock_logic.get_effect_mode()) + 1) % static_cast<int>(NosClockEffect::Count);
        
        // Use the currently selected color instead of hardcoded Blue
        clock_logic.set_effect(static_cast<NosClockEffect>(mode), colors[current_color_idx]);
        Serial.printf("Effect mode changed to: %d (Color index: %d)\n", mode, current_color_idx);
    }
    // Command 'c': Cycle Colors
    else if (cmd == 'c') {
        current_color_idx = (current_color_idx + 1) % num_colors;
        
        // Apply new color to the current effect immediately
        NosClockEffect current_mode = clock_logic.get_effect_mode();
        clock_logic.set_effect(current_mode, colors[current_color_idx]);
        
        Serial.printf("Color changed to index: %d\n", current_color_idx);
    }
    // Numerical Input: Set Brightness
    else if (isdigit(cmd)) {
        int brightness_percent = input.toInt();
        if (brightness_percent < 0) brightness_percent = 0;
        if (brightness_percent > 100) brightness_percent = 100;
        
        clock_logic.set_brightness(brightness_percent);
        Serial.printf("Set brightness: %d%%\n", brightness_percent);
    }
}

void loop() {
    serial_loop();

    time_logic.loop();

    // // --- TEMPERATURE LOGIC ---
    // static unsigned long lastTemp = 0;
    // if (millis() - lastTemp > 60000) {
    //     sensors.requestTemperatures();
    //     Serial.printf("NosClock Temp: %.2f C\n", sensors.getTempCByIndex(0));
    //     lastTemp = millis();
    // }
}

