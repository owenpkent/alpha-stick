# Hardware Documentation

Technical documentation for Alpha Stick hardware design.

---

## Overview

Alpha Stick uses an **ESP32-S3** microcontroller with:
- USB HID for wired gamepad mode
- Bluetooth LE for wireless operation
- WiFi for web-based configuration
- ADC for analog joystick input
- GPIO for buttons and external switches

---

## Block Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        ALPHA STICK                               │
│                                                                   │
│  ┌──────────────┐     ┌─────────────────┐     ┌──────────────┐  │
│  │   Joystick   │────▶│                 │────▶│   USB-C      │──┼──▶ PC/Console
│  │  (Analog)    │     │    ESP32-S3     │     │   (HID)      │  │
│  └──────────────┘     │                 │     └──────────────┘  │
│                       │  ┌───────────┐  │                        │
│  ┌──────────────┐     │  │  ADC      │  │     ┌──────────────┐  │
│  │   Buttons    │────▶│  │  GPIO     │  │────▶│  Bluetooth   │──┼──▶ Wireless
│  │  (Digital)   │     │  │  USB      │  │     │   LE HID     │  │
│  └──────────────┘     │  │  BLE      │  │     └──────────────┘  │
│                       │  │  WiFi     │  │                        │
│  ┌──────────────┐     │  └───────────┘  │     ┌──────────────┐  │
│  │  3.5mm Jacks │────▶│                 │────▶│   WiFi AP    │──┼──▶ Config
│  │  (Switches)  │     │                 │     │   (Web UI)   │  │
│  └──────────────┘     └─────────────────┘     └──────────────┘  │
│                                                                   │
│  ┌──────────────┐     ┌─────────────────┐                        │
│  │  Sip/Puff    │────▶│ Pressure Sensor │  (Optional)            │
│  │  (Optional)  │     └─────────────────┘                        │
│  └──────────────┘                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Bill of Materials (BOM)

### Core Components

| Qty | Component | Part Number | Description | Est. Cost |
|-----|-----------|-------------|-------------|-----------|
| 1 | ESP32-S3-DevKitC-1 | ESP32-S3-WROOM-1 | Main MCU, N16R8 variant recommended | $10 |
| 1 | Analog Joystick | KY-023 or PSP-1000 | Dual-axis with button | $3 |
| 2 | Tactile Button | 6x6x5mm | Primary buttons | $0.50 |
| 4 | 3.5mm Jack | PJ-307 | External switch input (mono) | $2 |
| 1 | USB-C Connector | TYPE-C-31-M-12 | Power and data | $0.50 |
| 1 | 3D Printed Case | — | PLA or PETG | $3 |
| — | Wires, screws | — | M3 screws, 24AWG wire | $2 |
| | | | **Total Core** | **~$21** |

### Optional Components

| Qty | Component | Part Number | Description | Est. Cost |
|-----|-----------|-------------|-------------|-----------|
| 1 | Hall-Effect Joystick | Alps RKJXV | Smoother, longer life | $15 |
| 1 | OLED Display | SSD1306 0.96" I2C | Status display | $5 |
| 1 | Pressure Sensor | MPXV7002DP | Sip/puff input | $10 |
| 1 | LiPo Battery | 3.7V 1000mAh | Wireless operation | $8 |
| 1 | Charging Module | TP4056 USB-C | Battery charging | $2 |
| 1 | Power Switch | SS12D00 | On/off toggle | $0.50 |
| 2 | RGB LED | WS2812B | Status indicators | $1 |

---

## Microcontroller Selection

### Why ESP32-S3?

| Feature | ESP32-S3 | ESP32-C3 | Arduino Pro Micro |
|---------|----------|----------|-------------------|
| USB HID Native | ✅ Yes | ✅ Yes | ✅ Yes |
| Bluetooth LE | ✅ Yes | ✅ Yes | ❌ No |
| WiFi | ✅ Yes | ✅ Yes | ❌ No |
| ADC Channels | 20 | 6 | 12 |
| Flash Storage | 16MB | 4MB | 32KB |
| RAM | 512KB | 400KB | 2.5KB |
| OTA Updates | ✅ Yes | ✅ Yes | ❌ No |
| Price | ~$10 | ~$5 | ~$20 |

**Recommendation:** ESP32-S3 for full features, ESP32-C3 for budget builds.

---

## Joystick Options

### Standard Analog (KY-023)

- **Pros:** Cheap ($2), widely available, easy to interface
- **Cons:** Potentiometer wear, spring-return only
- **Interface:** 2x analog (X, Y) + 1x digital (button)
- **Voltage:** 5V tolerant, works at 3.3V

### PSP-Style Analog

- **Pros:** Compact, smooth feel, good for thumb operation
- **Cons:** Slightly harder to source, needs custom mount
- **Interface:** 2x analog (X, Y)
- **Voltage:** 3.3V

### Hall-Effect (Alps RKJXV)

- **Pros:** No wear, ultra-smooth, long life, no contact bounce
- **Cons:** More expensive ($15+), larger footprint
- **Interface:** 2x analog (X, Y)
- **Voltage:** 5V (may need level shifting)

### Ultra-Light Force (Custom)

Inspired by Feather joystick:
- **Magnet + Hall sensor array** — Detect position without mechanical resistance
- **Adjustable springs** — Add force feedback as needed
- **Target:** <10 gram actuation force

---

## Pin Assignments

### ESP32-S3-DevKitC-1 Pinout

