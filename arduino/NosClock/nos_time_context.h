#include <Wire.h>
#include <WiFi.h>
#include "time.h"
#include <math.h>

// WiFi and Time configuration
const char* ssid = "NosynaLair-IOT";
const char* password = "SmartNosyna";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7200;      // Timezone offset in seconds
const int daylightOffset_sec = 3600;  // Daylight saving time offset

// I2C AW9523B Pins and Config
const int sdaPin = 8;
const int sclPin = 9;
const int aw9523Addr = 0x58; 

const byte REG_GCR = 0x11;
const byte REG_LED_P0 = 0x12;
const byte REG_LED_P1 = 0x13;
const byte REG_RESET = 0x7F;          // Software reset register

const byte REG_R[] = {0x26, 0x29, 0x2C, 0x2D};
const byte REG_G[] = {0x27, 0x2A, 0x20, 0x2E};
const byte REG_B[] = {0x28, 0x2B, 0x21, 0x2F};

// ESP32-C3 HV5222 Pins
const int pinClk = 6;
const int pinData = 7;
const int pinEn = 10;

unsigned long lastTimeUpdate = 0;
unsigned long lastLedUpdate = 0;

// I2C write function
void writeI2C(byte reg, byte value) {
  Wire.beginTransmission(aw9523Addr);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Setup AW9523 with robustness checks
void setupAW9523() {
  // 1. Software reset to clear any stuck states
  writeI2C(REG_RESET, 0x00);
  delay(50); // Wait for chip to reboot

  // 2. Configure LEDs and global current
  writeI2C(REG_LED_P0, 0x00);
  writeI2C(REG_LED_P1, 0x00);
  writeI2C(REG_GCR, 0x03); 
}

// Set color for a specific LED index (0 to 3)
void setLedRGB(int index, byte r, byte g, byte b) {
  writeI2C(REG_R[index], r);
  writeI2C(REG_G[index], g);
  writeI2C(REG_B[index], b);
}

// Set color for all LEDs
void setAllRGB(byte r, byte g, byte b) {
  for(int i = 0; i < 4; i++) {
     setLedRGB(i, r, g, b);
  }
}

// Shift 32 bits to HV5222
void shiftOut32(uint32_t data) {
  for (int i = 31; i >= 0; i--) {
    digitalWrite(pinData, (data >> i) & 1);
    digitalWrite(pinClk, HIGH);
    delayMicroseconds(1);
    digitalWrite(pinClk, LOW); 
    delayMicroseconds(1);
  }
}

// Encode 3 independent digits into one 32-bit integer for one HV5222 chip
uint32_t encode3Digits(int d1, int d2, int d3) {
  uint32_t digit1, digit2, digit3;
  
  if (d1 == 0) digit1 = 1UL << 9;
  else digit1 = 1UL << (d1 - 1);
  
  if (d2 == 0) digit2 = 1UL << 19;
  else digit2 = 1UL << (d2 - 1 + 10);
  
  if (d3 == 0) digit3 = 1UL << 31;
  else digit3 = 1UL << (d3 - 1 + 22);

  return digit1 | digit2 | digit3;
}

// Rhythmic 1-second color crossfade effect
void updateColorBeat() {
  // 1000ms cycle for a full transition from one color to the next
  unsigned long ms = millis() % 1000;
  float progress = ms / 1000.0; // linear progress from 0.0 to 1.0

  // Easing function (Cosine wave) makes the transition smooth
  float easedProgress = (1.0 - cos(progress * PI)) / 2.0;

  // Determine current and next color based on the current second
  unsigned long sec = millis() / 1000;
  
  // 6 target hues: Red(0), Yellow(42), Green(85), Cyan(127), Blue(170), Magenta(213)
  uint8_t targetHues[] = {0, 42, 85, 127, 170, 213};
  int numColors = 6;

  uint8_t startHue = targetHues[sec % numColors];
  uint8_t endHue = targetHues[(sec + 1) % numColors];

  // Interpolate hue smoothly
  float currentHue;
  if (startHue == 213 && endHue == 0) {
    // Special case to wrap around the color wheel
    currentHue = 213.0 + easedProgress * (256.0 - 213.0); 
  } else {
    currentHue = startHue + easedProgress * (endHue - startHue);
  }
  
  uint8_t hue = ((int)currentHue) % 256;

  uint8_t r, g, b;
  
  // Color wheel math
  if (hue < 85) {
    r = hue * 3; g = 255 - hue * 3; b = 0;
  } else if (hue < 170) {
    uint8_t h = hue - 85;
    r = 255 - h * 3; g = 0; b = h * 3;
  } else {
    uint8_t h = hue - 170;
    r = 0; g = h * 3; b = 255 - h * 3;
  }

  // Constant brightness limit (150 out of 255)
  int maxBrightness = 150;
  r = (r * maxBrightness) / 255;
  g = (g * maxBrightness) / 255;
  b = (b * maxBrightness) / 255;

  setAllRGB(r, g, b);
}

void setup() {
  Serial.begin(115200);
  
  // Allow power to stabilize before doing anything
  delay(500); 

  // Setup Tubes
  pinMode(pinClk, OUTPUT);
  pinMode(pinData, OUTPUT);
  pinMode(pinEn, OUTPUT);
  
  // Turn off outputs during initialization
  digitalWrite(pinEn, LOW);
  shiftOut32(0);
  shiftOut32(0);
  // Turn on outputs at maximum hardware brightness
  digitalWrite(pinEn, HIGH);

  // Setup I2C Dots
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(100000); // Set standard 100kHz speed for stability

  // Retry loop: ping the AW9523B to ensure it's awake before configuring
  Serial.print("Initializing AW9523B");
  for (int i = 0; i < 5; i++) {
    Wire.beginTransmission(aw9523Addr);
    if (Wire.endTransmission() == 0) {
      Serial.println(" -> OK");
      break; // Chip responded
    }
    Serial.print(".");
    delay(100);
  }
  
  setupAW9523();
  setAllRGB(0, 0, 0);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  // Setup NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  // Update LEDs smoothly every 20ms
  if (millis() - lastLedUpdate >= 20) {
    lastLedUpdate = millis();
    updateColorBeat();
  }

  // Update clock digits every second
  if (millis() - lastTimeUpdate >= 1000) {
    lastTimeUpdate = millis();
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return; 
    }

    // Split time into digits
    int h1 = timeinfo.tm_hour / 10;
    int h2 = timeinfo.tm_hour % 10;
    int m1 = timeinfo.tm_min / 10;
    int m2 = timeinfo.tm_min % 10;
    int s1 = timeinfo.tm_sec / 10;
    int s2 = timeinfo.tm_sec % 10;

    uint32_t leftChipPattern = encode3Digits(h1, h2, m1);
    uint32_t rightChipPattern = encode3Digits(m2, s1, s2);

    // Prevent ghosting: turn off, shift data, turn on at absolute max brightness
    digitalWrite(pinEn, LOW); 
    shiftOut32(rightChipPattern); 
    shiftOut32(leftChipPattern);
    digitalWrite(pinEn, HIGH); 
  }
}