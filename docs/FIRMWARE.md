# Firmware Documentation

Technical documentation for Alpha Stick firmware.

---

## Overview

Alpha Stick firmware runs on ESP32-S3 using the Arduino framework via PlatformIO. It provides:

- **USB HID Gamepad** — Plug-and-play controller support
- **Bluetooth LE HID** — Wireless gamepad mode
- **Web Configuration** — Browser-based settings UI
- **OTA Updates** — Firmware updates over WiFi

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     FIRMWARE ARCHITECTURE                        │
│                                                                   │
│  ┌─────────────┐   ┌─────────────┐   ┌─────────────────────┐    │
│  │   Inputs    │   │  Processing │   │      Outputs        │    │
│  │             │   │             │   │                     │    │
│  │ ┌─────────┐ │   │ ┌─────────┐ │   │ ┌─────────────────┐ │    │
│  │ │Joystick │─┼──▶│ │Deadzone │─┼──▶│ │  USB HID        │ │    │
│  │ └─────────┘ │   │ └─────────┘ │   │ └─────────────────┘ │    │
│  │ ┌─────────┐ │   │ ┌─────────┐ │   │ ┌─────────────────┐ │    │
│  │ │ Buttons │─┼──▶│ │ Curves  │─┼──▶│ │  Bluetooth HID  │ │    │
│  │ └─────────┘ │   │ └─────────┘ │   │ └─────────────────┘ │    │
│  │ ┌─────────┐ │   │ ┌─────────┐ │   │ ┌─────────────────┐ │    │
│  │ │ Switches│─┼──▶│ │ Tremor  │─┼──▶│ │  Status LEDs    │ │    │
│  │ └─────────┘ │   │ │ Filter  │ │   │ └─────────────────┘ │    │
│  │ ┌─────────┐ │   │ └─────────┘ │   │                     │    │
│  │ │Sip/Puff │ │   │ ┌─────────┐ │   │                     │    │
│  │ └─────────┘ │   │ │Debounce │ │   │                     │    │
│  │             │   │ └─────────┘ │   │                     │    │
│  └─────────────┘   └─────────────┘   └─────────────────────┘    │
│                                                                   │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │                    Configuration Layer                       │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │ │
│  │  │  WiFi AP │  │ Web UI   │  │   NVS    │  │ Profiles │    │ │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │ │
│  └─────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

---

## Project Structure

```
firmware/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp            # Entry point, setup/loop
│   ├── input.cpp           # Joystick and button reading
│   ├── input.h
│   ├── processing.cpp      # Deadzone, curves, filters
│   ├── processing.h
│   ├── usb_hid.cpp         # USB HID gamepad
│   ├── usb_hid.h
│   ├── ble_hid.cpp         # Bluetooth LE HID
│   ├── ble_hid.h
│   ├── config.cpp          # Configuration management
│   ├── config.h
│   ├── web_server.cpp      # Web configuration UI
│   ├── web_server.h
│   ├── ota.cpp             # Over-the-air updates
│   └── ota.h
├── include/
│   ├── pins.h              # Pin definitions
│   └── version.h           # Firmware version
├── data/
│   └── www/                # Web UI files (SPIFFS)
│       ├── index.html
│       ├── style.css
│       └── app.js
└── config/
    └── default_profile.json
```

---

## PlatformIO Configuration

```ini
; platformio.ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
board_build.mcu = esp32s3
board_build.f_cpu = 240000000L

; USB HID support
build_flags = 
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1

; Partition scheme with OTA
board_build.partitions = min_spiffs.csv

; Libraries
lib_deps =
    ESP32 BLE Keyboard
    ArduinoJson
    ESPAsyncWebServer

; Upload settings
upload_speed = 921600
monitor_speed = 115200
```

---

## Core Components

### Input Module

Reads analog joystick and digital buttons:

```cpp
// input.h
#pragma once
#include <Arduino.h>

struct JoystickState {
    int16_t x;       // -32768 to 32767
    int16_t y;       // -32768 to 32767
    bool button;     // Joystick button pressed
};

struct ButtonState {
    bool buttons[8]; // Up to 8 buttons
    bool switches[4]; // 4 external 3.5mm jacks
};

class InputHandler {
public:
    void begin();
    void update();
    
    JoystickState getJoystick();
    ButtonState getButtons();
    
private:
    void readJoystick();
    void readButtons();
    void debounce();
    
    JoystickState joystick;
    ButtonState buttons;
    uint32_t lastDebounce[12];
};
```

### Processing Module

Applies deadzone, sensitivity curves, and filtering:

```cpp
// processing.h
#pragma once
#include "input.h"
#include "config.h"

class ProcessingPipeline {
public:
    void setConfig(const ControllerConfig& cfg);
    
    JoystickState process(const JoystickState& raw);
    
private:
    int16_t applyDeadzone(int16_t value, float deadzone);
    int16_t applyCurve(int16_t value, float sensitivity, CurveType curve);
    int16_t applyTremorFilter(int16_t value, int16_t prev, float factor);
    
    ControllerConfig config;
    JoystickState prevState;
};
```

### USB HID Module

Presents as standard USB gamepad:

```cpp
// usb_hid.h
#pragma once
#include "input.h"

class USBGamepad {
public:
    bool begin();
    void update(const JoystickState& joystick, const ButtonState& buttons);
    bool isConnected();
    
private:
    void sendReport();
};
```

### Bluetooth HID Module

Wireless gamepad using BLE:

```cpp
// ble_hid.h
#pragma once
#include "input.h"

class BLEGamepad {
public:
    bool begin(const char* deviceName);
    void update(const JoystickState& joystick, const ButtonState& buttons);
    bool isConnected();
    void startPairing();
    
private:
    bool connected;
};
```

---

## Configuration System

### Configuration Structure

```cpp
// config.h
#pragma once
#include <Arduino.h>

enum class CurveType {
    LINEAR,
    EXPONENTIAL,
    LOGARITHMIC,
    SCURVE
};

struct ControllerConfig {
    // Joystick
    float deadzone;          // 0.0 - 0.5
    float sensitivity;       // 0.0 - 2.0
    CurveType curveType;
    bool invertX;
    bool invertY;
    
    // Filtering
    float tremorFilter;      // 0.0 - 1.0 (0 = off)
    uint16_t debounceMs;     // 10 - 100
    
    // Buttons
    bool toggleMode[8];      // true = toggle, false = momentary
    
    // Wireless
    char deviceName[32];
    bool bleEnabled;
    
    // Profile
    char profileName[32];
};

class ConfigManager {
public:
    void begin();
    void load();
    void save();
    void reset();
    
    ControllerConfig& get();
    void set(const ControllerConfig& cfg);
    
    // Profile management
    bool loadProfile(const char* name);
    bool saveProfile(const char* name);
    void listProfiles(char** names, int* count);
    
private:
    ControllerConfig config;
    void loadFromNVS();
    void saveToNVS();
};
```

### NVS Storage

Configuration persists across reboots using ESP32 Non-Volatile Storage:

```cpp
void ConfigManager::saveToNVS() {
    Preferences prefs;
    prefs.begin("alphastick", false);
    
    prefs.putFloat("deadzone", config.deadzone);
    prefs.putFloat("sensitivity", config.sensitivity);
    prefs.putUChar("curveType", (uint8_t)config.curveType);
    prefs.putBool("invertX", config.invertX);
    prefs.putBool("invertY", config.invertY);
    prefs.putFloat("tremor", config.tremorFilter);
    prefs.putUShort("debounce", config.debounceMs);
    prefs.putString("devName", config.deviceName);
    prefs.putBool("bleOn", config.bleEnabled);
    
    prefs.end();
}
```

---

## Web Configuration UI

### WiFi Access Point

On startup or button combo, ESP32 creates a WiFi AP:

```cpp
void WebConfig::begin() {
    WiFi.softAP("AlphaStick", "configure");
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(SPIFFS, "/www/index.html", "text/html");
    });
    
    server.on("/api/config", HTTP_GET, handleGetConfig);
    server.on("/api/config", HTTP_POST, handleSetConfig);
    server.on("/api/calibrate", HTTP_POST, handleCalibrate);
    
    server.begin();
}
```

### Web UI Features

- **Joystick visualization** — Live position display
- **Deadzone adjustment** — Slider with visual preview
- **Sensitivity curves** — Interactive curve editor
- **Button mapping** — Toggle/momentary mode selection
- **Profile management** — Save/load configurations
- **Firmware update** — Upload new firmware via browser

---

## Sensitivity Curves

### Curve Types

```cpp
int16_t ProcessingPipeline::applyCurve(int16_t value, float sensitivity, CurveType curve) {
    float normalized = value / 32767.0f;  // -1.0 to 1.0
    float sign = (normalized >= 0) ? 1.0f : -1.0f;
    float absVal = fabs(normalized);
    float result;
    
    switch (curve) {
        case CurveType::LINEAR:
            result = absVal * sensitivity;
            break;
            
        case CurveType::EXPONENTIAL:
            result = pow(absVal, 2.0f) * sensitivity;
            break;
            
        case CurveType::LOGARITHMIC:
            result = sqrt(absVal) * sensitivity;
            break;
            
        case CurveType::SCURVE:
            // Slow-fast-slow curve
            result = (3.0f * absVal * absVal - 2.0f * absVal * absVal * absVal) * sensitivity;
            break;
    }
    
    result = constrain(result, 0.0f, 1.0f);
    return (int16_t)(result * sign * 32767.0f);
}
```

