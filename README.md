# NosClock

**NosClock** is a smart 6-digit Nixie tube clock that seamlessly bridges **authentic retro aesthetics with cutting-edge modern electronics**. Powered by the **ESP32-C3** RISC-V microcontroller, it pairs iconic vintage **IN-12 cold-cathode Nixie display tubes** with state-of-the-art integrated circuits — including high-voltage shift register drivers, precision RTC timekeeping, addressable RGB LEDs, and expansion options for environmental sensors.

By combining retro industrial charm with modern IoT capabilities, NosClock runs on **ESPHome** for native Home Assistant smart home automation, offering dynamic visual backlighting effects and customizable sensors.

The custom hardware PCB was designed in **KiCad**, and the matching enclosure was designed in **Autodesk Fusion 360**. The hardware features an onboard 170V DC high-voltage boost converter, high-side anode switching, 32-channel shift register drivers, and efficient power regulation from a single 12V DC input.

> [!CAUTION]
> **HIGH VOLTAGE WARNING**: This hardware generates **170V DC** high voltage to power the Nixie tubes. High voltage can cause severe electric shock, serious injury, electrocution, or property damage. **Do not touch open PCB contacts or high-voltage nodes while powered.** Always ensure power is completely disconnected before touching, servicing, or modifying the hardware.

![NosClock Assembly](images/clock.jpg)

---

## ✨ Key Features

- **Flicker-Free High-Voltage Shift Register Driving**:
  Utilizes dedicated high-voltage shift register decoders (**HV5222PJ**) instead of traditional tube multiplexing. This eliminates high-frequency display flickering completely, significantly enhancing visual perception and comfort. Additionally, direct driving allows reaching ultra-low minimum tube brightness levels, perfect for an unobtrusive night mode.

- **Dual RGB LEDs per Digit & Addressable RGB Colons**:
  Each Nixie tube digit socket is illuminated by **two independent SK6812 MINI addressable RGB LEDs**, and every colon dot is also an RGB LED (4 colon dots driven via the **AW9523B** I2C driver). This dual-LED configuration unlocks rich visual lighting effects (*DigitSync*, *Aurora*, *Scanner*, *ProgressBar*) with fine-grained color control.

- **Smart Home Integration (Home Assistant via ESPHome)**:
  Full native ESPHome integration allows seamless remote control of tube brightness, backlighting colors, effects, and display power directly from Home Assistant. Enables powerful smart home automations such as:
  - **Air Raid Alerts (Повітряна Тривога)** visual flashing notifications.
  - **High CO2 Warnings** (automatic visual alert when indoor CO2 levels rise).
  - **Night Mode Automation** (automatically reducing brightness during sleeping hours).

- **Dual Time Synchronization (NTP + Battery-Backed RTC)**:
  Time is automatically synchronized via **NTP** over Wi-Fi whenever an internet connection is available. For offline reliability and power loss recovery, an onboard Real-Time Clock (**DS3231MZ / DS1307**) powered by a CR1220 backup battery maintains continuous timekeeping.

- **Expandable External I2C Sensor Interface**:
  Features a dedicated onboard 4-pin I2C expansion connector (`J103`), enabling effortless connection of additional hardware sensors — such as the **Sensirion SCD4x** CO2, temperature, and humidity sensor module for real-time indoor air quality monitoring.

---

## Technical Specifications

### Custom PCB & Hardware Architecture

NosClock is powered by a custom-designed printed circuit board (PCB) engineered in **KiCad**. The PCB integrates high-voltage boost conversion, high-side transistor switching, digital logic level conversion, addressable RGB backlighting, and sensor expansion onto a single compact board with optimized ground isolation.

![NosClock Custom PCB](images/pcb.jpg)

#### Component Breakdown

