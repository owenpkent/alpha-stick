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

### Isometric Joystick Components (Alternative to Standard Joystick)

| Qty | Component | Part Number | Description | Est. Cost |
|-----|-----------|-------------|-------------|-----------|
| 4 | Micro Load Cell | 50g or 100g | Force sensing, Wheatstone bridge | $8 |
| 2 | 24-bit ADC | NAU7802 or HX711 | High-resolution force measurement | $4-14 |
| 1 | I2C Multiplexer | PCA9548 | Required if using 2x NAU7802 | $2-6 |
| | | | **Total Isometric Add-on** | **~$14-28** |

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

### Isometric Force-Sensing (No Movement)

An **isometric joystick** detects force direction without any physical movement — the stick is stationary and rigid. This is ideal for users who cannot exert enough force to overcome spring resistance or who have limited range of motion.

#### How It Works

Four micro load cells are arranged in a cross pattern around a rigid center post. When the user pushes in any direction, the load cells detect the force differential and calculate X/Y axis values.

```
                    [Load Cell Y+]
                         │
                         ▼
         [Load Cell X-] ─●─ [Load Cell X+]    ● = Rigid center post
                         │                        (does not move)
                         ▼
                    [Load Cell Y-]

Force calculation:
  X = (X+ reading) - (X- reading)
  Y = (Y+ reading) - (Y- reading)
```

#### Components Required

| Qty | Component | Description | Est. Cost |
|-----|-----------|-------------|-----------|
| 4 | Micro Load Cell | 50g or 100g range, Wheatstone bridge | $8 |
| 2 | NAU7802 24-bit ADC | High-precision I2C ADC (or HX711) | $14 |
| 1 | Rigid mounting structure | 3D printed or machined | $2 |
| | | **Total** | **~$24** |

#### Sensor Options

**Option 1: NAU7802 (Recommended)**
- **Resolution:** 24-bit (16 million steps)
- **Sensitivity:** Sub-gram detection with 50g load cell (~0.003g theoretical)
- **Interface:** I2C (address 0x2A, needs multiplexer for 2 units)
- **Sample rate:** Up to 320 SPS
- **Source:** Adafruit #4538 (~$7 each)

**Option 2: HX711 (Budget)**
- **Resolution:** 24-bit
- **Sensitivity:** Good, but noisier than NAU7802
- **Interface:** Custom 2-wire protocol (CLK + DATA)
- **Sample rate:** 10 or 80 SPS
- **Source:** Amazon/AliExpress (~$2 each)

#### Load Cell Selection

For <5g sensitivity, use **50g or 100g rated micro load cells**:

| Rating | Sensitivity | Best For |
|--------|-------------|----------|
| 50g | ~0.003g with 24-bit ADC | Ultra-light touch, minimal force |
| 100g | ~0.006g with 24-bit ADC | Light touch with more headroom |
| 500g | ~0.03g with 24-bit ADC | Standard use, more robust |

**Recommended:** 50g for accessibility focus, 100g for general use.

#### Wiring Diagram (NAU7802)

```
Load Cells (X-axis pair)          NAU7802 #1
┌─────────────────────┐          ┌──────────┐
│  X+ Load Cell       │          │          │
│    E+ (red) ────────┼─────────▶│ E+       │
│    E- (black) ──────┼─────────▶│ E-       │
│    A+ (white) ──────┼─────────▶│ A+       │
│    A- (green) ──────┼─────────▶│ A-       │
└─────────────────────┘          │          │
┌─────────────────────┐          │          │      ┌──────────┐
│  X- Load Cell       │          │  SDA ────┼─────▶│ GPIO8    │
│    (wired in series │          │  SCL ────┼─────▶│ GPIO9    │
│     or separate)    │          │  VCC ────┼─────▶│ 3.3V     │
└─────────────────────┘          │  GND ────┼─────▶│ GND      │
                                 └──────────┘      └──────────┘
                                                     ESP32-S3

Load Cells (Y-axis pair)          NAU7802 #2
┌─────────────────────┐          ┌──────────┐
│  Y+ and Y- Load     │          │          │
│  Cells (same as     │─────────▶│ (same    │
│  above)             │          │  wiring) │
└─────────────────────┘          └──────────┘
                                      │
                                      ▼
                              I2C Multiplexer (PCA9548)
                              (required - same I2C address)
```

#### Alternative: HX711 Wiring

```
Load Cell (one axis)              HX711 Module
┌─────────────────────┐          ┌──────────┐
│  E+ (red) ──────────┼─────────▶│ E+       │
│  E- (black) ────────┼─────────▶│ E-       │
│  A+ (white) ────────┼─────────▶│ A+       │
│  A- (green) ────────┼─────────▶│ A-       │
└─────────────────────┘          │          │      ┌──────────┐
                                 │  DT ─────┼─────▶│ GPIO4    │
                                 │  SCK ────┼─────▶│ GPIO5    │
                                 │  VCC ────┼─────▶│ 3.3V     │
                                 │  GND ────┼─────▶│ GND      │
                                 └──────────┘      └──────────┘

Repeat for Y-axis with HX711 #2 on GPIO6/GPIO7
```

#### Pin Assignments (Isometric Mode)

| Function | GPIO | Type | Notes |
|----------|------|------|-------|
| NAU7802 SDA | GPIO8 | I2C | Shared with OLED |
| NAU7802 SCL | GPIO9 | I2C | Shared with OLED |
| I2C Mux Reset | GPIO10 | Digital | Optional |
| — OR — | | | |
| HX711 #1 DT | GPIO4 | Digital | X-axis data |
| HX711 #1 SCK | GPIO5 | Digital | X-axis clock |
| HX711 #2 DT | GPIO6 | Digital | Y-axis data |
| HX711 #2 SCK | GPIO7 | Digital | Y-axis clock |

#### Firmware Considerations

- **Calibration:** Tare on startup, store calibration factor in NVS
- **Filtering:** Apply low-pass filter for tremor reduction
- **Deadzone:** Configurable center deadzone (important for accessibility)
- **Sensitivity curve:** Adjustable response curve (linear, exponential, S-curve)
- **Sample rate:** 80-320 Hz for responsive gaming

#### Mechanical Design Notes

- Center post must be **rigid** — no flex or movement
- Load cells should be **preloaded** slightly for bidirectional sensing
- Consider **silicone damping** between post and load cells
- **Knob height** affects leverage — taller = more sensitive

#### Sourcing

| Part | AliExpress | Amazon |
|------|------------|--------|
| 50g Micro Load Cell | "50g load cell" ~$1-2 ea | ~$3-4 ea |
| 100g Micro Load Cell | "100g micro load cell" ~$1-2 ea | ~$3-4 ea |
| HX711 Module | "HX711 module" ~$1-2 ea | SparkFun SEN-13879 ~$10 |
| NAU7802 | — | Adafruit #4538 ~$7 |
| PCA9548 I2C Mux | "PCA9548 module" ~$2 | Adafruit #5626 ~$6 |

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