### Curve Visualization

```
Linear (sensitivity=1.0):        Exponential:
Output                           Output
  │         /                      │        ╱
  │       /                        │      ╱
  │     /                          │    ╱
  │   /                            │  ╱
  │ /                              │╱
  └─────────── Input               └─────────── Input

Logarithmic:                     S-Curve:
Output                           Output
  │    ╱────                       │       ╱──
  │  ╱                             │     ╱
  │╱                               │   ╱
  │                                │ ╱
  │                                │╱
  └─────────── Input               └─────────── Input
```

---

## Tremor Filtering

Simple exponential moving average filter:

```cpp
int16_t ProcessingPipeline::applyTremorFilter(int16_t value, int16_t prev, float factor) {
    if (factor <= 0.0f) return value;
    
    // Higher factor = more smoothing
    return (int16_t)(prev * factor + value * (1.0f - factor));
}
```

Adjustable from 0 (off) to 0.9 (heavy smoothing).

---

## OTA Updates

### ArduinoOTA Integration

```cpp
void OTAManager::begin() {
    ArduinoOTA.setHostname("alphastick");
    
    ArduinoOTA.onStart([]() {
        // Disable gamepad output during update
    });
    
    ArduinoOTA.onEnd([]() {
        ESP.restart();
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        // Handle error, don't brick device
    });
    
    ArduinoOTA.begin();
}

void OTAManager::loop() {
    ArduinoOTA.handle();
}
```

### Web-Based Update

Upload firmware binary via web UI:

```cpp
server.on("/api/update", HTTP_POST, 
    // On complete
    [](AsyncWebServerRequest* req) {
        req->send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
        ESP.restart();
    },
    // On upload
    [](AsyncWebServerRequest* req, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        if (!index) {
            Update.begin(UPDATE_SIZE_UNKNOWN);
        }
        Update.write(data, len);
        if (final) {
            Update.end(true);
        }
    }
);
```

---

## Main Loop

```cpp
// main.cpp
#include "input.h"
#include "processing.h"
#include "usb_hid.h"
#include "ble_hid.h"
#include "config.h"
#include "web_server.h"
#include "ota.h"

InputHandler input;
ProcessingPipeline processor;
USBGamepad usbGamepad;
BLEGamepad bleGamepad;
ConfigManager config;
WebConfig webConfig;
OTAManager ota;

void setup() {
    Serial.begin(115200);
    
    config.begin();
    config.load();
    processor.setConfig(config.get());
    
    input.begin();
    usbGamepad.begin();
    
    if (config.get().bleEnabled) {
        bleGamepad.begin(config.get().deviceName);
    }
    
    // Check for config mode button combo
    if (digitalRead(BUTTON_1) == LOW && digitalRead(BUTTON_2) == LOW) {
        webConfig.begin();
        ota.begin();
    }
}

void loop() {
    input.update();
    
    JoystickState raw = input.getJoystick();
    JoystickState processed = processor.process(raw);
    ButtonState buttons = input.getButtons();
    
    if (usbGamepad.isConnected()) {
        usbGamepad.update(processed, buttons);
    }
    
    if (bleGamepad.isConnected()) {
        bleGamepad.update(processed, buttons);
    }
    
    ota.loop();
    
    delay(1);  // ~1000Hz update rate
}
```

---

## Building & Flashing

### Prerequisites

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
3. Connect ESP32-S3 via USB

### Build Commands

```powershell
# Build firmware
cd C:\Users\Owen\dev\alpha-stick\firmware; pio run

# Upload to device
pio run --target upload

# Upload filesystem (web UI)
pio run --target uploadfs

# Monitor serial output
pio device monitor

# Clean build
pio run --target clean
```

---

## Debugging

### Serial Output

Enable debug output in `platformio.ini`:

```ini
build_flags = 
    -DCORE_DEBUG_LEVEL=4
    -DDEBUG_MODE=1
```

### Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| USB not recognized | Driver issue | Install ESP32 USB driver |
| Joystick drift | ADC noise | Increase deadzone |
| BLE not pairing | Conflicting device | Clear pairing on both ends |
| Web UI not loading | SPIFFS not uploaded | Run `pio run --target uploadfs` |
| OTA fails | Partition too small | Use `min_spiffs.csv` partition |

---

## Future Enhancements

- [ ] XInput mode for native Xbox support
- [ ] Mouse emulation mode
- [ ] Keyboard emulation mode
- [ ] Macro recording and playback
- [ ] Multiple joystick support (dual-stick)
- [ ] Gyro/accelerometer input
- [ ] Voice command integration

See [TODO.md](../TODO.md) for roadmap.
]]>