- **Microcontroller**: Espressif [ESP32-C3-MINI-1](https://www.espressif.com/sites/default/files/documentation/esp32-c3-mini-1_datasheet_en.pdf) module (RISC-V 32-bit single-core CPU, 2.4GHz Wi-Fi 4, and Bluetooth 5 LE).
- **Nixie Display Tubes**: 6x **IN-12B** cold-cathode Nixie tubes arranged in 3 multiplexed digit pairs with anti-ghosting pre-blanking timing and automatic slot-machine digit burn-in protection sweeps.
- **High-Voltage Boost Supply**: Onboard step-up converter built around the [UC3843](https://www.ti.com/lit/ds/symlink/uc3843.pdf) current-mode PWM controller and [FQD12N20L](https://www.onsemi.com/pdf/datasheet/fqd12n20l-d.pdf) 200V N-channel MOSFET, converting 12V DC to **170V DC** for Nixie tube anodes.
- **Low-Voltage Power Supply**:
  - [MP2307](https://www.monolithicpower.com/en/documentview/productdocument/index/doc_url/%2Fm%2Fp%2Fmp2307_r1.9.pdf) 3A synchronous step-down buck converter stepping 12V DC input down to 5V DC.
  - [AP7361-33](https://www.diodes.com/assets/Datasheets/AP7361.pdf) 1A low-dropout (LDO) linear regulator converting 5V DC to 3.3V DC for the ESP32-C3 MCU and digital logic.
- **Display Driver & Logic**:
  - 2x Microchip [HV5222PJ](https://ww1.microchip.com/downloads/en/DeviceDoc/20005847A.pdf) 32-channel high-voltage open-drain shift registers in PLCC-44 sockets.
  - [CD4504](https://www.ti.com/lit/ds/symlink/cd4504b.pdf) CMOS hex voltage level shifter and [SN74LV1T34](https://www.ti.com/lit/ds/symlink/sn74lv1t34.pdf) logic buffer.
- **Timekeeping**: High-precision [DS3231MZ](https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231M.pdf) / DS1307 I2C Real-Time Clock with CR1220 coin cell battery backup and automatic NTP synchronization.
- **Environmental Sensors**:
  - Sensirion [SCD4x / SCD41](https://sensirion.com/media/documents/48C4B71E/66432D15/Sensirion_CO2_Sensors_SCD4x_Datasheet.pdf) photoacoustic CO2, temperature, and relative humidity sensor (I2C address `0x62`).
  - Dallas [DS18B20](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf) 1-Wire digital temperature sensor on GPIO2.
- **RGB Underlighting**: 12x [SK6812MINI](https://mouser.com/datasheet/2/737/SK6812MINI_REV02_EN-1501726.pdf) addressable RGB LEDs under Nixie tube sockets, supplemented by 4x RGB colon LEDs driven by an [AW9523B](https://www.awinic.com/en/product-detail/AW9523B) 16-channel I2C LED driver.
- **User Interface Controls**: Physical multi-gesture push button on GPIO20 supporting single-click, double-click, and long-press interactions.
- **Power & Connectivity**:
  - 12V DC power input via 2-pin JST-XH terminal (`J104`).
  - USB-C connector (`J101`) for power and serial programming.
- **Enclosure**: Custom protective enclosure designed in **Autodesk Fusion 360** tailored specifically for the PCB dimensions and IN-12 Nixie tube arrangement.

---

## 📁 Repository Structure

```
NosClock/
├── esphome/                     # ESPHome configuration & native custom component
│   ├── nosclock.yaml            # Main ESPHome YAML configuration file
│   └── components/
│       └── nosclock/            # C++ custom component for Nixie mux & LED effects
└── hardware/                    # KiCad 8 project files & manufacturing outputs
    ├── NosClock.kicad_pro       # KiCad main project file
    ├── NosClock.kicad_sch       # Root schematic sheet
    ├── NosClock.kicad_pcb       # Complete multi-layer PCB layout
    ├── production/              # Manufacturing files (BOM, positions, Gerber ZIP)
    └── bom/                     # Interactive HTML Bill of Materials (iBOM)
```

---

## 🚀 Firmware Deployment & Operation

NosClock runs on **ESPHome**, featuring a native custom C++ component in `esphome/components/nosclock` that seamlessly integrates the Nixie display multiplexing, addressable RGB LED effects, and sensors directly into Home Assistant.

### Features exposed in Home Assistant:
- **Display Effect (`select`)**: Choose LED underlighting effects (`Empty`, `Solid`, `Aurora`, `DigitSync`, `Scanner`, `ProgressBar`).
- **Display Color (`select`)**: Choose active LED color (`Red`, `Green`, `Blue`, `Yellow`, `Cyan`, `Magenta`, `White`).
- **Tubes Brightness (`number`)**: Adjust Nixie tube display brightness (10% to 100%).
- **Effect Brightness (`number`)**: Adjust RGB LED underlighting brightness (0% to 100%).
- **Enabled (`switch`)**: Toggle Nixie display power on/off.
- **High CO2 Alert (`binary_sensor`)**: Automated gas alarm state triggered when CO2 exceeds 1500 ppm.
- **Sensors**: Ambient Temperature, Ambient Humidity, CO2 (SCD4x), and Temperature (DS18B20).

![Home Assistant Integration](images/home-assistant.jpg)

![ESPHome Controls](images/esphome.jpg)

### Flashing via ESPHome:
1. Create a `secrets.yaml` file in your ESPHome directory containing:
   ```yaml
   wifi_ssid: "Your_WiFi_SSID"
   wifi_password: "Your_WiFi_Password"
   wifi_fallback_password: "Fallback_AP_Password"
   nosclock_api_key: "Your_HA_API_Encryption_Key"
   nosclock_ota_password: "Your_OTA_Password"
   ```
2. Flash the firmware using the ESPHome CLI:
   ```bash
   esphome run esphome/nosclock.yaml
   ```

---

## 🔘 Physical Multi-Button Controls

The onboard physical push-button connected to **GPIO20** (`clock_button`) handles gesture inputs:

- **Single Click** (`< 0.5s`): Advances to the next LED effect mode (`DigitSync` ➔ `Scanner` ➔ `Aurora`, etc.).
- **Double Click**: Advances to the next LED background color.
- **Long Press** (`≥ 1.0s`): Toggles the Nixie display power ON or OFF.

---

## 📐 Hardware Schematics & PCB Design

Designed using **KiCad**, the hardware is split into modular schematic sheets:

- [NosClock.kicad_sch](file:///c:/Users/vadym/src/NosClock/hardware/NosClock.kicad_sch) — Top-level system interconnects and MCU pin mapping.
- `ThreeDigits.kicad_sch` — Shift register digit mapping and multiplexing matrix.
- `Lamps.kicad_sch` — Anode switching and lamp driver stages.
- `HighSideSwitch.kicad_sch` — High-side PNP transistor switches for anode multiplexing.
- `UC3843.kicad_sch` — 170V DC Boost converter power stage.
- `MP2307.kicad_sch` — 12V to 5V DC-DC buck converter circuit.
- `Colons.kicad_sch` — PWM colon dimming circuit.

Interactive assembly BOM is available in [hardware/bom/ibom.html](file:///c:/Users/vadym/src/NosClock/hardware/bom/ibom.html).

---

## 📄 License

This project is licensed under open-source terms. See project files for details.