```
┌─────────────────────────────────────┐
│          ESP32-S3-DevKitC           │
│                                     │
│  3V3 ─┤├─ GND                       │
│   EN ─┤├─ GPIO43 (TX)               │
│ IO36 ─┤├─ GPIO44 (RX)               │
│ IO37 ─┤├─ GPIO1  ─── Joystick X     │
│ IO38 ─┤├─ GPIO2  ─── Joystick Y     │
│ IO39 ─┤├─ GPIO42                    │
│ IO40 ─┤├─ GPIO41                    │
│ IO41 ─┤├─ GPIO40                    │
│ IO42 ─┤├─ GPIO39 ─── Button 1       │
│  RXD ─┤├─ GPIO38 ─── Button 2       │
│  TXD ─┤├─ GPIO37 ─── Joystick Btn   │
│ IO21 ─┤├─ GPIO36                    │
│ IO47 ─┤├─ GPIO35 ─── Jack 1         │
│ IO48 ─┤├─ GPIO34 ─── Jack 2         │
│  USB ─┤├─ GPIO33 ─── Jack 3         │
│  GND ─┤├─ GPIO26 ─── Jack 4         │
│                                     │
│        [USB-C Port]                 │
└─────────────────────────────────────┘
```

### Pin Assignment Table

| Function | GPIO | Type | Notes |
|----------|------|------|-------|
| Joystick X | GPIO1 | ADC | ADC1_CH0 |
| Joystick Y | GPIO2 | ADC | ADC1_CH1 |
| Joystick Button | GPIO37 | Digital | Internal pullup |
| Button 1 | GPIO39 | Digital | Internal pullup |
| Button 2 | GPIO38 | Digital | Internal pullup |
| Jack 1 (Switch) | GPIO35 | Digital | External pullup |
| Jack 2 (Switch) | GPIO34 | Digital | External pullup |
| Jack 3 (Switch) | GPIO33 | Digital | External pullup |
| Jack 4 (Switch) | GPIO26 | Digital | External pullup |
| I2C SDA (OLED) | GPIO8 | I2C | Optional |
| I2C SCL (OLED) | GPIO9 | I2C | Optional |
| Sip/Puff | GPIO3 | ADC | Optional, ADC1_CH2 |
| LED Data | GPIO48 | Digital | WS2812B, optional |

---

## 3.5mm Jack Wiring

Standard mono 3.5mm jack for switch compatibility (Xbox Adaptive Controller compatible):

```
        ┌─────┐
  Tip ──┤     ├── GPIO (with pullup)
        │     │
Sleeve ─┤     ├── GND
        └─────┘

Switch closed: GPIO reads LOW
Switch open:   GPIO reads HIGH (pulled up)
```

**Debouncing:** Handle in firmware, configurable 10-100ms.

---

## Power

### USB-C Powered (Default)
- **Input:** 5V via USB-C
- **Regulation:** ESP32 devkit has onboard 3.3V regulator
- **Current:** ~150mA typical, 500mA peak (WiFi TX)

### Battery Powered (Optional)
- **Battery:** 3.7V LiPo, 1000mAh minimum
- **Charger:** TP4056 module with USB-C input
- **Protection:** TP4056 includes overcharge/overdischarge protection
- **Runtime:** ~8-12 hours typical use

### Power Circuit (Battery Option)

```
USB-C ──┬──▶ TP4056 ──▶ LiPo Battery
        │        │
        │        ▼
        │    3.7-4.2V
        │        │
        └────────┼──▶ ESP32 VIN
                 │
                 ▼
            (Auto-switch via TP4056)
```

---

## Schematic Notes

### Joystick Connection

```
Joystick Module:
  VCC ─────▶ 3.3V
  GND ─────▶ GND
  VRx ─────▶ GPIO1 (ADC)
  VRy ─────▶ GPIO2 (ADC)
  SW  ─────▶ GPIO37 (with 10K pullup to 3.3V)
```

### Button Connection

```
Button (Normally Open):
  One leg ───▶ GPIO (configured as INPUT_PULLUP)
  Other leg ──▶ GND
```

### 3.5mm Jack Connection

```
3.5mm Jack (Mono):
  Tip ────▶ GPIO (with 10K pullup to 3.3V)
  Sleeve ─▶ GND
```

---

## PCB Design Guidelines

### Recommended Specs

| Parameter | Value |
|-----------|-------|
| Layers | 2 |
| Board thickness | 1.6mm |
| Copper weight | 1oz |
| Minimum trace | 0.3mm |
| Minimum via | 0.4mm |
| Finish | HASL or ENIG |

### Design Tips

1. **Keep ADC traces short** — Reduce noise on analog inputs
2. **Ground plane on bottom** — Improve EMI performance
3. **Decoupling caps** — 100nF near ESP32 power pins
4. **USB data lines** — Match impedance, keep short
5. **Test points** — Add for GND, 3.3V, and key signals

---

## Sourcing Components

### Recommended Suppliers

| Supplier | Best For | Notes |
|----------|----------|-------|
| AliExpress | ESP32, joysticks, connectors | Cheap, slow shipping |
| Amazon | Fast delivery, kits | Higher prices |
| Digi-Key | Quality components, datasheets | Minimum orders |
| LCSC | PCB components, assembly | Ships from China |
| JLCPCB | PCB fabrication, assembly | Cheap PCBs |

### Ready-Made Kits (Future)

We plan to offer pre-packaged component kits for easier builds.

---

## Safety Considerations

- **ESD Protection:** Handle ESP32 with care, use grounding strap
- **LiPo Safety:** Never puncture, short, or overcharge batteries
- **Enclosure:** Ensure no exposed conductors in final build
- **Mounting:** Secure to prevent falls, especially near medical equipment

---

## Next Steps

1. Finalize component selection
2. Create KiCad schematic
3. Design PCB layout
4. Order prototype boards
5. Test and iterate

See [FIRMWARE.md](FIRMWARE.md) for software implementation details.
]]>
